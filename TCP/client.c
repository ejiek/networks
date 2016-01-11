#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr

#define SERV   "\x1B[35m"
#define CLIENT "\x1B[36m"
#define RESET  "\033[0m"
 
int main(int argc , char *argv[])
{
    int sock;
    int err = 0; // 0 - no error
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
 
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
		    if( recv(sock , server_reply , 2000 , 0) < 0){
        	    puts("recv failed");
	            break;
        	}
         
        	printf(SERV "%s" RESET, server_reply);
    	}
	    else break;
    } 
    if(shutdown(sock, SHUT_RDWR) == 0){    
    	if(close(sock) == 0){
		    puts("Socket closed");
	    }
	    else{
		    puts("Failed to close socket");
		    err = 2;
	    }
    }
    else puts("Failed to shutdown soket");
    return err;
}
