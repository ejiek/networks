#ifndef ENET_H_
#define ENET_H_

#include<stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>

#define BUFLEN 512

struct mes_buf{
    int number;
    char msg[BUFLEN];
};

int mn_recv(int , char *, int *, struct mes_buf [100], struct timeval *, fd_set *);
int get_mn(char msg[BUFLEN]);
int add_to_buf(int, char *, struct mes_buf mesbuf[100]);
int nprint(int, char *);
int get_from_buf(int , struct mes_buf mesbuf[100], char *);
int return_bigger(int, int);

#endif // ENET_H_
