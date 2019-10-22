.PHONY: all clean run
CC=g++
CFLAGS=-std=c++11 -O2
LDFLAGS=-lm -lstdc++
TARGET=train test

all: $(TARGET)

train: src/train.cpp
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -Iinc

test: src/test.cpp
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -Iinc

clean:
	rm -f $(TARGET)
