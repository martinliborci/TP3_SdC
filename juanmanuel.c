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
#include <linux/gpio.h>     // Raspberry Pi GPIO
// ==================================================================================
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Martín Exequiel Liborci - Iván David Reyes Romo");
MODULE_DESCRIPTION("Juan Manuel - The Best Driver");
MODULE_VERSION("F1.5T");
MODULE_SUPPORTED_DEVICE("Raspberry Pi 4 B");
// ==================================================================================
static dev_t devID;           // dev_t se compone del major y minor. Identifica al dispositivo
static struct cdev charDev;   // Estructura de datos interna al kernel que representa un dispositivo de caracteres.
                              // Intermente almacena el dev_t que identifica al dispositivo
static struct class *devClass;// Identifica la clase de dispositivo que se va a crear
static struct proc_dir_entry *procEntry; // Estructura usada para crear la entrada del dispositivo en /proc

static char senalString[] = "abcd"; // String que contiene bits leídos y que se pasará al espacio de usuario 
// ==================================================================================
static struct gpio s2[] = { 
        { 18, GPIOF_IN, "s2_b4" },	// {GPIO#, ModoEntrada, señal#_boton# }
		    { 23, GPIOF_IN, "s2_b3" },  // señal 2, botón 3
        { 24, GPIOF_IN, "s2_b2" },
        { 25, GPIOF_IN, "s2_b1" }
};
//----------------------------------------------------------------------------------
static struct gpio s1[] = { 
        { 12, GPIOF_IN, "s1_b4" },	// señal 1, botón 4
		    { 16, GPIOF_IN, "s1_b3" },
        { 20, GPIOF_IN, "s1_b2" },
        { 21, GPIOF_IN, "s1_b1" }	
};
//----------------------------------------------------------------------------------
#define senal1 0
#define senal2 1
static char senalSelec = senal1; // Identificador de la señal que actualmente se está leyendo
static struct gpio *pSL = s1;    // Puntero a los pines de entrada de la señal que actualmente se está leyendo
// ==================================================================================
// Es llamada cuando se abre el archivo a nivel de usuario
// Vincula fd de nivel de usuario con el inodo de nivel de kernel
static int my_open(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Juan Manuel: open()\n");
  return 0;
}
//----------------------------------------------------------------------------------
// Es llamada cuando se cierra el archivo a nivel de usuario
static int my_close(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Juan Manuel: close()\n");
  return 0;
}
// ==================================================================================
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

  // Seleccionar el conjunto de pines a leer
  if(senalSelec == senal1)
      pSL = s1;
  else
      pSL = s2;

  // Leer los pines de la señal seleccionada
  for(i=0; i<4; i++){
      valorPin = gpio_get_value(pSL[i].gpio);
      printk(KERN_INFO "%d", valorPin);
      if(valorPin)
          senalString[3-i] = '1';
      else
          senalString[3-i] = '0';
  }
  senalString[5] = '\0';

  nr_bytes = strlen(senalString);

  if ((*off) > 0) /* Tell the application that there is nothing left to read */
      return 0;

  if (len < nr_bytes) // Si el buffer es más chico que el string
    return -ENOSPC;

  // Transfiere data desde el espacio de kernel al espacio de usuario
  if (copy_to_user(buf, senalString, nr_bytes))
    return -EINVAL;

  (*off) += len;  // Actualizar el puntero del archivo escrito

  return nr_bytes; 
}
// ==================================================================================
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
// ==================================================================================
// Esta función se llama cuando se carga el módulo con el comando insmod
// Realiza las inicializaciones preliminares del módulo para ser utilizado
int juanmanuel_init( void )
{
  int ret = 0;
  struct device *dev_ret;

  printk(KERN_INFO "Hi! I'm Juan Manuel, the best driver!\n");

  // Reserva de los números major y minor asociados al dispositivo
  // major: identifica al manejador
  // minor: identifica al dispositivo concreto entre los que gestiona ese manejador
  // dev_t devID: almacenará el identificador del dispositivo dentro del núcleo.
  //              Internamente, está compuesto por los valores major y minor asociados a ese dispositivo.
  // El major lo elige el SO
  // 0: primer minor que solicitamos 
  // 1: cantidad de números minor que se quieren reservar
  // catangaF1: nombre del dispositivo
  if ((ret = alloc_chrdev_region(&devID, 0, 1, "miCatangaF1Kernel")) < 0){
      printk(KERN_INFO "Error en alloc_chrdev_region()\n"); 
      return ret;
    }

  // Crear una clase para el dispositivo gestionado por el manejador
  // catangaF1Class: nombre de la clase
  // THIS_MODULE: driver que gestiona este tipo de dispositivos
  if (IS_ERR(devClass = class_create(THIS_MODULE, "catangaF1Class"))){
      printk(KERN_INFO "Error en class_create()\n");
      unregister_chrdev_region(devID, 1); // En caso de error, se liberan los major y minors
      return PTR_ERR(devClass);
  }

  // crea un dispositivo de la clase catangaF1Class y lo registra con sysfs con nombre miCatangaF1Sysfs
  // devClass: clase de dispositivo que se va a crear
  // devID: mayor y minor que identifica el dispositivo concreto
  // miCatangaF1Sysfs: nombre del fichero del dispositivo con el que figurará en /dev/
  // A partir de este momento, se habrá creado el fichero /dev/seq y las llamadas de apertura, lectura, escritura, 
  // cierre, etc. sobre ese fichero especial son redirigidas por el SO a las funciones de acceso 
  // correspondientes exportadas por el manejador del dispositivo.
  if (IS_ERR(dev_ret = device_create(devClass, NULL, devID, NULL, "miCatangaF1Sysfs"))){
      class_destroy(devClass);
      unregister_chrdev_region(devID, 1);
      return PTR_ERR(dev_ret);
  }

  // Inicializar la estructura charDev que representa un dispositivo de caracteres
  // pugs_fops: conjunto de funciones de acceso que proporciona el dispositivo
  cdev_init(&charDev, &dev_entry_fops);
  if ((ret = cdev_add(&charDev, devID, 1)) < 0){
      device_destroy(devClass, devID);
      class_destroy(devClass);
      unregister_chrdev_region(devID, 1);
      return ret;
  }

  // Se crea la entrada en el /proc
  procEntry = proc_create( "miCatangaF1Proc", 0666, NULL, &proc_entry_fops);
  if (procEntry == NULL) {
      ret = -ENOMEM;
      printk(KERN_INFO "miCatangaF1Proc: No se puede crear entrada en /proc..!!\n");
  }

  // Se reservan los pines a utilizar por la Señal 1
  ret = gpio_request_array(s1, ARRAY_SIZE(s1));
  if (ret){ // Error
    printk(KERN_ERR "Error en gpio_request_array para s1: %d\n", ret);
    gpio_free_array(s1, ARRAY_SIZE(s1)); // Se liberan los pines
  }

  // Se reservan los pines a utilizar por la Señal 2
  ret = gpio_request_array(s2, ARRAY_SIZE(s2));
  if (ret){ // Error
    printk(KERN_ERR "Error en gpio_request_array para s1: %d\n", ret);
    gpio_free_array(s2, ARRAY_SIZE(s1)); // Se liberan los pines
  }

  return ret;
}
//----------------------------------------------------------------------------------
// Esta función se llama cuando se descarga el módulo del kernel con el comando rmmod
// Realiza las operaciones inversas realizadas por init_module al cargarse el módulo
void juanmanuel_exit( void )
{
  device_destroy(devClass, devID);
  class_destroy(devClass);
  unregister_chrdev_region(devID, 1);
  cdev_del(&charDev);
  remove_proc_entry("miCatangaF1Proc", NULL);
  printk(KERN_INFO "YPF Ya Paso Fangio!\n");
  gpio_free_array(s1, ARRAY_SIZE(s1)); // Se liberan los pines reservados para Señal 1
  gpio_free_array(s2, ARRAY_SIZE(s2)); // Se liberan los pines reservados para Señal 1
}
//==================================================================================
module_init( juanmanuel_init ); // Renombra init_module
module_exit( juanmanuel_exit ); // Renombra cleanup_module
//==================================================================================
