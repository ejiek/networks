#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>    
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1){
        puts("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
        //print the error message
        puts("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0){
        puts("Fail: accept");
        return 1;
    }
    puts("Connection accepted");
     
    bzero(client_message, sizeof client_message);
    //Receive a message from client
    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 ){
        //Send the message back to client
	write(client_sock , client_message , strlen(client_message));
	bzero(client_message, sizeof client_message);
    }
     
    if(read_size == 0){
        puts("Client disconnected");
	if(shutdown(socket_desc, SHUT_RDWR) == 0){
		puts("Socket is down");
	    	if(close(socket_desc) == 0){
			puts("Socket is closed");
		}
		else{
			puts("Fail: cannot close socket");
			return 1;
		}
	}
	else{
		 puts("Fail: cannot shut socket down");
    		return 1;
   	}
    }
    else if(read_size == -1){
        puts("recv failed");
	return 1;
    }
     
    return 0;
}
