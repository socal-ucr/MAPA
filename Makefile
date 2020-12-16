LDFLAGS=-L/usr/local/lib -lpthread -latomic -Lperegrine/tbb2020/lib/intel64/gcc4.8 -Lperegrine/core/bliss-0.73/ -lbliss -ltbb
CC=g++
CFLAGS=-O0 -g -std=c++2a -Wall -Wextra -Wpedantic -fPIC -fconcepts -Iperegrine/core -Iperegrine/tbb2020/include
OBJ=peregrine/core/DataGraph.o peregrine/core/PO.o peregrine/core/utils.o peregrine/core/PatternGenerator.o peregrine/core/showg.o
BLISS_LDFLAGS=-L$(ROOT_DIR)/core/bliss-0.73/ -lbliss

all: mgap

mgap: Mgap.cc $(OBJ) bliss
	$(CC) Mgap.cc $(OBJ) -o $@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

bliss:
	make -C peregrine/core/bliss-0.73