#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>    
#include<pthread.h>
 
void *connection_handler(void *);

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
     
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
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ){
        if (client_sock < 0){
            puts("Fail: accept");
            return 1;
        }
        puts("Connection accepted");
         
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
         
        if( pthread_create( &sniffer_thread , NULL , connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
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
        puts("Client disconnected");
	if(shutdown(client_sock, SHUT_RDWR) == 0){
		puts("Socket is down");
	    	if(close(client_sock) == 0){
			puts("Socket is closed");
		}
		else{
			puts("Fail: cannot close socket");
			goto END;
		}
	}
	else{
		 puts("Fail: cannot shut socket down");
    		goto END;
   	}
    }
    else if(read_size == -1){
        puts("recv failed");
    }
         
    //Free the socket pointer
END:free(socket_desc);
     
    return 0;
}
