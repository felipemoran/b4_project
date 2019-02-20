#!/bin/bash

sudo rmmod morse_repeat

make

sudo insmod morse_repeat.ko

sudo chmod 666 /sys/kernel/morse/morseSend/*

sudo echo "Felipe" > /sys/kernel/morse/morseSend/asciiMessage
