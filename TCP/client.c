#include<stdio.h> //printf
#include<string.h>    //strlen
#include<stdlib.h>
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<unistd.h>   

#define SERV   "\x1B[35m"
#define CLIENT "\x1B[36m"
#define RESET  "\033[0m"
 
int main(int argc , char *argv[])
{
    int sock, server_port;
    int err = 0; // 0 - no error
    struct sockaddr_in server;
    char message[1000] , server_reply[2000], server_addr[16], server_tmp_port[6];
    
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
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr( server_addr );
    server.sin_family = AF_INET;
    server.sin_port = htons( server_port );
 
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
    while(1){
	    bzero(server_reply, sizeof server_reply);
	    bzero(message, sizeof message);
	    printf(CLIENT "$ ");
	    fgets (message, sizeof message, stdin);
        printf(RESET);
	    if(strncmp(message,"quit",4) != 0){
		//Send some data
       		if( send(sock , message , strlen(message) , 0) < 0){
                puts("Send failed");
                err = 1;
                break;
        	}
        	//Receive a reply from the server
		    if( recv(sock , server_reply , 2000 , 0) < 1){
        	    puts("recv failed");
	            break;
        	}
         
        	printf(SERV "%s" RESET, server_reply);
    	}
	    else break;
    } 
    if(shutdown(sock, SHUT_RDWR) == 0){    
        puts("Socket is shuted down o.0");
    }
    else puts("Failed to shutdown soket");
    if(close(sock) == 0){
	    puts("Socket closed");
	}
	else{
	    puts("Failed to close socket");
	    err = 2;
	}
    return err;
}
