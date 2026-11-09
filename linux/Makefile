CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O0 -fno-stack-protector \
          -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -fno-pie
LDFLAGS ?= -no-pie

COMMON_OBJECT = network.o

.PHONY: all clean

all: badftp-server badftp-client

badftp-server: server.o $(COMMON_OBJECT)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

badftp-client: client.o $(COMMON_OBJECT)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

server.o: server.c network.h
	$(CC) $(CFLAGS) -c $< -o $@

client.o: client.c network.h
	$(CC) $(CFLAGS) -c $< -o $@

network.o: network.c network.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f badftp-server badftp-client *.o
