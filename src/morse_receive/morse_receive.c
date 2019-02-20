#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/kobject.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/timekeeping.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>            // Required for the IRQ code


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Felipe Morán and Ayoub Bounaga");
MODULE_DESCRIPTION("Linux driver for sending Morse messages");
MODULE_VERSION("1.0");


// =============================== DEFINES ====================================
#define BINARY_MESSAGE_SIZE 500
#define ASCII_MESSAGE_SIZE 100
#define LETTER_BUFFER_SIZE 2
#define TITAH_BUFFER_SIZE 20

// =============================== LOCAL VARIABLES ============================
static unsigned int gpioInput = 10; // Inout pin has default value 10 but can be changed to something else when loading the module
static unsigned int basePeriod = 100; // Preriod of the smallest interval. Default value is 100 but this can be changed at load time or later

static char moduleName[15];
static unsigned int irqNumber;

static char asciiMessage[ASCII_MESSAGE_SIZE]; // buffer for ascii message
static char binaryMessageSimbol[BINARY_MESSAGE_SIZE];
static char binaryMessageDuration[BINARY_MESSAGE_SIZE];
static char letterBuffer[LETTER_BUFFER_SIZE];
static char titahBuffer[TITAH_BUFFER_SIZE];
volatile static unsigned int binaryIndex;
volatile static unsigned int asciiIndex;

static unsigned int lastInterrupt_ms;
static unsigned int thisInterrupt_ms;
static bool isFirstInterrupt = true;

// =============================== PARAMETERS =================================

module_param(gpioInput, uint, S_IRUGO);
MODULE_PARM_DESC(gpioInput, " Input GPIO number (default=10)");


module_param(basePeriod, uint, S_IRUGO);
MODULE_PARM_DESC(basePeriod, " ti period in ms (min=1, default=100, max=1000)");

// =============================== PROTOTYPES =================================

static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

static void convertBinaryToAscii(void);
static void titahToLetter(void);

// timespect manipulation helper functions
int timespec_to_ms(struct timespec* time_ts);
void timespec_now(struct timespec* now);
struct timespec timespec_subtract(struct timespec* time1_ts, struct timespec* time2_ts);

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
   ssize_t ret;
   ret = sprintf(buf, "%s\n", asciiMessage);
   memset(asciiMessage, 0, ASCII_MESSAGE_SIZE);

   return ret;
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

   binaryIndex = 0;
   asciiIndex = 0;

   printk(KERN_INFO "MORSE RECEIVE: Initializing MORSE RECEIVE kernel module\n");
   sprintf(moduleName, "morseReceive");

   kobj = kobject_create_and_add("morseReceive", kernel_kobj); // kernel_kobj points to /sys/kernel
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

   // irq setup
   irqNumber = gpio_to_irq(gpioInput);
   printk(KERN_INFO "MORSE RECEIVE: The input pin is mapped to IRQ: %d\n", irqNumber);
   result = request_irq(irqNumber,
              (irq_handler_t) gpio_irq_handler,
              IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
              "morse_repeat_gpio_handler",
              NULL);

   printk(KERN_INFO "MORSE RECEIVE: The interrupt request result is: %d\n", result);

   return result;
}

// =============================== EXIT =======================================

// Cleanup function
static void __exit morseSend_exit(void){
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

// interruption handler
static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   bool signal_value; 
   int elapsed_ms;
   int elapsed_periods;
   struct timespec thisInterrupt_ts;

   timespec_now(&thisInterrupt_ts);
   signal_value = gpio_get_value(gpioInput);

   printk(KERN_INFO "MORESE RECEIVE: Interrupt! (input: %d, binSimbol: %s, binDuration: %s)\n", gpio_get_value(gpioInput), binaryMessageSimbol, binaryMessageDuration);


   thisInterrupt_ms = thisInterrupt_ts.tv_sec * 1000 + thisInterrupt_ts.tv_nsec / 1000000;
   printk(KERN_INFO "MORESE RECEIVE: this_ms: %d \n", thisInterrupt_ms);
   
   if (isFirstInterrupt) {
      isFirstInterrupt = false;
      lastInterrupt_ms = thisInterrupt_ms;
   }

   elapsed_ms = thisInterrupt_ms - lastInterrupt_ms;
   printk(KERN_INFO "MORESE RECEIVE: elapsed_ms: %d \n", elapsed_ms);
   elapsed_periods = elapsed_ms/basePeriod;

   binaryMessageSimbol[binaryIndex] = signal_value ? '1' : '0';
   binaryMessageDuration[binaryIndex] = elapsed_periods;
   binaryIndex++;

   trigger conversion
   convertBinaryToAscii();

   return (irq_handler_t) IRQ_HANDLED;
}


