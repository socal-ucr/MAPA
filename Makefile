LDFLAGS=-Lperegrine/tbb2020/lib/intel64/gcc4.8 -Lperegrine/core/bliss-0.73/ -lbliss
CC=g++-9
CFLAGS=-O3 -std=c++2a -Wall -Wextra -Wpedantic -fPIC -fconcepts -Iperegrine/core -Iperegrine/tbb2020/include

all: mgap

mgap: mgap.cpp
	$(CC) mgap.cpp $(LDFLAGS) $(CFLAGS)