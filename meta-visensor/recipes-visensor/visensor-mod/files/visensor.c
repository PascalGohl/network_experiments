// some code snipets from: https://github.com/minghuascode/qemu120/blob/master/xternapp/kernel-mmap/mmap.example/mmap.c

//#include <linux/config.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#ifdef MODVERSIONS
#  include <linux/modversions.h>
#endif
#include <asm/io.h>
#include <linux/device.h>
#include <asm/uaccess.h> /* copy_from/to_user */

/* character device structures */
static dev_t aslam_dev;
static struct cdev aslam_cdev;

/* methods of the character device */
static int aslam_open(struct inode *inode, struct file *filp);
static int aslam_release(struct inode *inode, struct file *filp);
static int aslam_mmap(struct file *filp, struct vm_area_struct *vma);
static int aslam_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static int aslam_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);

/* the file operations, i.e. all character device methods */
static struct file_operations aslam_fops = {
    .open = aslam_open,
    .release = aslam_release,
    .mmap = aslam_mmap,
    .read = aslam_read,
    //    .write = aslam_write,
    .owner = THIS_MODULE, };

// internal data
// length of the memory area
#define IMAGE_SIZE 360960 //=752*480
static const int npages = IMAGE_SIZE * 10 / PAGE_SIZE;

#define REGBASE 0x7e200000
#define REGSIZE 16

// pointer to the vmalloc'd area - alway page aligned
static int *vmalloc_area;
// pointer to the kmalloc'd area, rounded up to a page boundary
static int *kmalloc_area;
// pointer to the device register area
static void *config_register_ptr;
// original pointer for kmalloc'd area as returned by kmalloc
static void *kmalloc_ptr;
// allocated bytes in memory
static int bytes_in_memory;

struct class *cl;

/* character device open method */
static int aslam_open(struct inode *inode, struct file *filp) {
  return 0;
}
/* character device last close method */
static int aslam_release(struct inode *inode, struct file *filp) {
  return 0;
}

// helper function, mmap's the kmalloc'd area which is physically contiguous
int aslam_kmem(struct file *filp, struct vm_area_struct *vma) {
  int ret;
  long length = vma->vm_end - vma->vm_start;
  printk("aslam_kmem with length %d\n", length);

  /* check length - do not allow larger mappings than the number of
   pages allocated */
  if (length > npages * PAGE_SIZE) {
    printk("requested size longer than allocated memory\n");
    return -EIO;
  }

  /* map the whole physically contiguous area in one piece */
  if ((ret = remap_pfn_range(vma, vma->vm_start,
                             virt_to_phys((void *) kmalloc_area) >> PAGE_SHIFT,
                             length, vma->vm_page_prot)) < 0) {
    printk("remap_pfn_range failed\n");
    return ret;
  }

  return 0;
}
// helper function, mmap's the vmalloc'd area which is not physically contiguous
int aslam_vmem(struct file *filp, struct vm_area_struct *vma) {
  int ret;
  long length = vma->vm_end - vma->vm_start;
  unsigned long start = vma->vm_start;
  char *vmalloc_area_ptr = (char *) vmalloc_area;
  unsigned long pfn;

  /* check length - do not allow larger mappings than the number of
   pages allocated */
  if (length > npages * PAGE_SIZE) {
    printk("requested size longer than allocated memory\n");
    return -EIO;
  }

  /* loop over all pages, map it page individually */
  while (length > 0) {
    pfn = vmalloc_to_pfn(vmalloc_area_ptr);
    if ((ret = remap_pfn_range(vma, start, pfn, PAGE_SIZE, PAGE_SHARED)) < 0) {
      return ret;
    }
    start += PAGE_SIZE;
    vmalloc_area_ptr += PAGE_SIZE;
    length -= PAGE_SIZE;
  }
  return 0;
}

/* character device aslam method */
static int aslam_mmap(struct file *filp, struct vm_area_struct *vma) {
  /* at offset 0 we map the vmalloc'd area */
  //        if (vma->vm_pgoff == 0) {
  //                return aslam_vmem(filp, vma);
  //        }
  /* at offset NPAGES we map the kmalloc'd area */
  if (vma->vm_pgoff == 0) {
    return aslam_kmem(filp, vma);
  }
  /* at any other offset we return an error */
  return -EIO;
}

