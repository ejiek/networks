all: server client enet

server: server.c
	gcc -pthread -o server server.c enet.o -I.
client: client.c
	gcc -o client client.c enet.o -I.
enet:
	gcc -c enet.c
