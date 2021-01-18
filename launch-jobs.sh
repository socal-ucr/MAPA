#!/bin/bash

Policy="baselineV1 baselineV2 LASTgreedy"
cmd=""
prefix=""
if [ $1 == "--sim" ]
then
    cmd="./mgapSim"
    prefix="sim"
elif [ $1 == "--real" ]
then
    cmd="./mgapReal"
    prefix="real"
fi

for pol in $Policy; do
    $cmd $pol $2 $3 >> "${prefix}_$2_StdOut.txt" 2>&1
done