// tableau de référence pour identifier les lettres vis-a-vis la symbolisation titah
void titahToLetter() {
   memset(letterBuffer, 0, LETTER_BUFFER_SIZE);

   if      (strcmp(titahBuffer , "titah") == 0) strcpy(letterBuffer, "A");
   else if (strcmp(titahBuffer , "tahtititi") == 0) strcpy(letterBuffer, "B") ;
   else if (strcmp(titahBuffer , "tahtitahti") == 0) strcpy(letterBuffer, "C") ;
   else if (strcmp(titahBuffer , "tahtiti") == 0) strcpy(letterBuffer, "D") ;
   else if (strcmp(titahBuffer , "ti") == 0) strcpy(letterBuffer, "E") ;
   else if (strcmp(titahBuffer , "tititahti") == 0) strcpy(letterBuffer, "F") ;
   else if (strcmp(titahBuffer , "tahtahti") == 0) strcpy(letterBuffer, "G") ;
   else if (strcmp(titahBuffer , "titititi") == 0) strcpy(letterBuffer, "H") ;
   else if (strcmp(titahBuffer , "titi") == 0) strcpy(letterBuffer, "I") ;
   else if (strcmp(titahBuffer , "titahtahtah") == 0) strcpy(letterBuffer, "J") ;
   else if (strcmp(titahBuffer , "tahtitah") == 0) strcpy(letterBuffer, "K") ;
   else if (strcmp(titahBuffer , "titahtiti") == 0) strcpy(letterBuffer, "L") ;
   else if (strcmp(titahBuffer , "tahtah") == 0) strcpy(letterBuffer, "M") ;
   else if (strcmp(titahBuffer , "tahti") == 0) strcpy(letterBuffer, "N") ;
   else if (strcmp(titahBuffer , "tahtahtah") == 0) strcpy(letterBuffer, "O") ;
   else if (strcmp(titahBuffer , "titahtahti") == 0) strcpy(letterBuffer, "P") ;
   else if (strcmp(titahBuffer , "tahtahtitah") == 0) strcpy(letterBuffer, "Q") ;
   else if (strcmp(titahBuffer , "titahti") == 0) strcpy(letterBuffer, "R") ;
   else if (strcmp(titahBuffer , "tititi") == 0) strcpy(letterBuffer, "S") ;
   else if (strcmp(titahBuffer , "tah") == 0) strcpy(letterBuffer, "T") ;
   else if (strcmp(titahBuffer , "tititah") == 0) strcpy(letterBuffer, "U") ;
   else if (strcmp(titahBuffer , "titititah") == 0) strcpy(letterBuffer, "V") ;
   else if (strcmp(titahBuffer , "titahtah") == 0) strcpy(letterBuffer, "W") ;
   else if (strcmp(titahBuffer , "tahtititah") == 0) strcpy(letterBuffer, "X") ;
   else if (strcmp(titahBuffer , "tahtitahtah") == 0) strcpy(letterBuffer, "Y") ;
   else if (strcmp(titahBuffer , "tahtahtiti") == 0) strcpy(letterBuffer, "Z") ;
   else if (strcmp(titahBuffer , "titahtahtahtah") == 0) strcpy(letterBuffer, "1")  ;
   else if (strcmp(titahBuffer , "tititahtahtah") == 0) strcpy(letterBuffer, "2") ;
   else if (strcmp(titahBuffer , "titititahtah") == 0) strcpy(letterBuffer, "3") ;
   else if (strcmp(titahBuffer , "tititititah") == 0) strcpy(letterBuffer, "4") ;
   else if (strcmp(titahBuffer , "tititititi") == 0) strcpy(letterBuffer, "5") ;
   else if (strcmp(titahBuffer , "tahtitititi") == 0) strcpy(letterBuffer, "6") ;
   else if (strcmp(titahBuffer , "tahtahtititi") == 0) strcpy(letterBuffer, "7") ;
   else if (strcmp(titahBuffer , "tahtahtahtiti") == 0) strcpy(letterBuffer, "8") ;
   else if (strcmp(titahBuffer , "tahtahtahtahti") == 0) strcpy(letterBuffer, "9") ;
   else if (strcmp(titahBuffer , "tahtahtahtahtah") == 0) strcpy(letterBuffer, "0") ;
   
   memset(titahBuffer, 0, TITAH_BUFFER_SIZE);
}



