#!/bin/bash

if [ -z "$1" ];
then
	echo "Please Provide binary !"
	echo "Ex: ./exec.sh /bin/ls"
else
	echo "Input: $@"
	../../pin-3.7-97619-g0d0c92f4f-gcc-linux/pin -t obj-intel64/MyPinTool.so -- "$@"
fi
