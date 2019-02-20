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
#define BINARY_LETTER_BUFFER_SIZE 50

// =============================== LOCAL VARIABLES ============================
static unsigned int gpioOutput = 22; // Output pin has default value 22 but can be changed to something else when loading the module
static unsigned int basePeriod = 100; // Preriod of the smallest interval. Default value is 100 but this can be changed at load time or later

volatile static bool hasNewMessage = false; // flow control variable. Indicated that a new message has been converted and is ready to be sent 
volatile static bool doneSendingMessage = true; // flow control variable. Indicated that the message has been sent
static char moduleName[15];

static char asciiMessage[ASCII_MESSAGE_SIZE]; // buffer for ascii message
static char binaryMessage[BINARY_MESSAGE_SIZE];
static char binaryLetterBuffer[BINARY_LETTER_BUFFER_SIZE];

// =============================== PARAMETERS =================================

module_param(gpioOutput, uint, S_IRUGO);
MODULE_PARM_DESC(gpioOutput, " Output GPIO number (default=22)");


module_param(basePeriod, uint, S_IRUGO);
MODULE_PARM_DESC(basePeriod, " ti period in ms (min=1, default=100, max=1000)");

// =============================== PROTOTYPES =================================

static void conversion_ascii_binaire(void);
static void convert(char letter);

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
static ssize_t asciiMessage_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
   printk(KERN_INFO "MORSE SEND: Entered asciiMessage_store \n");

   strcpy(asciiMessage, buf);
   printk(KERN_INFO "MORSE SEND: ascii message: %s\n", asciiMessage);

   while (!doneSendingMessage) { // wait for message to finish being sent 
      msleep(basePeriod);
   }
   doneSendingMessage = false; // reset the flag just after

   conversion_ascii_binaire();
   printk(KERN_INFO "MORSE SEND: binary message: %s\n", binaryMessage);
   hasNewMessage = true; // signal the transmission_task that thre's new stuff to send

   return asciiMessage;
}

// transform some of the above variables in kobject attributes so that they can be read/written later when the module is loaded
static struct kobj_attribute period_attr =       __ATTR(basePeriod, 0660, period_show, period_store);
// static struct kobj_attribute asciiMessage_attr = __ATTR(asciiMessage, 0660, asciiMessage_show, asciiMessage_store);
static struct kobj_attribute asciiMessage_attr = __ATTR_WO(asciiMessage);

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

static struct task_struct *transmission_task; //  pointer to the thread transmission_task

// the transmission_task main loop. This loops waits for a message and sends it as soon as possible before waiting agin
static int send(void *arg){
   char symbol;
   int counter;

   printk(KERN_INFO "MORSE SEND: Thread has started running \n");

   while(!kthread_should_stop()){           // Returns true when kthread_stop() is called
      set_current_state(TASK_RUNNING);

      if (hasNewMessage) {
         printk(KERN_INFO "MORSE SEND: New message to send: %s\n", binaryMessage);
         hasNewMessage = false;
         doneSendingMessage = false;

         symbol = binaryMessage[0];
         counter = 0;

         while (symbol != 0) {
            gpio_set_value(gpioOutput, symbol - '0');
            symbol = binaryMessage[++counter];
            msleep(basePeriod);
         }
         printk(KERN_INFO "MORSE SEND: done sending message\n");
         doneSendingMessage = true;
      }

      set_current_state(TASK_INTERRUPTIBLE);
      msleep(basePeriod/2);                // millisecond sleep for half of the period
   }

   printk(KERN_INFO "MORSE SEND: Thread has run to completion \n");
return 0;
}

// =============================== INIT =======================================

// initialization function
static int __init morseSend_init(void){
   int result = 0;

   printk(KERN_INFO "MORSE SEND: Initializing MORSE SEND kernel module\n");
   sprintf(moduleName, "morseSend");

   kobj = kobject_create_and_add("morseSend", kernel_kobj); // kernel_kobj points to /sys/kernel
   if(!kobj){
      printk(KERN_ALERT "MORSE SEND: failed to create kobject\n");
      return -ENOMEM;
   }
   // add the attributes to /sys/kernel/morse/morseSend
   result = sysfs_create_group(kobj, &attr_group);
   if(result) {
      printk(KERN_ALERT "MORSE SEND: failed to create sysfs group\n");
      kobject_put(kobj);                // clean up -- remove the kobject sysfs entry
      return result;
   }

   gpio_request(gpioOutput, "sysfs");          // request gpioOutput
   gpio_direction_output(gpioOutput, 0);       // Set the gpio to be in output mode and turn off

   transmission_task = kthread_run(send, NULL, "send_thread");  // Start the LED flashing thread
   if(IS_ERR(transmission_task)){                                     // Kthread name is LED_flash_thread
      printk(KERN_ALERT "MORSE SEND: failed to create the transmission_task\n");
      return PTR_ERR(transmission_task);
   }
   return result;
}

