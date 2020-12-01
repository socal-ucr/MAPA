LDFLAGS=-L./peregrine/ -lperegrine
CC=g++-9
CFLAGS=-O3 -std=c++2a -Wall -Wextra -Wpedantic -fPIC -fconcepts -I./peregrine/core

all: peregrine mgap

peregrine:
	make -C ./peregrine

mgap: main.cpp peregrine
	$(CC) main.cpp $(LDFLAGS) $(CFLAGS)