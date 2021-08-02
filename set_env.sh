export PATH=$PWD:$PATH
export CPATH=$PWD/unittest-cpp:$CPATH
export LD_LIBRARY_PATH=$PWD/unittest-cpp/UnitTest++/.libs:$LD_LIBRARY_PATH
export LIBRARY_PATH=$PWD/unittest-cpp/UnitTest++/.libs:$LD_LIBRARY_PATH
cd peregrine
source tbb2020/bin/tbbvars.sh intel64
cd -