// =============================== EXIT =======================================

// Cleanup function
static void __exit morseSend_exit(void){
   kthread_stop(transmission_task);                         // Stop the transmission thread
   kobject_put(kobj);                      // clean up -- remove the kobject sysfs entry
   gpio_set_value(gpioOutput, 0);
   gpio_unexport(gpioOutput);                  // Unexport the output GPIO
   gpio_free(gpioOutput);                      // Free the input GPIO
   printk(KERN_INFO "MORSE SEND: Goodbye from the MORSE SEND kernel module!\n");
}

// identify the initialization and cleanup function
module_init(morseSend_init);
module_exit(morseSend_exit);

// =============================== PRIVATE ====================================

//Fonction_Convert
static void convert(char letter){
   letter = toupper(letter);
   memset(binaryLetterBuffer, 0, BINARY_LETTER_BUFFER_SIZE); // clear buffer
   printk(KERN_INFO "MORSE SEND: adding letter: %c\n", letter);

   switch(letter){
      case '0': strcpy(binaryLetterBuffer, "1110111011101110111000"); return;
      case '1': strcpy(binaryLetterBuffer, "10111011101110111000"); return;
      case '2': strcpy(binaryLetterBuffer, "101011101110111000"); return;
      case '3': strcpy(binaryLetterBuffer, "1010101110111000"); return;
      case '4': strcpy(binaryLetterBuffer, "10101010111000"); return;
      case '5': strcpy(binaryLetterBuffer, "101010101000"); return;
      case '6': strcpy(binaryLetterBuffer, "11101010101000"); return;
      case '7': strcpy(binaryLetterBuffer, "1110111010101000"); return;
      case '8': strcpy(binaryLetterBuffer, "111011101110101000"); return;
      case '9': strcpy(binaryLetterBuffer, "11101110111011101000"); return;
      case 'A': strcpy(binaryLetterBuffer, "10111000"); return;
      case 'B': strcpy(binaryLetterBuffer, "111010101000"); return; 
      case 'C': strcpy(binaryLetterBuffer, "111011101000"); return;
      case 'D': strcpy(binaryLetterBuffer, "1110101000"); return;
      case 'E': strcpy(binaryLetterBuffer, "1000"); return;
      case 'F': strcpy(binaryLetterBuffer, "101011101000"); return;
      case 'G': strcpy(binaryLetterBuffer, "111011101000"); return;
      case 'H': strcpy(binaryLetterBuffer, "1010101000"); return;
      case 'I': strcpy(binaryLetterBuffer, "101000"); return;
      case 'J': strcpy(binaryLetterBuffer, "1011101110111000"); return;
      case 'K': strcpy(binaryLetterBuffer, "111010111000"); return;
      case 'L': strcpy(binaryLetterBuffer, "101110101000"); return;
      case 'M': strcpy(binaryLetterBuffer, "1110111000"); return;
      case 'N': strcpy(binaryLetterBuffer, "11101000"); return;
      case 'O': strcpy(binaryLetterBuffer, "11101110111000"); return;
      case 'P': strcpy(binaryLetterBuffer, "10111011101000"); return;
      case 'Q': strcpy(binaryLetterBuffer, "1110111010111000"); return;
      case 'R': strcpy(binaryLetterBuffer, "1011101000"); return;
      case 'S': strcpy(binaryLetterBuffer, "10101000"); return;
      case 'T': strcpy(binaryLetterBuffer, "111000"); return;
      case 'U': strcpy(binaryLetterBuffer, "1010111000"); return;
      case 'V': strcpy(binaryLetterBuffer, "101010111000"); return;
      case 'W': strcpy(binaryLetterBuffer, "101110111000"); return;
      case 'X': strcpy(binaryLetterBuffer, "11101010111000"); return;
      case 'Y': strcpy(binaryLetterBuffer, "1110101110111000"); return;
      case 'Z': strcpy(binaryLetterBuffer, "11101110101000"); return;
      case ' ': strcpy(binaryLetterBuffer, "000000"); return;
      default : strcpy(binaryLetterBuffer, ""); return;
   }

}

// convert the ascii message to "binary"
static void conversion_ascii_binaire(void) {
   int i;
   memset(binaryMessage, 0, BINARY_MESSAGE_SIZE);

   for (i = 0 ; i < strlen(asciiMessage) ; ++i){
      convert(asciiMessage[i]);
      strcat(binaryMessage, binaryLetterBuffer);
   }
   memset(asciiMessage, 0, ASCII_MESSAGE_SIZE);
}


