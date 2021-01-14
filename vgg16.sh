#!/bin/bash

GPUS=$@
echo -e "VGG-16 $GPUS\n"
cd caffe
./build/tools/caffe train --solver=caffe-models/vgg-16/solver.prototxt --gpu $GPUS
