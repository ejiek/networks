all: server client

server: server.c
	gcc -lpthread -o server server.c -I.
client: client.c
	gcc -o client client.c -I.
