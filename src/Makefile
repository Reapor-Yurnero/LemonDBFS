CC=gcc
CXX=g++
#FUSE_CFLAGS=$(shell pkg-config fuse --cflags --libs)
#CFLAGS=-g $(FUSE_CFLAGS) -D_FILE_OFFSET_BITS=64 -lm
CFLAGS=-Wall -c `pkg-config fuse --cflags --libs`
CXXFLAGS=-std=c++14 -Wall -Wextra -Werror -pedantic -Wno-unused-result -Wno-unused-parameter 
SRC=$(wildcard *.cpp ./query/*.cpp ./query/data/*.cpp ./query/management/*.cpp ./db/*.cpp)
O_FILES=$(patsubst %.cpp,%.o,$(SRC))

all: build
 
build: $(O_FILES) lemondbfs.o
	$(CXX) $(CXXFLAGS) $(O_FILES) lemondbfs.o `pkg-config fuse --cflags --libs` -o lmdbfs_main

lemondbfs.o: lemondbfs.c
	$(CC) $(CFLAGS) lemondbfs.c -o lemondbfs.o

clean:
	find . -type f -name '*.o' -exec rm {} +
