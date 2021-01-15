#!/bin/bash

Policy="baselineV1 baselineV2 LASTgreedy"

for pol in $Policy; do
    ./mgapSim $pol $@ >> simStdOut.txt 2>&1
done

for pol in $Policy; do
    ./mgapReal $pol $@ >> realStdOut.txt 2>&1
done
