#include<stdio.h> //printf
#include<string.h>    //strlen
#include<stdlib.h>
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<unistd.h>
#include "enet.h"

#define SERV   "\x1B[35m"
#define CLIENT "\x1B[36m"
#define RESET  "\033[0m"

int main(int argc , char *argv[])
{
    int sock, server_port, reason, lmn = 0, rmn = 0, time_to_wait = 3;
    struct sockaddr_in server;
    int slen = sizeof(server);
    char message[BUFLEN] , server_reply[BUFLEN], server_addr[16], server_tmp_port[6];
    struct timeval timeout;
    fd_set readset;
    struct mes_buf mesbuf[100];

    timeout.tv_sec = time_to_wait;

    puts("Insert server addres (default 127.0.0.1)");
    fgets(server_addr, sizeof server_addr, stdin);
    puts("Insert server port (default 8888)");
    fgets(server_tmp_port, sizeof server_tmp_port, stdin);
    server_port = strtoul(server_tmp_port, NULL,10);

    if(server_addr[0] == '\n'){
        strcpy(server_addr, "127.0.0.1");
    }

    if(server_addr[strlen(server_addr)-1] == '\n') server_addr[strlen(server_addr)-1] = '\0';

    if(server_port == 0){
        server_port = 8888;
    }

    //Create socket
    sock = socket(AF_INET , SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    FD_ZERO(&readset);
    FD_SET(sock, &readset);

    server.sin_addr.s_addr = inet_addr( server_addr );
    server.sin_family = AF_INET;
    server.sin_port = htons( server_port );

    if(sendto(sock, "HEL\n\0", BUFLEN, 0, &server, slen)==-1)
        puts("failed to send Hello");
    if(recvfrom(sock, server_reply, BUFLEN, 0, &server, &slen)==-1)
        puts("failed to recieve a Hello answer");
    printf("Received packet from %s:%d\n",
        inet_ntoa(server.sin_addr), ntohs(server.sin_port));

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected");

    printf(SERV "Colour of Server messages \n" RESET);
    printf(CLIENT "Colour of Your messages \n\n" RESET);
    //keep communicating with server
    while(1)
    {
	bzero(server_reply, sizeof server_reply);
	bzero(message, sizeof message);
	printf(CLIENT "$ ");
	fgets (message, sizeof message, stdin);
        printf(RESET);
	if(strncmp(message,"quit",4) != 0){
		//Send some data
       	    nsend(&lmn, sock , message);
        	//Receive a reply from the server
    timeout.tv_sec = time_to_wait;
    if(reason = mn_recv(sock ,server_reply, &rmn, &mesbuf, &timeout, &readset) > 0){
        printf(SERV "%s" RESET, server_reply);
    }
	else break;
    }
    }
    if( send(sock , "BEY\n\0" , BUFLEN , 0) < 0){
        puts("Send \'BEY\' failed");
    }
    if(shutdown(sock, SHUT_RDWR) == 0){
    	if(close(sock) == 0){
		puts("Socket closed");
	}
	else{
		puts("Failed to close socket");
		return 1;
	}
    }
    else puts("Failed to shutdown soket");
    return 0;
}


