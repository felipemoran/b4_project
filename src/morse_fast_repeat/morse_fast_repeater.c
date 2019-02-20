#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code


static unsigned int gpioOutput = 27;
static unsigned int gpioInput = 4;
static unsigned int irqNumber;
static unsigned int numberPresses = 0;
static bool	    ledOn = 0;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("M2 SETI");
MODULE_DESCRIPTION("GPIO BLIK");
MODULE_VERSION("0.000002");

static irq_handler_t  ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);
 

static int __init ebbgpio_init(void){
	int result = 0;
	printk(KERN_INFO "GPIO_TEST: Initializing the GPIO_TEST LKM\n");

	// check if pgio is valid
	if (!gpio_is_valid(gpioOutput)){
	printk(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
	return -ENODEV;
	}

	// output setup
	ledOn = true;
	gpio_request(gpioOutput, "sysfs");
	gpio_direction_output(gpioOutput, ledOn);
	gpio_export(gpioOutput, false);

	// input setup
	gpio_request(gpioInput, "sysfs");
	gpio_direction_input(gpioInput);
	gpio_set_debounce(gpioInput, 200);
	gpio_export(gpioInput, false);

	// test input read
	printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", gpio_get_value(gpioInput));

	// irq setup
	irqNumber = gpio_to_irq(gpioInput);
	printk(KERN_INFO "GPIO_TEST: The button is mapped to IRQ: %d\n", irqNumber);
	result = request_irq(irqNumber,
		        (irq_handler_t) ebbgpio_irq_handler,
		        IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		        "ebb_gpio_handler",
		        NULL);
	printk(KERN_INFO "GPIO_TEST: The interrupt request result is: %d\n", result);
	return result;
}
 

static void __exit ebbgpio_exit(void){
	// cleanup
	printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", gpio_get_value(gpioInput));
	printk(KERN_INFO "GPIO_TEST: The button was pressed %d times\n", numberPresses);
	gpio_set_value(gpioOutput, 0);
	gpio_unexport(gpioOutput);
	free_irq(irqNumber, NULL);
	gpio_unexport(gpioInput);
	gpio_free(gpioOutput);
	gpio_free(gpioInput);
	printk(KERN_INFO "GPIO_TEST: Goodbye from the LKM!\n");
}

// interruption handler
static irq_handler_t ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
	bool signal_value = gpio_get_value(gpioInput);
	gpio_set_value(gpioOutput, signal_value);
	printk(KERN_INFO "GPIO_TEST: Interrupt! (input: %d, output: %d)\n", gpio_get_value(gpioInput), gpio_get_value(gpioOutput));

	return (irq_handler_t) IRQ_HANDLED;
}
 

module_init(ebbgpio_init);
module_exit(ebbgpio_exit);
