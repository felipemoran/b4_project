#!/bin/bash

sudo rmmod morse_send

sudo insmod morse_send.ko

sudo chmod 666 /sys/kernel/morse/morseSend/*

sudo echo "Felipe" > /sys/kernel/morse/morseSend/asciiMessage
