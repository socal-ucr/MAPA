#!/bin/bash

GPUS=$@
echo -e "ResNet-50 $GPUS \n"
cd caffe
./build/tools/caffe train --solver=caffe-models/resnet-50/solver.prototxt --gpu $GPUS
