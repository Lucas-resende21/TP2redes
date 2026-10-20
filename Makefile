CC = gcc
CFLAGS = -std=gnu11 -g

TARGETS = servidor cliente

all: $(TARGETS)

servidor: servidor.c
	$(CC) $(CFLAGS) -o server servidor.c

cliente: cliente.c
	$(CC) $(CFLAGS) -o client cliente.c

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean