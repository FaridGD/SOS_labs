CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGETS = ser client

all: $(TARGETS)

server: ser.c
	$(CC) $(CFLAGS) ser.c -o ser

client: client.c
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f $(TARGETS)

