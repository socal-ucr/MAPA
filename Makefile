LDFLAGS=-L/usr/local/lib -lpthread -latomic -Lperegrine/tbb2020/lib/intel64/gcc4.8 -Lperegrine/core/bliss-0.73/ -lbliss -ltbb
CC=g++
CFLAGS=-O3 -std=c++2a -Wall -Wextra -Wpedantic -fPIC -fconcepts -Iperegrine/core -Iperegrine/tbb2020/include
OBJ=peregrine/core/DataGraph.o peregrine/core/PO.o peregrine/core/utils.o peregrine/core/PatternGenerator.o peregrine/core/showg.o
BLISS_LDFLAGS=-L$(ROOT_DIR)/core/bliss-0.73/ -lbliss

all: MapaSim MapaReal

MapaSim: MapaSim.cc $(OBJ) bliss
	$(CC) MapaSim.cc $(OBJ) -o $@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)


MapaReal: MapaReal.cc $(OBJ) bliss
	$(CC) MapaReal.cc $(OBJ) -o $@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

bliss:
	make -C peregrine/core/bliss-0.73

clean:
	rm MapaSim MapaReal
