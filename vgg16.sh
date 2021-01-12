#!/bin/bash

GPUS=$@
echo "VGG-16 $GPUS"
cd caffe
./build/tools/caffe train --solver=caffe-models/vgg-16/solver.prototxt --gpu $GPUS
