#!/bin/bash

Policy="baselineV1 baselineV2 LASTgreedy LASTbw"

for pol in $Policy; do
    ./mgap $pol $@
done
