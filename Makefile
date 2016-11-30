all: client server

client:
CC = gcc
CFLAGS = -Wall
DEPS = sntp.h client_functions.h
OBJ = client.o sntp.o client_functions.o

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

clean:
	rm -f *.o client server client_functions sntp
