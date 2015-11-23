#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>    
#include<pthread.h>
 
#define SCK "\x1B[34m"
#define TRD "\x1B[33m"
#define ERR "\x1B[31m"
#define RST "\033[0m"

void *connection_handler(void *);

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1){
        puts(ERR"Main socket: could not create"RST);
    }
    puts(SCK"Main socket: created"RST);
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
        //print the error message
        puts(ERR"Main socket: could not bind"RST);
        return 1;
    }
    puts(SCK"Main socket: binded"RST);
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts(SCK"Main socket: waiting"RST);
    c = sizeof(struct sockaddr_in);
     
    //accept connection from an incoming client
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ){
        if (client_sock < 0){
            puts(ERR"Main socket: failed to accept connection"RST);
            return 1;
        }
        puts(SCK"Main socket: connection accepted"RST);
         
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
         
        if( pthread_create( &sniffer_thread , NULL , connection_handler , (void*) new_sock) < 0)
        {
            puts(ERR"Main thread: failed to create new thread"RST);
            return 1;
        }
        puts(TRD"Main thread: new thread created"RST);
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts(TRD"Main thread: Handler assigned"RST);
    }
    if(shutdown(socket_desc, SHUT_RDWR) == 0){
	puts(SCK"Main socket: down"RST);
	if(close(socket_desc) == 0){
	    puts(SCK"Main socket: closed"RST);
	}
	else{
	    puts(ERR"Main socket: failed to close "RST);
	    return 1;
	}
    }
    else{
        puts(ERR"Main socket: failed to shut down"RST);
    	return 1;
    }
    
    return 0;
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int client_sock = *(int*)socket_desc;
    int read_size;
    char client_message[2000];

    bzero(client_message, sizeof client_message);
    //Receive a message from client
    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
	write(client_sock , client_message , strlen(client_message));
	bzero(client_message, sizeof client_message);
    }


    if(read_size == 0){
        puts(SCK"Subsocket: client disconnected"RST);
	if(shutdown(client_sock, SHUT_RDWR) == 0){
		puts(SCK"Subsocket: down"RST);
	    	if(close(client_sock) == 0){
			puts(SCK"Subsocket: closed"RST);
		}
		else{
			puts(ERR"Subsocket: failed to close"RST);
			goto END;
		}
	}
	else{
		 puts(ERR"Subsocket: failed to shut down"RST);
    		goto END;
   	}
    }
    else if(read_size == -1){
        puts(ERR"Subsocket: failed to recive"RST);
    }
         
    //Free the socket pointer
END:free(socket_desc);
     
    return 0;
}
