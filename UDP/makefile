all: server client enet

server: server.c enet.o
	gcc -pthread -o server server.c enet.o -I.
client: client.c enet.o
	gcc -o client client.c enet.o -I.
enet:
	gcc -c enet.c
