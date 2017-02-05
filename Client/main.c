#include <stdio.h>
#include <stdlib.h>
#include "../server/inport.h"
SOCKET client_init(char * server, int port){
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }

    struct hostent *hostinfo = NULL;
    SOCKADDR_IN sin = { 0 }; /* initialise la structure avec des 0 */
    const char *hostname = server;

    hostinfo = gethostbyname(hostname); /* on r�cup�re les informations de l'h�te auquel on veut se connecter */
    if (hostinfo == NULL) /* l'h�te n'existe pas */
    {
        fprintf (stderr, "Unknown host %s.\n", hostname);
        exit(EXIT_FAILURE);
    }

    sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr; /* l'adresse se trouve dans le champ h_addr de la structure hostinfo */
    sin.sin_port = htons(port); /* on utilise htons pour le port */
    sin.sin_family = AF_INET;

    if(connect(sock,(SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        perror("connect()");
        exit(errno);
    }
    return sock;
}

int main()
{
   /* WSADATA WSAData;
    WSAStartup(MAKEWORD(2,0), &WSAData);*/

    SOCKET clientSocket = client_init("localhost", 1337);

    fd_set set;

    char buffer[SIZE];
    char name[20];

    do{
        printf("Entrez un pseudo :\n");
        scanf("%s", name);
    }while(name[0]=='@');

    send_client(name, clientSocket);
    recv_client(buffer, clientSocket);    

    printf("%s\n", buffer);
    fflush(stdout);
    if(strcmp(buffer, "Pseudo non valide") == 0){
        close(clientSocket);
        return 0;
    }

    while(1){
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        FD_SET(clientSocket, &set);
        if(select(clientSocket + 1, &set, NULL, NULL, NULL) == -1){
            perror("select()");
            exit(errno);
        }
        if(FD_ISSET(STDIN_FILENO, &set)){
            fgets(buffer, SIZE - 1, stdin);
            {
                char *p = NULL;
                p = strstr(buffer, "\n");
                if(p!=NULL){
                    *p = 0;
                }else{
                    buffer[SIZE - 1] = 0;
                }
            }
            send_client(buffer, clientSocket);
            if(strcmp(buffer, "@exit") == 0) break;
        }else if(FD_ISSET(clientSocket, &set)){
            if(recv_client(buffer, clientSocket) == 1){
                printf("Connection lost");
                close(clientSocket);
            }
            printf("%s\n", buffer);
            fflush(stdout);
        }
    }

    close(clientSocket);

    /*WSACleanup();*/
    return 0;
}
