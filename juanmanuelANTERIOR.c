// ==================================================================================
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>

#include <linux/random.h>

// ----------------------------------------------------------------------------------
#include <linux/gpio.h>     // Raspberry Pi GPIO
// ==================================================================================
static dev_t devID; 		// dev_t se compone del major y minor. Identifica al dispositivo
static struct cdev charDev; 	// Estructura de datos interna al kernel que representa un dispositivo de caracteres.
                            // Intermente almacena el dev_t que identifica al dispositivo
static struct class *devClass; 	// Para que las aplicaciones de usuario puedan utilizar el dispositivo 
                            // es necesario crear un fichero especial de tipo dispositivo de caracteres 
                            // dentro del sistema de ficheros (/dev/miCatangaF1User)
static char c;     // buffer en espacio de kernel
// ----------------------------------------------------------------------------------
/*
static struct gpio s2[] = { 
        { 18, GPIOF_IN, "s2_b4" },	// señal 2, botón 4
		{ 23, GPIOF_IN, "s2_b3" },
        { 24, GPIOF_IN, "s2_b2" },
        { 25, GPIOF_IN, "s2_b1" }
};
*/
static struct gpio s1[] = { 
        { 12, GPIOF_IN, "s1_b4" },	// señal 1, botón 4
		{ 16, GPIOF_IN, "s1_b3" },
        { 20, GPIOF_IN, "s1_b2" },
        { 21, GPIOF_IN, "s1_b1" }	
};
#define senal1 0
#define senal2 1
static char senalSelec;
// ==================================================================================
// 
// Vincula fd de nivel de usuario con el inodo de nivel de kernel
static int my_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Juan Manuel: open()\n");
    return 0;
}
// ----------------------------------------------------------------------------------
static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Juan Manuel: close()\n");
    return 0;
}
// ==================================================================================
// Lee el estado del dispositivo
// Esta función se llama cuando se ejecuta fread() a nivel de usuario
// Copia n bytes del espacio de kernel al espacio de ususario
// f: archivo en /dev/ asociado al dispositivo, es desde donde se va a leer su estado
// buff: buffer de nivel de usuario donde se copiarán los datos leídos
// len: cantidad de chars a escribir
// off: offset, posición en el archivo desde la cual se comenzará a leer los datos
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
     int i;

    printk(KERN_INFO "Juan Manuel: read()\n");

    c = 0;

    gpio_get_value(s1[0].gpio);
    printk(KERN_INFO "Valor leido: %#.2X\n",s1[0].gpio);
    c = (s1[0].gpio);
     printk(KERN_INFO "Valor leido: %#.2X\n",c);
    /*
    for(i=4; i < 8; i++){
        gpio_get_value(s1[i].gpio);
        c |= (s1[i].gpio)<<(i-4);    // Valor leído de la señal seleccionada a pasar al espacio de usuario
    }
    */
    
    get_random_bytes(&c,sizeof(char));
    printk(KERN_INFO "Valor leido: %#.2X\n",c);

    // unsigned long __copy_to_user (	void __user * to, const void * from, unsigned long n);
    // to: dirección de destino en el espacio de usuario
    // from: dirección de origen en el espacio de kernel
    // n: cantidad de bytes a copiar
    // (*off) = 0;
    copy_to_user(buf, &c, 1);
    return 0;
    /*
    if (*off == 0) {
        if (copy_to_user(buf, &c, 1) != 0){
            printk(KERN_INFO "Excepción EFAULT\n");
            return -EFAULT;
        }
        else {
            (*off)++;
            return 1;
        }
    }
    else
        return 0;
    */
}
// ----------------------------------------------------------------------------------
// my_write escribe "len" bytes en "buf" y devuelve la cantidad de bytes escrita, 
// que debe ser igual "len".
// Cuando hago un $ echo "bla bla bla..." > /dev/SdeC_drv3, se convoca a my_write.!!
// $ sudo su 
// # echo -n "M" > /dev/miCatangaF1Sysfs
// exit
// sudo cat /dev/miCatangaF1Sysfs
// Estribir en el dispositivo
// Esta función se llama cuando se ejecuta fwrite() a nivel de usuario
// Copia n bytes del espacio de usuario al espacio de kernel
// f: archivo en /dev/ asociado al dispositivo donde se va a escribir
// buff: buffer de nivel de usuario desde donde se leerán los datos a escribir
// len: cantidad de chars a escribir
// off: offset, posición en el archivo desde la cual se comenzará a escribir los datos
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Juan Manuel: write()\n");

    // Al presionar una tecla se cambia de señal
    if(senalSelec == senal1)
        senalSelec = senal2;
    else
        senalSelec = senal1;

    printk(KERN_INFO "Se ha seleccionado la señal %d para sensar\n", senalSelec);
