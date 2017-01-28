#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inport.h"

#define MAX_C 12

SOCKET serveur_init(int port){
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }
    SOCKADDR_IN sin = { 0 };
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    if(bind(sock, (SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
    {
        perror("bind()");
        exit(errno);
    }
    printf("Serveur lié !\n");

    if(listen(sock, MAX_C) == SOCKET_ERROR)
    {
        perror("listen()");
        exit(errno);
    }
    printf("Serveur listen !\n");

    return sock;
}
typedef struct client{
    SOCKET socket;
    char * name;
} Client;
int main()
{
    /*WSADATA WSAData;
    WSAStartup(MAKEWORD(2,0), &WSAData);*/

    SOCKET serverSocket = serveur_init(1337);
    int max = serverSocket;

    fd_set set;

    SOCKET csock;

    Client clients[MAX_C];
    int nbClients = 0;

    char buffer[255];

    while(1){
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        FD_SET(serverSocket, &set);
        int i;
        for(i=0;i<nbClients; i++){
            FD_SET(clients[i].socket, &set);
        }
        if(select(max + 1, &set, NULL, NULL, NULL) == -1){
            perror("select()");
            exit(errno);
        }
        if(FD_ISSET(STDIN_FILENO, &set)){
            break;
        }
        else if(FD_ISSET(serverSocket, &set)){
            printf("New client\n");
            SOCKADDR_IN csin = { 0 };
            int sinsize = sizeof csin;

            csock = accept(serverSocket, (SOCKADDR *)&csin, &sinsize);

            if(csock == INVALID_SOCKET)
            {
                perror("accept()");
                exit(errno);
            }
            FD_SET(csock, &set);

            max = (csock>max) ? csock : max;

            recv_client(buffer, csock);
            printf("Client %s est connecte\n", buffer);
            send_client("Vous etez bien connete\n", csock);
            clients[nbClients].socket = csock;
            clients[nbClients].name = buffer;
            nbClients++;
        }else{
            int i, j;
            for(i=0; i<nbClients; i++){
                if(FD_ISSET(clients[i].socket, &set)){
                    if(recv_client(buffer, clients[i].socket) == 1){
                        close(clients[i].socket);
                        break;
                    }else{
                        printf("%s\n", buffer);
                        for(j=0; j<nbClients && j!=i; j++){
                            send_client(buffer, clients[j].socket);

                        }
                    }
                    break;
                }
            }
        }

    }

   /*
    int sinsize = sizeof csin;

    csock = accept(serverSocket, (SOCKADDR *)&csin, &sinsize);

    if(csock == INVALID_SOCKET)
    {
        perror("accept()");
        exit(errno);
    }
    printf("Serveur accepte socket !\n");*/

    int i;
    for(i=0;i<nbClients;i++){
        close(clients[i].socket);
    }
    close(serverSocket);

    /*WSACleanup();*/
    return 0;
}
