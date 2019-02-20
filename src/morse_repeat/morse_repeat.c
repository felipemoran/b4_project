#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Felipe Morán and Ayoub Bounaga");
MODULE_DESCRIPTION("Linux driver for sending Morse messages");
MODULE_VERSION("1.0");

// =============================== DEFINES ====================================

// =============================== LOCAL VARIABLES ============================

static unsigned int gpioOutput = 27;
static unsigned int gpioInput = 4;
static unsigned int irqNumber;

// =============================== PARAMETERS =================================

module_param(gpioOutput, uint, S_IRUGO);
MODULE_PARM_DESC(gpioOutput, " output GPIO number (default=27)");


module_param(gpioInput, uint, S_IRUGO);
MODULE_PARM_DESC(gpioInput, " input GPIO number (default=4)");

// =============================== PROTOTYPES =================================

static irq_handler_t  gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

// =============================== KOBJECTS SETUP =============================

// =============================== INIT =======================================

static int __init ebbgpio_init(void){
	int result = 0;
	printk(KERN_INFO "MORESE REPEATE: Initializing the MORESE REPEATE\n");

	// check if pgio is valid
	if (!gpio_is_valid(gpioOutput)){
		printk(KERN_INFO "MORESE REPEATE: invalid output GPIO\n");
		return -ENODEV;
	}

	// output setup
	gpio_request(gpioOutput, "sysfs");
	gpio_direction_output(gpioOutput, 0);

	// input setup
	gpio_request(gpioInput, "sysfs");
	gpio_direction_input(gpioInput);

	// irq setup
	irqNumber = gpio_to_irq(gpioInput);
	printk(KERN_INFO "MORESE REPEATE: The input pin is mapped to IRQ: %d\n", irqNumber);
	result = request_irq(irqNumber,
		        (irq_handler_t) gpio_irq_handler,
		        IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		        "morse_repeat_gpio_handler",
		        NULL);

	printk(KERN_INFO "MORESE REPEATE: The interrupt request result is: %d\n", result);
	return result;
}

// =============================== EXIT =======================================

static void __exit ebbgpio_exit(void){
	// cleanup
	gpio_set_value(gpioOutput, 0);
	
	free_irq(irqNumber, NULL);

	gpio_unexport(gpioOutput);
	gpio_unexport(gpioInput);
	
	gpio_free(gpioOutput);
	gpio_free(gpioInput);
	
	printk(KERN_INFO "MORESE REPEATE: Goodbye from the kernel module!\n");
}

module_init(ebbgpio_init);
module_exit(ebbgpio_exit);

// =============================== PRIVATE ====================================

// interruption handler
static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
	bool signal_value = gpio_get_value(gpioInput);
	gpio_set_value(gpioOutput, signal_value);
	printk(KERN_INFO "MORESE REPEATE: Interrupt! (input: %d, output: %d)\n", gpio_get_value(gpioInput), gpio_get_value(gpioOutput));

	return (irq_handler_t) IRQ_HANDLED;
}