/*www.makelinux.net/ldd3/chp-9-sect-4*/
//static int aslam_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
//{
//  size_t no_of_bytes_to_read;
//  size_t max_no_bytes;
//  char fpga_buffer[no_of_bytes];
//  void* ptr;
//
//
//  if(*f_pos >= no_of_bytes)//if read to the end of the file return 0 bytes
//  {
//    return 0;
//  }
//
//  if(count < (no_of_bytes - *f_pos))//if they are asking for less than the maximum number of bytes
//  {
//    no_of_bytes_to_read = count;
//  }
//  else//otherwise only let them copy however many bytes are available
//  {
//    no_of_bytes_to_read = no_of_bytes;
//  }
//
//  if(no_of_bytes_to_read > (no_of_bytes - *f_pos))//make sure they don't go off end of the fpga registers
//  {
//    no_of_bytes_to_read = (no_of_bytes - *f_pos);
//  }
//
//  ptr = config_register_ptr;
//  ptr += *f_pos * sizeof(char);//move to where they are in the file
//
//  memcpy_fromio(&fpga_buffer, ptr, no_of_bytes_to_read);
//
//
//  copy_to_user(buf,fpga_buffer,no_of_bytes_to_read);
//
//  *f_pos += no_of_bytes_to_read;
//  return no_of_bytes_to_read;
//
//}

static int aslam_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{

  if(*f_pos != 0)
  {
    printk("Tried to read invalid range: %d, %d\n", (int)*f_pos, (int)count);
  }
  switch(count)
  {
    case(0):
    {
      int memory_address;
      printk("kmalloc_area: [%p]\n", (void*)kmalloc_area);
      printk("kmalloc_ptr: [%p]\n", (void*)kmalloc_ptr);
      printk("virt_to_phys((void *) kmalloc_area): [%p]\n", (void*)virt_to_phys((void *) kmalloc_area));
      memory_address = (int)virt_to_phys((void *) kmalloc_area);
      copy_to_user(buf, &memory_address, 4);
      break;
    }
    case(1):
    {
      printk("allocated bytes: [%d]\n", bytes_in_memory);
      copy_to_user(buf, &bytes_in_memory, 4);
      break;
    }
    default:
      printk("Tried to read invalid range: %d, %d\n", (int)*f_pos, (int)count);
      return -EIO;
  }


  //  char text[8];
  //  sprintf( text, "abcdefg" );
  //  copy_to_user(buf, text, 8);

  return 4;

}

//
//
//static int aslam_write( struct file *filp, char *buf, size_t count, loff_t *f_pos)
//{
//
//  char tmp[no_of_bytes];
//  size_t no_of_bytes_to_write;
//  void* ptr;
//
//  if(count <= no_of_bytes)
//  {
//    no_of_bytes_to_write = count;
//  }
//  else
//  {
//    no_of_bytes_to_write = no_of_bytes;
//  }
//
//  if(no_of_bytes_to_write > (no_of_bytes - *f_pos))//make sure they don't go off end of file in read from their current position
//  {
//    no_of_bytes_to_write = (no_of_bytes - *f_pos);
//  }
//
//
//  ptr = config_register_ptr;
//
//  ptr += *f_pos * sizeof(char);//move to where they are in the file
//
//  copy_from_user(&tmp,buf,no_of_bytes_to_write);
//
//  memcpy_toio(ptr,&tmp,no_of_bytes_to_write);
//
//
//  return no_of_bytes_to_write;
//}
//static int aslam_init_config_register()
//{
//  //  int result;
//  //  result = check_mem_region(config_register, REGSIZE);
//  //  if (result) {
//  //    printk(KERN_INFO "SLAM_SENSOR: can't get I/O mem address 0x%x\n", config_register);
//  //    return result;
//  //  }
//
//  //  if (request_mem_region(config_register, REGSIZE, "slam_sensor") == NULL)
//  //    return -1;
//
//  if(request_mem_region(REGBASE, REGSIZE, "slam_sensor") != NULL){
//    printk(KERN_INFO "SLAM_SENSOR: request_mem_region failed\n");
//    release_mem_region(REGBASE, REGSIZE);
//    return -1;
//  }
//
//  // map the config register to pointer
//  config_register_ptr = ioremap(REGBASE, REGSIZE);
//  if (config_register_ptr == 0) {
//    printk(KERN_INFO "SLAM_SENSOR: can't map I/O mem address 0x%x\n", config_register_ptr);
//    return -1;
//  }
//
//  return 0;
//}
//
//static int aslam_cleanup_config_register()
//{
//  iounmap(REGBASE);
//  release_mem_region(REGBASE, REGSIZE);
//}

