#!/bin/bash

GPUS=$@
echo -e "GoogleNet $GPUS \n"
cd caffe
./build/tools/caffe train --solver=caffe-models/bvlc_googlenet/solver.prototxt --gpu $GPUS
