#include "inport.h"

void send_client(const char * buffer, SOCKET csock){
    if(send(csock, buffer, strlen(buffer), 0) < 0)
    {
        perror("send()");
        exit(errno);
    }
}
int recv_client(char * buffer, SOCKET csock){
    int n;
    if((n = recv(csock, buffer, SIZE - 1, 0)) < 0)
    {
        n = 0;
        return 1;
    }
    buffer[n]='\0';
    return 0;
}
