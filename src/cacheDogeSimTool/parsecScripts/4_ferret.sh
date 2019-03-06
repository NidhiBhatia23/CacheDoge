#!/bin/bash

EXE="ferret"
SIM=simdev
TIMES=5
TYPE=apps

PARSEC_PKGS="/root/src/PARSEC/parsec-2.1/pkgs"
DIR="$PARSEC_PKGS/$TYPE/$EXE/run"
EXE_PATH="$PARSEC_PKGS/$TYPE/$EXE/inst/amd64-linux.gcc.pre/bin/$EXE"

PIN="/root/pin-3.7-97619-g0d0c92f4f-gcc-linux/pin"
SO="/root/src/cacheDogeSimTool/obj-intel64/MyPinTool.so"
PIN_DEFAULT="$PIN -t $SO -- "
PIN_DOGE_CACHE="$PIN -t $SO -migrates --"

SIMDEV="$DIR/corel lsh $DIR/queries 5 5 4 $DIR/output.txt"

# Generate files
parsecmgmt -a run -p $EXE -i $SIM -n 4 -x pre

if [ -f tmp.txt ] ; then
    rm tmp.txt
fi

for i in $(seq 1 $TIMES);
do
    echo $i $EXE $SIM "NO Migration";
    
    $PIN_DEFAULT $EXE_PATH $SIMDEV 2>&1 | grep Total | tee -a tmp.txt
    
done

echo "";

for i in $(seq 1 $TIMES);
do
    echo $i $EXE $SIM "Migration";
    
    $PIN_DOGE_CACHE $EXE_PATH $SIMDEV 2>&1 | grep Total | tee -a tmp.txt
    
done


echo "\nTotal Acces Delay"
cat tmp.txt | grep "Acces" | head -n $TIMES | awk '{print $5}' | tr '\n' '\t'
echo ""
cat tmp.txt | grep "Acces" | tail -n $TIMES | awk '{print $5}' | tr '\n' '\t'
echo ""

echo "MPKI"
cat tmp.txt | grep "MPKI" | head -n $TIMES | awk '{print $4}' | tr '\n' '\t'
echo ""
cat tmp.txt | grep "MPKI" | tail -n $TIMES | awk '{print $4}' | tr '\n' '\t'
echo ""

# echo "DPKA"
# cat tmp.txt | grep "DPKA" | head -n $TIMES | awk '{print $3}' | tr '\n' '\t'
# echo ""
# cat tmp.txt | grep "DPKA" | tail -n $TIMES | awk '{print $3}' | tr '\n' '\t'
# echo ""

rm tmp.txt