/*
    // unsigned long __copy_from_user (void * to, const void __user * from, unsigned long n);
    // to: dirección de destino en el espacio de kernel
    // from: dirección de origen en el espacio de usuario
    // n: cantidad de bytes a copiar
    if ( copy_from_user(&c, buf + len - 1, 1) != 0 )
        return -EFAULT;
    else
        return len;
*/
    return len;
}
// ==================================================================================
static struct file_operations pugs_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};
// ==================================================================================
// Esta función se llama cuando se carga el módulo con el comando insmod
// Realiza las inicializaciones preliminares del módulo para ser utilizado
static int __init juanmanuel_init(void) /* Constructor */
{
    int ret;
    struct device *registedDev; // Estructura básica del dispositivo

    printk(KERN_INFO "Hi! I'm Juan Manuel, the best driver!\n"); // Mensaje en el log del kernel

    // Reserva de los números major y minor asociados al dispositivo
    // major: identifica al manejador
    // minor: identifica al dispositivo concreto entre los que gestiona ese manejador
    // dev_t devID: almacenará el identificador del dispositivo dentro del núcleo.
    //              Internamente, está compuesto por los valores major y minor asociados a ese dispositivo.
    // El major lo elige el SO
    // 0: primer minor que solicitamos 
    // 1: cantidad de números minor que se quieren reservar
    // catangaF1: nombre del dispositivo
    if ((ret = alloc_chrdev_region(&devID, 0, 1, "miCatangaF1Kernel")) < 0){  // SdeC_drv4 => miCatangaF1Kernel XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        printk(KERN_INFO "Error en alloc_chrdev_region()\n");
        return ret;
    }

    // Crear una clase para el dispositivo gestionado por el manejador
    // catangaF1Class: nombre de la clase
    // THIS_MODULE: driver que gestiona este tipo de dispositivos
    if (IS_ERR(devClass = class_create(THIS_MODULE, "catangaF1Class"))){ // SdeC_cdrv => catangaF1Class XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
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
    if (IS_ERR(registedDev = device_create(devClass, NULL, devID, NULL, "miCatangaF1Sysfs"))){  // SdeC_drv4 => miCatangaF1Sysfs XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        unregister_chrdev_region(devID, 1); // En caso de error, se liberan los major y minors
        return PTR_ERR(registedDev);
    }

    // Inicializar la estructura charDev que representa un dispositivo de caracteres
    // pugs_fops: conjunto de funciones de acceso que proporciona el dispositivo
    cdev_init(&charDev, &pugs_fops);
    // Asociar charDev con el identificador de dispositivo devID reservado previamente
    // 1: cantidad de números menores consecutivos correspondientes a este dispositivo
    if ((ret = cdev_add(&charDev, devID, 1)) < 0){
        device_destroy(devClass, devID);
        class_destroy(devClass);
        unregister_chrdev_region(devID, 1);
        return ret;
    }

    // Se reservan los pines a utilizar
    ret = gpio_request_array(s1, ARRAY_SIZE(s1));
    if (ret){ // Error
		printk(KERN_ERR "Error en gpio_request_array para s1: %d\n", ret);
		gpio_free_array(s1, ARRAY_SIZE(s1)); // Se liberan los pines
	}



    senalSelec = senal1; // La señal seleccionada por defecto es la señal 1
    return 0;
}
// ----------------------------------------------------------------------------------
// Esta función se llama cuando se descarga el módulo del kernel con el comando rmmod
// Realiza las operaciones inversas realizadas por init_module al cargarse el módulo
static void __exit juanmanuel_exit(void) /* Destructor */
{
    cdev_del(&charDev);
    device_destroy(devClass, devID);
    class_destroy(devClass);
    unregister_chrdev_region(devID, 1);
    gpio_free_array(s1, ARRAY_SIZE(s1)); // Se liberan los pines reservados
    printk(KERN_INFO "YPF Ya Paso Fangio!\n");
}
// ----------------------------------------------------------------------------------
module_init(juanmanuel_init); // Renombra init_module
module_exit(juanmanuel_exit); // Renombra cleanup_module
// ==================================================================================
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Martín Exequiel Liborci - Iván David Reyes Romo");
MODULE_DESCRIPTION("Juan Manuel - The Best Driver");
MODULE_VERSION("F1.5T");
MODULE_SUPPORTED_DEVICE("Raspberry Pi 4 B");
MODULE_INFO(parametro, "Temperatura/Humedad");
// ==================================================================================