CC = gcc
CFLAGS = -Wall
DEPS = sntp.h
OBJ = client.o sntp.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

client: $(OBJ)
	gcc $(CFLAGS) -o $@ $^
