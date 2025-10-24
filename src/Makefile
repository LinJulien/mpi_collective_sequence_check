EXE= test

all: $(EXE)

CXX=g++
CC=gcc

MPICC=mpicc

PLUGIN_FLAGS=-I`$(CC) -print-file-name=plugin`/include -g -Wall -fno-rtti -shared -fPIC

CFLAGS=-g -O3

libplugin.so: plugin.cpp
	$(CXX) $(PLUGIN_FLAGS) $(GMP_CFLAGS) -o $@ $<

test : libplugin.so test.c
	$(MPICC) test.c $(CFLAGS) -o $@ -fplugin=./$< 

clean:
	rm -rf $(EXE)

clean_all: clean
	rm -rf libplugin*.so *.dot
