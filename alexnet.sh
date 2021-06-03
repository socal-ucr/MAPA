#!/bin/bash

GPUS=$@
echo -e "AlexNet $GPUS \n"
cd caffe
./build/tools/caffe train --solver=caffe-models/bvlc_alexnet/solver.prototxt --gpu $GPUS
