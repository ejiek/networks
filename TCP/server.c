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
    struct sockaddr_in client;
};
struct client_descriptor client_d[100];
pthread_mutex_t lock;
pthread_mutex_t nock;

struct piece_of_news{
    int id, theme;
    char text[5000];
};
struct piece_of_news pofn[100];
const char *themes[4] = {"Tech ", "Science", "Movies", "Cars"};

void *connection_handler(void *);
void *accept_handler(void *);
int add_descriptor(int);
int add_tdescriptor(int , pthread_t);
int add_sdescriptor(int , struct sockaddr_in);
int shut(int id);
void list();
int readline(int , char *, int);

void list_themes(char *);
int list_news(int, char *);
int add_po_news(char *);
int show_news(char * , char *);
void help(char *);

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
    char reply[2000];

    bzero(client_message, sizeof client_message);
    //Receive a message from client
    //while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
    while( (read_size = readline(client_sock , client_message , 2000)) > 0 ){
        if(strncmp(client_message,"LIST",4) == 0){
            if(client_message[5] == '\0') list_themes(reply);
            else list_news(strtoul(client_message+5, NULL,10) ,reply);
        }
        else if(strncmp(client_message, "ADDN", 4) == 0){ 
            if(add_po_news(client_message) < 0){
                strcpy(reply, "Unable to add this piece of news: no free space\n");
            }
            else strcpy(reply, "This piece of news has been added!\n");
        }
        else if(strncmp(client_message, "SHOW", 4) == 0){
            show_news(client_message, reply);
        }
        else if(strncmp(client_message,"HELP",2) == 0){ help(reply);}
        else strcpy(reply, "WRONG COMMAN\n");
	    write(client_sock , reply , strlen(reply));
	    bzero(client_message, sizeof client_message);
        bzero(reply, sizeof reply);
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
    pthread_mutex_lock(&lock);
    for(int i=0; i < 100; i++){
        if(client_d[i].socket == client_sock){
            client_d[i].socket = 0;
            break;
        }
    }
    pthread_mutex_unlock(&lock);  
    //Free the socket pointer
    //free(socket_desc);
}

void *accept_handler(void *socket_desc){
    //accept connection from an incoming client
    //int client_descriptor[100];
    int client_sock, c , *new_sock, id;
    struct sockaddr_in client;
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
        
            add_sdescriptor(id, client);
            if( pthread_create( &sniffer_thread , NULL , connection_handler , (void*) new_sock) < 0){
                puts(ERR"Main thread: failed to create new thread"RST);
                goto AH_END;
            }
            add_tdescriptor(id, sniffer_thread);

            printf(TRD"Main thread: new thread created, id = %d, port = %d"RST"\n", id, ntohs(client.sin_port));
        }
    }
AH_END:

    for(int i=0; i < 100; i++){
        if(client_d[i].socket != 0){
	        shut(client_d[i].id);
        }
    }

    for(int i=0; i < 100; i++){
        if(client_d[i].socket != 0){
	        pthread_join( client_d[i].thread , NULL);
        }
    }
    //free(socket_desc);
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

int add_sdescriptor(int id, struct sockaddr_in client){
    pthread_mutex_lock(&lock);
    for(int i=0; i < 100; i++){
        if(client_d[i].id == id){
            client_d[i].client = client;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
    if(id == 0){return 1;}
    return 0;
}

int shut(int id){
    int i = id - 1;// correc id to real position
    pthread_mutex_lock(&lock);
    if(client_d[i].socket != 0){
        if(shutdown(client_d[i].socket, SHUT_RDWR) == 0){
        printf(SCK"Subsocket[%d]: down"RST"\n", id);
    	    if(close(client_d[i].socket) == 0){
            printf(SCK"Subsocket[%d]: closed"RST"\n", id);
    	    }
    	    else{
    		    printf(ERR"Subsocket[%d]: failed to close"RST"\n", id);
    		    goto SH_END;
    	    }
        }
        else{
	        printf(ERR"Subsocket[%d]: failed to shut down"RST"\n", id);
   	        goto SH_END;
        }
    }
SH_END:    
    pthread_mutex_unlock(&lock);
    return 0;
}

void list(){
    pthread_mutex_lock(&lock);
    for(int i=0; i < 100; i++){
        if(client_d[i].socket != 0){
            printf(TRD"ID = %d, port = %d"RST"\n", client_d[i].id , ntohs(client_d[i].client.sin_port));
        }
    }
    pthread_mutex_unlock(&lock);
}

int readline(int fd, char *buf, int len){
    char tmp = ' ';
    char *p = &tmp;
    int rc;
    for(int i = 0; i < len; i++){
        rc = recv( fd, p, 1, 0 );
        if( rc == 0 ) return 0;
        buf[i] = *p;
        if( (int)*p == '\n'){
            //buf[i] = '\0';
            return i + 1;
        }
    }
    buf[ len - 1 ] = '\0';
    return -1;
}

void list_themes(char *reply){
    char tmp[50];
    strcpy(reply, "Themes:\n");
    for(int i=0; i < sizeof(themes) / sizeof(themes[0]); i++){
        sprintf(tmp, "[%d] %s\n", i, themes[i]);
        strcat(reply, tmp);
    }
}

int list_news(int theme_id, char *reply){
    int is_any = 0;
    char tmp[20];
    printf("Theme id: %d\n", theme_id);
    strcpy(reply, "News:\n");
    for(int i=0; i < 100; i++){
        if(pofn[i].text[0] != '\0' && pofn[i].theme == theme_id){
            is_any++;
            sprintf(tmp, "[%d] ", i);
            strncpy(tmp+4 , pofn[i].text, sizeof(tmp)-4);
            if(tmp[sizeof tmp - 1] != '\0'){
                if (sizeof(tmp) < strlen(tmp)){
                    tmp[sizeof(tmp) - 1] = '\0';
                    tmp[sizeof(tmp) - 2] = '\n';
                }
                else{
                    tmp[strlen(tmp) - 1] = '\0';
                    tmp[strlen(tmp) - 2] = '\n';
                }
            }
            printf("sprintf and \\n correction: %s", tmp);
            strcat(reply, tmp);
            printf("Reply now: %s", reply);
            bzero(tmp, sizeof tmp);
        }
    }
    if(is_any > 0) return 0;
    return -1;
}

int add_po_news(char *message){
    int theme_id;
    if(message[4] != ' '){ return -2;}
    theme_id = strtoul(message+5, NULL,10);
    pthread_mutex_lock(&nock);
    for (int i = 0; i < 100; i++){
        if(pofn[i].text[0]  == '\0'){
            strcpy(pofn[i].text, message+7);
            pofn[i].theme = theme_id;
            goto ADD_END;
        }
    }
    pthread_mutex_unlock(&nock);
    return -1;
ADD_END:
    pthread_mutex_unlock(&nock);
    return 0;
}

int show_news(char *message, char *reply){
    int news_id;
    news_id = strtoul(message+5, NULL,10);
    if(pofn[news_id].text[0]=='\0') {return -1;}
    pthread_mutex_lock(&nock);
    strcpy(reply, pofn[news_id].text);
    pthread_mutex_unlock(&nock);
    return 0;
}

void help(char *reply){
    strcpy(reply, "Available commands:\nLIST - prints a list of themes\nADDN [theme id] [text] - adds piece of news to the theme\nHELP - prints this help\nSHOW [id] - prints the piece of news\n");
}
