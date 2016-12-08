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
DEPS = sntp.h server_functions.h
OBJ = server.o sntp.o server_functions.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

server: $(OBJ)
	gcc $(CFLAGS) -o $@ $^
	
clean:
	rm -f *.o client server sntp client_functions server_functions
