all: client server

client:
CC = gcc
CFLAGS = -Wall
DEPS = sntp.h
OBJ = client.o sntp.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

client: $(OBJ)
	gcc $(CFLAGS) -o $@ $^

server:
CC = gcc
CFLAGS = -Wall
DEPS = sntp.h
OBJ = server.o sntp.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

server: $(OBJ)
	gcc $(CFLAGS) -o $@ $^
