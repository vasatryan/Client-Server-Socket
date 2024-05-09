CC = gcc

all: client server

client: src/client.c
	$(CC) -o client src/client.c

server: src/server.c
	$(CC) -o server src/server.c

clean:
	rm -f client server
