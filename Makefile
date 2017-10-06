CFLAGS = -O3 -Wall -static -Wno-unknown-pragmas
LDLIBS = -lorcon -lm -lrt
CC = gcc

ALL = random_packets rcv_and_ack

all: $(ALL)

clean:
	rm -f *.o $(ALL)

random_packets: random_packets.c util.o
rcv_and_ack: rcv_and_ack.c util.o

util.c: util.h
