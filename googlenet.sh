#!/bin/bash

GPUS=$@
echo "GoogleNet $GPUS"
cd caffe
./build/tools/caffe train --solver=caffe-models/bvlc_googlenet/solver.prototxt --gpu $GPUS
