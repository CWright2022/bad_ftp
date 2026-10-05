CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2

COMMON_OBJECT = network.o

.PHONY: all clean

all: badftp-server badftp-client

badftp-server: server.o $(COMMON_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

badftp-client: client.o $(COMMON_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

server.o: server.c network.h
	$(CC) $(CFLAGS) -c $< -o $@

client.o: client.c network.h
	$(CC) $(CFLAGS) -c $< -o $@

network.o: network.c network.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f badftp-server badftp-client *.o
