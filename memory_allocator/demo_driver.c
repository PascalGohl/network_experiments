// from http://research.wand.net.nz/hardware/ntp.php
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/syslog.h>
//#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <asm/io.h>
#include <asm/irq.h>


struct Settings
{
  char ipv4[4];
   char ipv6[16];
   char port[2];
   char mac[6];
   char time[8];
   char addend[8];
};

MODULE_LICENSE("Dual BSD/GPL");

static long int register_address = 0x63C00000;
static long int register_length = 0xFFFF;
static char *device_name = "ntp_fpga";
static char *dumper_loc = "/NTP/dumper";
static int irq_number = 87;
static int device_major = 62;
static long int log_data_size = 262144;//256k

module_param(register_address, long, 0);
MODULE_PARM_DESC(register_address, "Base address for the register peripheral");

module_param(register_length, long, 0);
MODULE_PARM_DESC(register_length, "Address length of the register peripheral");

module_param(device_name, charp, 0);
MODULE_PARM_DESC(device_name, "Name for the device")
;
module_param(dumper_loc, charp, 0);
MODULE_PARM_DESC(dumper_loc, "Location of dumper program");

module_param(irq_number, int, 0);
MODULE_PARM_DESC(irq_number, "IRQ number for interrupt");

module_param(log_data_size, long, 0);
MODULE_PARM_DESC(log_data_size, "Size (in bytes) for the log buffer");

module_param(device_major, int, 0);
MODULE_PARM_DESC(device_major, "Major number of device");

void setup_buffer(void);
void setup_ntp(void);
int ntp_fpga_open(struct inode *inode, struct file *filp);
int ntp_fpga_release(struct inode *inode, struct file *filp);
ssize_t ntp_fpga_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t ntp_fpga_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static irqreturn_t buffer_full_handler(int irq, void *dummy, struct pt_regs * regs);
void ntp_fpga_exit(void);
int ntp_fpga_init(void);

/* Structure that declares the common */
/* file access functions */
struct file_operations ntp_fpga_fops = {
  read: ntp_fpga_read,
  write: ntp_fpga_write,
  open: ntp_fpga_open,
  release: ntp_fpga_release
};

/* Driver global variables */
/* Major number */
int current_buffer = 0;
int no_of_bytes = 128; //32 * 32bit registers
char *log_data;
char *log_data2;

/* Control variable for memory */
/* reservation of the ntp_fpga register*/
int port;

module_init(ntp_fpga_init);
module_exit(ntp_fpga_exit);

static struct workqueue_struct *my_wq;

typedef struct {struct work_struct my_work;int x;} my_work_t;
my_work_t *work;

static void my_wq_function( struct work_struct *work)
{
  printk(KERN_INFO "<NTP> this is an NTP log\n");

  current_buffer++;
  current_buffer = current_buffer % 2;// switch over to the next buffer

  return;
}




int ntp_fpga_init(void)
{
	int result;
	int status;


	/* Registering device */
	result = register_chrdev(device_major, device_name , &ntp_fpga_fops);
	if (result < 0)
	{
		printk("<1>ntp_fpga: cannot obtain major number %d\n",device_major);
		return result;
	}

	/* Registering port */
	port = check_mem_region(register_address, register_length);
	if (port)
	{
		printk("<1>ntp_fpga: cannot reserve memory\n");
		result = port;
		goto fail;
	}

	request_mem_region(register_address, register_length, device_name);
	setup_buffer();
	setup_ntp();

	status = request_irq(irq_number, buffer_full_handler, 0, device_name, NULL);//register the interrupts

	enable_irq(irq_number);

	if(status == 0)
	{
		printk("<1>IRQ setup successful\n");
	}
	else
	{
		printk("<1>IRQ setup failed! Status: %d\n",status);
		goto fail;
	}

	printk("<1>Inserting ntp_fpga module\n");

	my_wq = create_workqueue("my_queue");
	if (my_wq)
	{
		/* Create the work to be executed on an interrupt */
		work = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
		if (work)
		{
			INIT_WORK( (struct work_struct *)work, my_wq_function );
			work->x = 1;
		}
	}

	return 0;

	fail:
	ntp_fpga_exit();
	return result;
}

void setup_buffer()
{
	void *ptr;
	log_data = kmalloc(log_data_size,GFP_KERNEL);
	log_data2 = kmalloc(log_data_size, GFP_KERNEL);

	ptr = ioremap(register_address,register_length);
	//iowrite32(&log_data,ptr);//store the memory address of the buffer at the base of the ntp_fpga memory

	//iowrite32(&log_data2,ptr);//store the memory address of the 2nd buffer at the second position of the ntp_fpga memory

	memcpy_toio(ptr,&log_data,sizeof(&log_data));
	ptr+= sizeof(&log_data);//move pointer to next register
	memcpy_toio(ptr,&log_data2,sizeof(&log_data2));


}

