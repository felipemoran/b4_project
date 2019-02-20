#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/kobject.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Felipe Morán and Ayoub Bounaga");
MODULE_DESCRIPTION("Linux driver for sending Morse messages");
MODULE_VERSION("1.0");


// =============================== DEFINES ====================================
#define BINARY_MESSAGE_SIZE 500
#define ASCII_MESSAGE_SIZE 100

// =============================== LOCAL VARIABLES ============================
static unsigned int gpioInput = 10; // Inout pin has default value 10 but can be changed to something else when loading the module
static unsigned int basePeriod = 100; // Preriod of the smallest interval. Default value is 100 but this can be changed at load time or later

static char moduleName[15];

static char asciiMessage[ASCII_MESSAGE_SIZE]; // buffer for ascii message
static char binaryMessage[BINARY_MESSAGE_SIZE];

// =============================== PARAMETERS =================================

module_param(gpioInput, uint, S_IRUGO);
MODULE_PARM_DESC(gpioInput, " Input GPIO number (default=10)");


module_param(basePeriod, uint, S_IRUGO);
MODULE_PARM_DESC(basePeriod, " ti period in ms (min=1, default=100, max=1000)");

// =============================== PROTOTYPES =================================

static char* conversion_binaire_ascii(char* message_binaire , int* tab_duree , int duree_impulsion, int* compteur)
static char* switch_to_letter(char* tab)

// =============================== KOBJECTS SETUP =============================

// callback functio to display the base period 
static ssize_t period_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   return sprintf(buf, "%d\n", basePeriod);
}

// callback functio to store a new value for the base period 
static ssize_t period_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
   unsigned int period;                     // Using a variable to validate the data sent
   sscanf(buf, "%du", &period);             // Read in the period as an unsigned int
   if ((period>=1)&&(period<=1000)){        // Must be 1ms or greater, 1sec or less
      basePeriod = period;                  // Within range, assign to basePeriod variable
   }
   return period;
}

/** @brief A callback function to store the LED period value */
static ssize_t asciiMessage_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   int ret;
   ret = sprintf(buf, "%s\n", asciiMessage);
   memset(asciiMessage, 0, ASCII_MESSAGE_SIZE);
}

// transform some of the above variables in kobject attributes so that they can be read/written later when the module is loaded
static struct kobj_attribute period_attr =       __ATTR(basePeriod, 0660, period_show, period_store);
// static struct kobj_attribute asciiMessage_attr = __ATTR(asciiMessage, 0660, asciiMessage_show, asciiMessage_store);
static struct kobj_attribute asciiMessage_attr = __ATTR_RO(asciiMessage);

// array that groupd the parameters/attributes listed above
static struct attribute *moduleAttrs[] = {
   &period_attr.attr,
   &asciiMessage_attr.attr,
   NULL,
};

// contains the name of the module exposed to sysfs and the parameters set above
static struct attribute_group attr_group = {
   .name  = moduleName,                     // The name is generated in morseSend_init()
   .attrs = moduleAttrs,                    // The attributes array defined just above
};

static struct kobject *kobj; // The pointer to the kobject

// =============================== TASK SETUP =================================

// =============================== INIT =======================================

// initialization function
static int __init morseSend_init(void){
   int result = 0;

   printk(KERN_INFO "MORSE RECEIVE: Initializing MORSE RECEIVE kernel module\n");
   sprintf(moduleName, "morseReceive");

   kobj = kobject_create_and_add("morse", kernel_kobj); // kernel_kobj points to /sys/kernel
   if(!kobj){
      printk(KERN_ALERT "MORSE RECEIVE: failed to create kobject\n");
      return -ENOMEM;
   }
   // add the attributes to /sys/kernel/morse/morseReceive
   result = sysfs_create_group(kobj, &attr_group);
   if(result) {
      printk(KERN_ALERT "MORSE RECEIVE: failed to create sysfs group\n");
      kobject_put(kobj);                // clean up -- remove the kobject sysfs entry
      return result;
   }

   gpio_request(gpioInput, "sysfs");          // request gpioInput
   gpio_direction_input(gpioInput);       // Set the gpio to be in output mode and turn off

   transmission_task = kthread_run(send, NULL, "send_thread");  // Start the LED flashing thread
   if(IS_ERR(transmission_task)){                                     // Kthread name is LED_flash_thread
      printk(KERN_ALERT "MORSE RECEIVE: failed to create the transmission_task\n");
      return PTR_ERR(transmission_task);
   }
   return result;
}

// =============================== EXIT =======================================

// Cleanup function
static void __exit morseSend_exit(void){
   kthread_stop(transmission_task);                         // Stop the transmission thread
   kobject_put(kobj);                      // clean up -- remove the kobject sysfs entry
   gpio_set_value(gpioInput, 0);
   gpio_unexport(gpioInput);                  // Unexport the output GPIO
   gpio_free(gpioInput);                      // Free the input GPIO
   printk(KERN_INFO "MORSE RECEIVE: Goodbye from the MORSE RECEIVE kernel module!\n");
}

// identify the initialization and cleanup function
module_init(morseSend_init);
module_exit(morseSend_exit);

// =============================== PRIVATE ====================================