/* module initialization - called at module load time */
static int aslam_init(void) {
  int ret = 0;
  int i;

  /* allocate a memory area with kmalloc. Will be rounded up to a page boundary */
  if ((kmalloc_ptr = kmalloc((npages + 2) * PAGE_SIZE, GFP_KERNEL)) == NULL) {
    ret = -ENOMEM;
    return ret;
  }
  /* round it up to the page bondary */
  kmalloc_area = (int *) ((((unsigned long) kmalloc_ptr) + PAGE_SIZE - 1)
      & PAGE_MASK);

  //  if (aslam_init_config_register() != 0) {
  //    ret = -EIO;
  //    aslam_cleanup_config_register();
  //    return ret;
  //  }

  //        /* allocate a memory area with vmalloc. */
  //        if ((vmalloc_area = (int *)vmalloc(NPAGES * PAGE_SIZE)) == NULL) {
  //                ret = -ENOMEM;
  //                goto out_kfree;
  //        }

  /* get the major number of the character device */
  if ((ret = alloc_chrdev_region(&aslam_dev, 0, 1, "slam-sensor")) < 0) {
    printk(KERN_ERR "could not allocate major number for aslam\n");
    //    vfree(vmalloc_area);
    kfree(kmalloc_ptr);
  }

  /* initialize the device structure and register the device with the kernel */
  cdev_init(&aslam_cdev, &aslam_fops);
  if ((ret = cdev_add(&aslam_cdev, aslam_dev, 1)) < 0) {
    printk(KERN_ERR "could not allocate chrdev for aslam\n");
    unregister_chrdev_region(aslam_dev, 1);
    //    vfree(vmalloc_area);
    kfree(kmalloc_ptr);
  }

  cl = class_create(THIS_MODULE, "<device class name>");
  device_create(cl, NULL, aslam_dev, NULL, "slam-sensor");

  /* mark the pages reserved */
  for (i = 0; i < npages * PAGE_SIZE; i+= PAGE_SIZE) {
    //                  SetPageReserved(vmalloc_to_page((void *)(((unsigned long)vmalloc_area) + i)));
    SetPageReserved(virt_to_page(((unsigned long)kmalloc_area) + i));
  }

  /* store a pattern in the memory - the test application will check for it */
  for (i = 0; i < (npages * PAGE_SIZE / sizeof(int)); i += 2) {
    //                vmalloc_area[i] = (0xaffe << 16) + i;
    //                vmalloc_area[i + 1] = (0xbeef << 16) + i;
    kmalloc_area[i] = (0xdead << 16) + i;
    kmalloc_area[i + 1] = (0xbeef << 16) + i;
  }

  bytes_in_memory = npages * PAGE_SIZE;
  printk("allocated %d bytes in memory\n", bytes_in_memory);
  return ret;
}

/* module unload */
static void aslam_exit(void) {
  int i;

  device_destroy(cl, aslam_dev);
  class_destroy(cl);

  /* remove the character deivce */
  cdev_del(&aslam_cdev);
  unregister_chrdev_region(aslam_dev, 1);

  /* unreserve the pages */
  for (i = 0; i < npages * PAGE_SIZE; i+= PAGE_SIZE) {
    //SetPageReserved(vmalloc_to_page((void *)(((unsigned long)vmalloc_area) + i)));
    SetPageReserved(virt_to_page(((unsigned long)kmalloc_area) + i));
  }
  /* free the memory areas */
  //vfree(vmalloc_area);
  kfree(kmalloc_ptr);

  //  aslam_cleanup_config_register();
}

module_init( aslam_init);
module_exit( aslam_exit);
MODULE_DESCRIPTION("aslam test driver");
MODULE_AUTHOR("Pascal J. Gohl");
MODULE_LICENSE("GPL");

