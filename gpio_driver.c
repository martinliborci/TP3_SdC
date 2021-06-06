#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
// ==================================================================================
include <linux/gpio.h>     // Raspberry Pi GPIO
// ==================================================================================
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Martín Exequiel Liborci - Iván David Reyes Romo");
MODULE_DESCRIPTION("Juan Manuel - The Best Driver");
MODULE_VERSION("F1.5T");
MODULE_SUPPORTED_DEVICE("Raspberry Pi 4 B");
MODULE_INFO(parametro, "Temperatura/Humedad");
// ==================================================================================
static dev_t first;         // Global variable for the first device number
static struct cdev c_dev;   // Global variable for the character device structure
static struct class *cl;    // Global variable for the device clas
static struct proc_dir_entry *proc_entry;

#define BUFFER_LENGTH       PAGE_SIZE
static char *clipboard;                     // Space for the "clipboard"
static char ver[] = "abcd"; 
// ----------------------------------------------------------------------------------
static struct gpio s2[] = { 
        { 18, GPIOF_IN, "s2_b4" },	// señal 2, botón 4
		    { 23, GPIOF_IN, "s2_b3" },
        { 24, GPIOF_IN, "s2_b2" },
        { 25, GPIOF_IN, "s2_b1" }
};

static struct gpio s1[] = { 
        { 12, GPIOF_IN, "s1_b4" },	// señal 1, botón 4
		{ 16, GPIOF_IN, "s1_b3" },
        { 20, GPIOF_IN, "s1_b2" },
        { 21, GPIOF_IN, "s1_b1" }	
};

static struct gpio *pSL = s1;

#define senal1 0
#define senal2 1
static char senalSelec = senal1;
//----------------------------------------------------------------------------------
static int my_open(struct inode *i, struct file *f)
  {
      printk(KERN_INFO "SdeC_drv4: open()\n");
      return 0;
  }
//----------------------------------------------------------------------------------
static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "SdeC_drv4: close()\n");
    return 0;
}
//----------------------------------------------------------------------------------
static ssize_t gpio_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) 
{
  // Al presionar una tecla se cambia de señal
    if(senalSelec == senal1)
        senalSelec = senal2;
    else
        senalSelec = senal1;

    printk(KERN_INFO "Se ha seleccionado la señal %d para sensar\n", senalSelec);
  return len;
}

//----------------------------------------------------------------------------------
static ssize_t gpio_read(struct file *filp, char __user *buf, size_t len, loff_t *off) 
{
  
    int nr_bytes;
    int i;
    char valorPin;

    if(senalSelec == senal1)
        pSL = s1;
    else
        pSL = s2;


     printk(KERN_INFO "Valor GPIO 12: ");
    for(i=0; i<4; i++){
        valorPin = gpio_get_value(pSL[i].gpio);
        printk(KERN_INFO "%d", valorPin);
        if(valorPin)
            ver[3-i] = '1';
        else
            ver[3-i] = '0';

    }
    ver[5] = '\0';
  
  printk(KERN_INFO "Ver GPIO 12: %s\n", ver);
    
 nr_bytes=strlen(ver);

  if ((*off) > 0) /* Tell the application that there is nothing left to read */
      return 0;
   
  if (len < nr_bytes)
    return -ENOSPC;
  
    /* Transfiere data desde el espacio de kernel al espacio de usuario */ 
    /* cat /proc/clipboard                                              */

  if (copy_to_user(buf, ver, nr_bytes))
    return -EINVAL;
    
  (*off)+=len;  /* Update the file pointer */

  return nr_bytes; 
}

//----------------------------------------------------------------------------------
 
static const struct file_operations dev_entry_fops = 
{
    .owner = THIS_MODULE,
    .read = gpio_read,
    .write = gpio_write, 
    .open = my_open,
    .release = my_close   
}; 

//----------------------------------------------------------------------------------


// En kernel mas nuevos se debe utilizar proc_ops para escribir en /proc
static struct proc_ops proc_entry_fops={
    .proc_read = gpio_read,
    .proc_write = gpio_write
};
 

//---------------------------------------------------------------------------------

int init_clipboard_module( void )
{
  int ret = 0;
  clipboard = (char *)vmalloc( BUFFER_LENGTH );

    struct device *dev_ret;

    printk(KERN_INFO "GPIO: Registrado exitosamente..!!\n");

    if ((ret = alloc_chrdev_region(&first, 0, 1, "gpio_kernel")) < 0)
    {
        return ret;
    }

    if (IS_ERR(cl = class_create(THIS_MODULE, "gpio_class")))
    {
        unregister_chrdev_region(first, 1);
        return PTR_ERR(cl);
    }

    if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "gpio_dev")))
    {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return PTR_ERR(dev_ret);
    }

    cdev_init(&c_dev, &dev_entry_fops);
    if ((ret = cdev_add(&c_dev, first, 1)) < 0)
    {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return ret;
    }
  if (!clipboard) {
    ret = -ENOMEM;
  } else {

    memset( clipboard, 0, BUFFER_LENGTH );
    proc_entry = proc_create( "gpio_proc", 0666, NULL, &proc_entry_fops);
    if (proc_entry == NULL) {
      ret = -ENOMEM;
      vfree(clipboard);
      printk(KERN_INFO "gpio_proc: No puede crear entrada en /proc..!!\n");
    } else {
      printk(KERN_INFO "gpio_proc: Modulo cargado en el kernel..!!\n");
    }
  }

  // Se reservan los pines a utilizar
    ret = gpio_request_array(s1, ARRAY_SIZE(s1));
    if (ret){ // Error
		printk(KERN_ERR "Error en gpio_request_array para s1: %d\n", ret);
		gpio_free_array(s1, ARRAY_SIZE(s1)); // Se liberan los pines
	}

  return ret;
}

//----------------------------------------------------------------------------------
void exit_clipboard_module( void )
{

    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
  cdev_del(&c_dev);
  remove_proc_entry("gpio_proc", NULL);
  vfree(clipboard);
  printk(KERN_INFO "Clipboard: Adios mundo kernel..!!\n");
  gpio_free_array(s1, ARRAY_SIZE(s1)); // Se liberan los pines reservados
}

module_init( init_clipboard_module );
module_exit( exit_clipboard_module );

