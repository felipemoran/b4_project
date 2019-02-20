#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>       // Required for the GPIO functions
#include <linux/kobject.h>    // Using kobjects for the sysfs bindings
#include <linux/kthread.h>    // Using kthreads for the flashing functionality
#include <linux/delay.h>      // Using this header for the msleep() function
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Derek Molloy");
MODULE_DESCRIPTION("A simple Linux LED driver LKM for the BBB");
MODULE_VERSION("0.1");
 
static unsigned int gpioLED = 17;           ///< Default GPIO for the LED is 49
static unsigned int blinkPeriod = 1000;     ///< The blink period in ms
 
static bool ledOn = 0;                      ///< Is the LED on or off? Used for flashing
enum modes { OFF, ON, FLASH };              ///< The available LED modes -- static not useful here
static enum modes mode = FLASH;             ///< Default mode is flashing

static struct task_struct *task;            /// The pointer to the thread task

/** @brief The LED Flasher main kthread loop
 *
 *  @param arg A void pointer used in order to pass data to the thread
 *  @return returns 0 if successful
 */
static int flash(void *arg){
   printk(KERN_INFO "EBB LED: Thread has started running \n");
   while(!kthread_should_stop()){           // Returns true when kthread_stop() is called
      set_current_state(TASK_RUNNING);
      if (mode==FLASH) ledOn = !ledOn;      // Invert the LED state
      else if (mode==ON) ledOn = true;
      else ledOn = false;
      gpio_set_value(gpioLED, ledOn);       // Use the LED state to light/turn off the LED
      set_current_state(TASK_INTERRUPTIBLE);
      msleep(blinkPeriod/2);                // millisecond sleep for half of the period
   }
   printk(KERN_INFO "EBB LED: Thread has run to completion \n");
   return 0;
}
 
/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point. In this example this
 *  function sets up the GPIOs and the IRQ
 *  @return returns 0 if successful
 */
static int __init ebbLED_init(void){
   int result = 0;

   ledOn = true;
   gpio_request(gpioLED, "sysfs");          // gpioLED is 49 by default, request it
   gpio_direction_output(gpioLED, ledOn);   // Set the gpio to be in output mode and turn on
   gpio_export(gpioLED, false);  // causes gpio49 to appear in /sys/class/gpio
                                 // the second argument prevents the direction from being changed
 
   task = kthread_run(flash, NULL, "LED_flash_thread");  // Start the LED flashing thread
   if(IS_ERR(task)){                                     // Kthread name is LED_flash_thread
      printk(KERN_ALERT "EBB LED: failed to create the task\n");
      return PTR_ERR(task);
   }
   return result;
}
 
/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit ebbLED_exit(void){
   kthread_stop(task);                      // Stop the LED flashing thread
   gpio_set_value(gpioLED, 0);              // Turn the LED off, indicates device was unloaded
   gpio_unexport(gpioLED);                  // Unexport the Button GPIO
   gpio_free(gpioLED);                      // Free the LED GPIO
   printk(KERN_INFO "EBB LED: Goodbye from the EBB LED LKM!\n");
}
 
/// This next calls are  mandatory -- they identify the initialization function
/// and the cleanup function (as above).
module_init(ebbLED_init);
module_exit(ebbLED_exit);