/*   L'idée repose sur le fait que pendant la réception du message, il sera de la forme de 2 tableaux : l'état de l'interruption (niveau haut ou bas) , et la durée de cette interruption
     Du coup on definit la fonction conversion qui prend en paramètre le tableau des états (tableau char de 1 et 0) , et le tableau de durées, de même taille que le premier tableau qui indique la durée de chaque niveau d'interruption.
*/



void convertBinaryToAscii(void)
{
   int nextUnknownSimbol;
   int i;
   int destinationIndex, sourceIndex;
   
   nextUnknownSimbol = 0;
   for (i = 0 ; i < strlen(binaryMessageSimbol) ; ++i) {

      // detect ti
      if(binaryMessageSimbol[i] == '1' && 
         binaryMessageDuration[i] == 1) {

         strcat(titahBuffer , "ti");
      }

      // detect tah
      else if(binaryMessageSimbol[i] == '1' && 
              binaryMessageDuration[i] == 3) {

         strcat(titahBuffer , "tah");
      }

      // detect space between letters
      else if (binaryMessageSimbol[i] == '0' && 
               binaryMessageDuration[i] >= 3 &&
               binaryMessageDuration[i] < 7) {

         titahToLetter();
         strcat(asciiMessage , titahBuffer);
         nextUnknownSimbol = i + 1;
      }

      // detect space character
      else if (binaryMessageSimbol[i] == '0' && 
               binaryMessageDuration[i] >= 7) {

         titahToLetter();
         strcat(asciiMessage , titahBuffer);
         nextUnknownSimbol = i + 1;
         strcat(asciiMessage , " ");
         //continue;
      }

      // don't know what to do, skip
      else continue;
   }

   // bring everything from the position nextUnknownSimbol til the end to the beginning and reset the rest
   for (destinationIndex = 0; i < BINARY_MESSAGE_SIZE; ++destinationIndex) {
      sourceIndex = nextUnknownSimbol + destinationIndex;

      // copy to an earlier position
      if (destinationIndex < nextUnknownSimbol && sourceIndex < BINARY_MESSAGE_SIZE) {
         binaryMessageSimbol[destinationIndex]   = binaryMessageSimbol[sourceIndex];
         binaryMessageDuration[destinationIndex] = binaryMessageDuration[sourceIndex];
      } 
      // set remaining to zero
      else {
         binaryMessageSimbol[destinationIndex]   = 0;
         binaryMessageDuration[destinationIndex] = 0;
      }
   }
}


int timespec_to_ms(struct timespec* time_ts) {
    return time_ts->tv_sec * 1000 + time_ts->tv_nsec / 1000000000;
}


void timespec_now(struct timespec* now) {
    // clock_gettime(CLOCK_REALTIME, &now);
    getnstimeofday(now);
}


struct timespec timespec_subtract(struct timespec* time1_ts, struct timespec* time2_ts) {
    struct timespec timeSub_ts;
    struct timespec time_neg_ts;
    struct timespec timeAdd_ts;

    time_neg_ts.tv_sec = -(time2_ts->tv_sec + 1);
    time_neg_ts.tv_nsec = (int) (1000000000 - time2_ts->tv_nsec);

    timeAdd_ts.tv_sec = time1_ts->tv_sec + time_neg_ts.tv_sec;
    timeAdd_ts.tv_nsec = time1_ts->tv_nsec + time_neg_ts.tv_nsec;

    if (timeAdd_ts.tv_nsec > 1000000000) {
        timeAdd_ts.tv_nsec -= 1000000000;
        timeAdd_ts.tv_sec += 1;
    }

    return timeSub_ts;
}