#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "$0 [--sim | --real] [system_arch] [jobs_file]"
    exit
fi

Policy="baselineV1 baselineV2 LASTgreedy LASTpreserve"
cmd=""
prefix=""
if [ $1 == "--sim" ]
then
    cmd="./MapaSim"
    prefix="sim"
elif [ $1 == "--real" ]
then
    cmd="./MapaReal"
    prefix="real"
fi

for pol in $Policy; do
    $cmd $pol $2 $3
done
