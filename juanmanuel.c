#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

static dev_t first; 		// Global variable for the first device number
static struct cdev c_dev; 	// Global variable for the character device structure
static struct class *cl; 	// Global variable for the device class

static char c;              // buffer en espacio de kernel

static int my_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "SdeC_drv4: open()\n");
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "SdeC_drv4: close()\n");
    return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "SdeC_drv4: read()\n");

    if (*off == 0)
    {
        if (copy_to_user(buf, &c, 1) != 0)
            return -EFAULT;
        else
        {
            (*off)++;
            return 1;
        }
    }
    else
        return 0;

}

// my_write escribe "len" bytes en "buf" y devuelve la cantidad de bytes escrita, 
// que debe ser igual "len".

// Cuando hago un $ echo "bla bla bla..." > /dev/SdeC_drv3, se convoca a my_write.!!

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "SdeC_drv4: write()\n");

    if ( copy_from_user(&c, buf + len - 1, 1) != 0 )
        return -EFAULT;
    else
        return len;
}

static struct file_operations pugs_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

static int __init drv4_init(void) /* Constructor */
{
    int ret;
    struct device *dev_ret;

    printk(KERN_INFO "Hi! I'm Juan Manuel, the best driver!\n");

    if ((ret = alloc_chrdev_region(&first, 0, 1, "SdeC_drv4")) < 0)
    {
        return ret;
    }

    if (IS_ERR(cl = class_create(THIS_MODULE, "SdeC_cdrv")))
    {
        unregister_chrdev_region(first, 1);
        return PTR_ERR(cl);
    }

    if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "SdeC_drv4")))
    {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return PTR_ERR(dev_ret);
    }

    cdev_init(&c_dev, &pugs_fops);
    if ((ret = cdev_add(&c_dev, first, 1)) < 0)
    {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return ret;
    }
    return 0;
}

static void __exit drv4_exit(void) /* Destructor */
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "YPF: Ya Paso Fangio!\n");
}

module_init(drv4_init);
module_exit(drv4_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Martín Exequiel Liborci - Iván David Reyes Romo");
MODULE_DESCRIPTION("Juan Manuel - The Best Driver");
MODULE_VERSION("Version 5.00");
MODULE_SUPPORTED_DEVICE("Raspberry Pi 4 B");
MODULE_INFO(parametro, "Temperatura/Humedad");