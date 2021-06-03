#!/bin/bash

GPUS=$@
echo -e "CaffeNet $GPUS \n"
cd caffe
./build/tools/caffe train --solver=caffe-models/bvlc_reference_caffenet/solver.prototxt --gpu $GPUS
