# b4 Project

The aim of this project is to make a Raspberry Pi function as the sender, repeater or receiver of a Morse encoded message sent and received through GPIO pins.

The project is thus divided in three different kernel modules: `morse_send`, `morse_receive` and `morse_repeat`. Each module is contained in it's own folder in the `src` folder and has a `Makefile` for it's compilation.

## Basic functionality

All the code of this project is intended to be run in the kernel space. The reception of signals is done with interruptions, and the communication with the user is done by exploiting `sysfs` files.

## morse_send

This modules receives an ASCII message, converts it to a binary format according to the Morse encoding convention and sends it by toggling a GPIO pin high and low.

By default the output pin is number 22 (BCM numbering), but a parameter name `gpioOutput` can be passed when loading the module in order to change it.

Also, by default the base period (duration of a ti symbol) is 100 ms, but this value can be changed by passing the parameter `basePeriod` with a value when loading.

```
sudo insmod morse_send gpioOutput=6 basePeriod=50
```

It is also possible to change the base period after the module has been loaded by writing the new value to `/sys/kernel/morseSend/morseSend/basePeriod`. Sudo privilege is needed for this action.

```
sudo echo 50 > /sys/kernel/morseSend/morseSend/basePerios
```

Finally, in order to input the ASCII message to be sent one only needs to write it to `/sys/kernel/morseSend/morseSend/asciiMessage`.

```
sudo echo "this is a message" > /sys/kernel/morseSend/morseSend/asciiMessage
```

## morse_repeat

This modules receives an a signal through it's input GPIO and repeats it in the output GPIO without making any kind of conversion whatsoever.

By default the input pin is number 4 (BCM numbering) and the output pin is 27, but a parameter named `gpioInput` or `gpioOutput` can be passed when loading the module in order to change it.

Since it just repeats the incoming signal to the output pin, without making sense of it, there's no need to define a `basePeriod` parameter and there are no files for communication with the user space.

```
sudo insmod morse_repeat gpioInput=6 gpioOutput=9
```

## morse_receive

This module receives a signal through a GPIO pin, decodes it and stores the message in a buffer until the user reads it.

Like the previous modules, the input pin and the base period can be changed in the same fashion.

It's output can be read from `/sys/kernel/morseReceive/morseReceive` 

```
cat /sys/kernel/morseReceive/morseReceive
```

This module however is not working properly and might cause a kernel panic when loaded.
