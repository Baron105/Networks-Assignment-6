all: client.c server.c
	gcc -o client client.c
	gcc -o server server.c

client: client.c
	gcc -o client client.c
	./client

server: server.c
	gcc -o server server.c
	./server

clean:
	rm -f client server