#include "enet.h"

int mn_recv(int sock, char *client_message, int *mn, struct mes_buf mesbuf[100], struct timeval *timeout, fd_set *readset){
    char msg_with_n[BUFLEN], tmp[BUFLEN];
    int read_size, new_mn, buf_read_size, reason;

    while(reason = select(sock+1, readset, NULL, NULL, timeout) > 0){

    if( (read_size = recv(sock , msg_with_n, BUFLEN , 0)) > 0){
        new_mn = get_mn(msg_with_n);
        printf("recieved number: %d\n", new_mn);
        if( (buf_read_size = get_from_buf(*mn+1, mesbuf, tmp)) > 0){
            *mn = *mn + 1;
            strcpy(client_message, tmp);
            return buf_read_size;
        }
        else{
            strcpy(tmp, msg_with_n+4);
            printf("mn: %d\n", *mn);
            if( new_mn == *mn + 1){
                *mn = *mn + 1;
                strcpy(client_message, tmp);
                printf("recieved message: %s\n", client_message);
                return strlen(client_message);
            }
            else{
                if(new_mn > *mn){
                    add_to_buf(new_mn, tmp, &mesbuf[100]);
                }
            }
        }
    }

    }
    
    if( (buf_read_size = get_from_buf(*mn+1, mesbuf, tmp)) > 0){
        *mn = *mn + 1;
        return buf_read_size;
    }
    
    return reason;

}

int get_mn(char msg[BUFLEN]){
    return strtoul(msg, NULL,10);
}

int add_to_buf(int mn, char *msg, struct mes_buf mesbuf[100]){
  for(int i = 0; i < 100; i++){
    if(mesbuf[i].number == mn){
        return -1;
    }
  }
  for(int i = 0; i < 100; i++){
    if(mesbuf[i].number == 0){
        mesbuf[i].number = mn;
        strncpy(mesbuf[i].msg, msg, BUFLEN);
        return 0;
    }
  }
  return -2;
}

int get_from_buf(int nm, struct mes_buf mesbuf[100], char *tmp){
    char n[3];
    nprint(nm, n);
    for(int i = 0; i < 100; i++){
        if(strncmp(mesbuf[i].msg, n, 3) == 0){
            strcpy(tmp, mesbuf[i].msg+4);
            bzero(mesbuf[i].msg, BUFLEN);
            return strlen(tmp);
        }
    }
    return 0;
}

// Receiving part

int nprint(int n, char *message){
  if (n < 0) return -1;
  strncpy(message, "000 ", 4);
  if(n < 10) sprintf(message+2, "%d", n);
  else if(n < 100) sprintf(message+1, "%d", n);
  else if(n < 1000) sprintf(message, "%d", n);
  else return -2;
  return 0;
}

int return_bigger(int recv_size, int buf_size){
    if(recv_size > buf_size) return recv_size;
    else return buf_size;
}

int nsend(int *mn, int sock , char *message){
    char n[BUFLEN];
    strncpy(n, "000", 3);
    nprint(inc_mn(mn), n);
    printf("mn == %d\n", *mn);
    n[3] = ' ';
    strncpy(n+4, message, BUFLEN-4);
    strncpy(message, n, BUFLEN);
    if( send(sock , message, BUFLEN , 0) < 0){
        puts("Send failed");
        return 1;
    }
    return 0;
}

int inc_mn(int *mn){
  if(*mn<1000) *mn = *mn + 1;
  else *mn = 0;
return *mn;
}
