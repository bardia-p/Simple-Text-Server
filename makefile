all: server client

server: server.o
	gcc	server.o -o server

server.o: server.c
	gcc	server.c -c

client: client.o
	gcc	client.o -o client

client.o: client.c
	gcc	client.c -c

cleanup:
	rm	-f *.o server client
