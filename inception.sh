#!/bin/bash

GPUS=$@
echo -e "Inception-v3 $GPUS \n"
cd caffe
./build/tools/caffe train --solver=caffe-models/inception-v3/solver.prototxt --gpu $GPUS
