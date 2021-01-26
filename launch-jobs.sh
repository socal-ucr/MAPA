#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "$0 [--sim | --real] [system_arch] [jobs_file]"
    exit
fi

Policy="baselineV1 baselineV2 LASTgreedy LASTpreserve LASTpreserveRoute LASTgreedyRoute"
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
    $cmd $pol $2 $3 >> "${prefix}_$3_StdOut.txt" 2>&1
done