void setup_ntp(void)
{
	void* ptr;
	struct Settings set;
	char test1[4] = {'i','p','v','4'};
  	char test2[16] = {'|','i','p','v','6','_','_','_','_','_','_','_','_','_','_','|'};
   char test3[2] = {'p','o'};
   char test4[6] = {'|','m','a','c','_','|'};
   char test5[8] = {'|','_','t','i','m','e','_','|'};
   char test6[8] = {'|','_','a','d','d','e','_','|'};
	memcpy(set.ipv4, test1,sizeof(test1));
	memcpy(set.ipv6, test2,sizeof(test2));
	memcpy(set.port, test3,sizeof(test3));
	memcpy(set.mac, test4,sizeof(test4));
	memcpy(set.time, test5,sizeof(test5));
	memcpy(set.addend, test6,sizeof(test6));

	ptr = ioremap(register_address,register_length);
	memcpy_toio(ptr,&set,sizeof(struct Settings));
}

void ntp_fpga_exit(void)
{
  /* Make major number free! */
  unregister_chrdev(device_major, device_name);

    /* Make port free! */
  if (!port)
  {
    release_mem_region(register_address, register_length);
  }
  free_irq(irq_number, NULL);
  flush_workqueue( my_wq );
  destroy_workqueue( my_wq );

  printk("<1>Removing ntp_fpga module\n");
}


int ntp_fpga_open(struct inode *inode, struct file *filp)
 {

  /* Success */
  return 0;

}

int ntp_fpga_release(struct inode *inode, struct file *filp)
{

  /* Success */
  return 0;
}


irqreturn_t buffer_full_handler(int irq, void *dummy, struct pt_regs * regs)
{

  queue_work( my_wq, (struct work_struct *)work );//schedule the dumper function
  return IRQ_HANDLED;
}

/*www.makelinux.net/ldd3/chp-9-sect-4*/
ssize_t ntp_fpga_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
   size_t no_of_bytes_to_read;
   size_t max_no_bytes;
	char fpga_buffer[no_of_bytes];
	void* ptr;

	if(*f_pos > no_of_bytes)//if they want to read from the log buffer
	{
		max_no_bytes = log_data_size;
		if(*f_pos >= max_no_bytes)//if read to the end of the file return 0 bytes
		{
		   return 0;
		}

		if(count < max_no_bytes)//if they are asking for less than the maximum number of bytes
		{
		  no_of_bytes_to_read = count;
		}
		else//otherwise only let them copy however many bytes are available
		{
		  no_of_bytes_to_read = max_no_bytes;
		}

		if(no_of_bytes_to_read > (max_no_bytes - *f_pos))//make sure they don't go off end of file in read from their current position
		{
		   no_of_bytes_to_read = (max_no_bytes - *f_pos);
		}


		if(current_buffer == 1)//return the current buffer, i.e the one not being used by the ntp_fpga at the moment
		{
		  copy_to_user(buf,log_data,no_of_bytes_to_read);
		}
		else
		{
		  copy_to_user(buf,log_data2,no_of_bytes_to_read);
		}

		*f_pos += no_of_bytes_to_read;

		return no_of_bytes_to_read;
	}
	else//otherwise reading from fpga registers
	{
		if(*f_pos >= no_of_bytes)//if read to the end of the file return 0 bytes
		{
		return 0;
		}

		if(count < (no_of_bytes - *f_pos))//if they are asking for less than the maximum number of bytes
		{
		no_of_bytes_to_read = count;
		}
		else//otherwise only let them copy however many bytes are available
		{
		no_of_bytes_to_read = no_of_bytes;
		}

		if(no_of_bytes_to_read > (no_of_bytes - *f_pos))//make sure they don't go off end of the fpga registers
		{
		no_of_bytes_to_read = (no_of_bytes - *f_pos);
		}

		ptr = ioremap(register_address,register_length);
		ptr += *f_pos * sizeof(char);//move to where they are in the file

		memcpy_fromio(&fpga_buffer, ptr, no_of_bytes_to_read);


		copy_to_user(buf,fpga_buffer,no_of_bytes_to_read);

		*f_pos += no_of_bytes_to_read;
		return no_of_bytes_to_read;
	}
}


ssize_t ntp_fpga_write( struct file *filp, char *buf, size_t count, loff_t *f_pos)
{

  char tmp[no_of_bytes];
  size_t no_of_bytes_to_write;
  void* ptr;

  if(count <= no_of_bytes)
  {
    no_of_bytes_to_write = count;
  }
  else
  {
     no_of_bytes_to_write = no_of_bytes;
  }

  if(no_of_bytes_to_write > (no_of_bytes - *f_pos))//make sure they don't go off end of file in read from their current position
  {
     no_of_bytes_to_write = (no_of_bytes - *f_pos);
  }


  ptr = ioremap(register_address,register_length);

  ptr += *f_pos * sizeof(char);//move to where they are in the file

  copy_from_user(&tmp,buf,no_of_bytes_to_write);

  memcpy_toio(ptr,&tmp,no_of_bytes_to_write);


  return no_of_bytes_to_write;
}



