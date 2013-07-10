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
    .write = aslam_write,
    .owner = THIS_MODULE, };

// internal data
// length of the memory area
#define IMAGE_SIZE 360960 //=752*480
static int npages = IMAGE_SIZE * 3 / PAGE_SIZE;

#define REGBASE 0x41200000

// pointer to the vmalloc'd area - alway page aligned
static int *vmalloc_area;
// pointer to the kmalloc'd area, rounded up to a page boundary
static int *kmalloc_area;
// pointer to the device register area
void __iomem  *config_register;
// original pointer for kmalloc'd area as returned by kmalloc
static void *kmalloc_ptr;

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

static int aslam_read(struct file *filp, char *buf,
  size_t count, loff_t *f_pos) {

  /* Buffer to read the device */
  char parlelport_buffer;

  readl(0x00000002, config_register + 0x0004);

  /* We transfer data to user space */
  copy_to_user(buf,&parlelport_buffer,1);

  /* We change the reading position as best suits */
  if (*f_pos == 0) {
    *f_pos+=1;
    return 1;
  } else {
    return 0;
  }
}

static int aslam_write( struct file *filp, char *buf,
  size_t count, loff_t *f_pos) {

  char *tmp;

  /* Buffer writing to the device */
  char parlelport_buffer;

  tmp=buf+count-1;
  copy_from_user(&parlelport_buffer,tmp,1);

  while (count--) {
      writeb(*(ptr++), address);
      wmb();
  }

  return 1;
}

static int aslam_init_register()
{
  // map the config register to pointer
  config_register = ioremap(REGBASE, 16);
}

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

  aslam_init_register();
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

  printk("allocated %d bytes in memory\n", (npages + 2) * PAGE_SIZE);
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
}

module_init( aslam_init);
module_exit( aslam_exit);
MODULE_DESCRIPTION("aslam test driver");
MODULE_AUTHOR("Pascal J. Gohl");
MODULE_LICENSE("GPL");
