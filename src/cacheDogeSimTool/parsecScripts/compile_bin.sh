#! /bin/bash

PIN="/root/pin-3.7-97619-g0d0c92f4f-gcc-linux/pin"
PARSEC_DIR="/root/parsec-2.1"
CACHEDOGE_TOOL="obj-intel64/MyPinTool.so"
PATH_TO_TOOL=/root/src/cacheDogeSimTool/$CACHEDOGE_TOOL
HERE=$(pwd)

#Compile Tool
cd ..; ./compile.sh 2>&1 >> /dev/null; cd $HERE
