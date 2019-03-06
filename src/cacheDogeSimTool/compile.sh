#!/bin/bash

rm obj-intel64/*

make PIN_ROOT=/root/pin-3.7-97619-g0d0c92f4f-gcc-linux TARGET=intel64 obj-intel64/MyPinTool.so
