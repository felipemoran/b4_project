#!/bin/bash

sudo rmmod morse_receive

make

sudo insmod morse_receive.ko

sudo chmod 666 /sys/kernel/morse/morseSend/*

sudo echo "Felipe" > /sys/kernel/morse/morseSend/asciiMessage
