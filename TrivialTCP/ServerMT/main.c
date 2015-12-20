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

struct client_descriptor
{
	int	id, socket;
	pthread_t thread;
};
struct client_descriptor client_d[100];
struct sockaddr_in client;
pthread_mutex_t lock;

void *connection_handler(void *);
void *accept_handler(void *);
int add_descriptor(int);
int add_tdescriptor(int , pthread_t);
int shut(int id);
void list();

int main(int argc , char *argv[])
{
    int socket_desc , status_addr;
    struct sockaddr_in server;
    char message[1000];
    
    //initialize strucutre
    pthread_mutex_lock(&lock);    
    for (int i = 0; i < 100; i++){
        client_d[i].socket = 0;
        client_d[i].id = i + 1;
    }
    pthread_mutex_unlock(&lock);

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

    pthread_t accept_thread;

    if( pthread_create( &accept_thread , NULL , accept_handler , (void*) socket_desc) < 0)
        {
            puts(ERR"Main thread: failed to create new thread"RST);
            return 1;
        }
    
    while(1){
        bzero(message, sizeof message);
        fgets (message, sizeof message, stdin);
	    if(strncmp(message,"quit",4) == 0){ break;}
        if(strncmp(message,"shut ",5) == 0){ 
            printf("ID to shut down: %d\n", strtoul(message+5, NULL,10));
            shut(strtoul(message+5, NULL,10));
        }
        if(strncmp(message,"list",4) == 0){
            list();
        }
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

    if (pthread_join(accept_thread , (void**)&status_addr) != 0){
        puts("Main thread: can't join accept thread");
    }
    
    puts("Main thread termination phase");
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
        puts(TRD"Subthread: client disconnected"RST);
	    if(shutdown(client_sock, SHUT_RDWR) == 0){
		    puts(SCK"Subsocket: down"RST);
	    	if(close(client_sock) == 0){
			    puts(SCK"Subsocket: closed"RST);
		    }
		    else{
			    puts(ERR"Subsocket: failed to close"RST);
			    goto CH_END;
		    }
	    }
	    else{
		    puts(ERR"Subsocket: failed to shut down"RST);
    		goto CH_END;
   	    }
    }
    else if(read_size == -1){
        puts(ERR"Subsocket: failed to recive"RST);
    }
         

CH_END:
    puts("connection_handler termination phase");
    pthread_mutex_lock(&lock);
    for(int i=0; i < 100; i++){
        if(client_d[i].socket == client_sock){
            client_d[i].socket = 0;
            break;
        }
    }
    pthread_mutex_unlock(&lock);  
    //Free the socket pointer
    free(socket_desc);
}

void *accept_handler(void *socket_desc){
    //accept connection from an incoming client
    //int client_descriptor[100];
    int client_sock, c , *new_sock, id;
    //struct sockaddr_in client;
    c = sizeof(struct sockaddr_in);

    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ){
        if (client_sock < 0){
            puts(ERR"Main socket: failed to accept connection"RST);
            goto AH_END;
        }
        
        puts(SCK"Main socket: connection accepted"RST);
        
        id = add_descriptor(client_sock);
        if (id == 0){
            puts(ERR"Main socket: all subsockets are busy"RST);
        }
        else{
            pthread_t sniffer_thread;
            new_sock = malloc(1);
            *new_sock = client_sock;
        
            add_tdescriptor(id, sniffer_thread);

            if( pthread_create( &sniffer_thread , NULL , connection_handler , (void*) new_sock) < 0){
                puts(ERR"Main thread: failed to create new thread"RST);
                goto AH_END;
            }
            printf(TRD"Main thread: new thread created, id = %d, port = %d"RST"\n", id, ntohs(client.sin_port));
        }
    }
AH_END:
    //Now join the thread , so that we dont terminate before the thread
    //pthread_join( sniffer_thread , NULL);
    puts("accept_handler termination phase");
}

int add_descriptor(int socket){
    int id = 0; // 0 -  all sockets are busy
    pthread_mutex_lock(&lock);
    for(int i=0; i < 100; i++){
        if(client_d[i].socket == 0){
            client_d[i].socket = socket;
            id = client_d[i].id;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
    return id;
}

int add_tdescriptor(int id, pthread_t sniffer_thread){
    pthread_mutex_lock(&lock);
    for(int i=0; i < 100; i++){
        if(client_d[i].id == id){
            client_d[i].thread = sniffer_thread;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
    if(id == 0){return 1;}
    return 0;
}

int shut(int id){
    pthread_mutex_lock(&lock);
    for(int i=0; i < 100; i++){
        if(client_d[i].id == id){
	        if(shutdown(client_d[i].socket, SHUT_RDWR) == 0){
		        puts(SCK"Subsocket: down"RST);
	    	    if(close(client_d[i].socket) == 0){
			        puts(SCK"Subsocket: closed"RST);
		        }
		        else{
			        puts(ERR"Subsocket: failed to close"RST);
			        goto SH_END;
		        }
	        }
	        else{
		        puts(ERR"Subsocket: failed to shut down"RST);
    		    goto SH_END;
   	        }
        }
    }
SH_END:    
    pthread_mutex_unlock(&lock);
    puts("shut termination phase");
    return 0;
}

void list(){
    pthread_mutex_lock(&lock);
    for(int i=0; i < 100; i++){
        if(client_d[i].socket != 0){
            printf(TRD"ID = %d, port = %d"RST"\n", client_d[i].id , ntohs(client.sin_port));
        }
    }
    pthread_mutex_unlock(&lock);
}
