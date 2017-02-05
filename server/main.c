#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inport.h"

#define MAX_C 12
#define PASSWORD_ADMIN "May The Force Be With You"
typedef struct client{
    SOCKET socket;
    char name[20];
    int isAdmin;
    int nbMessages;
} Client;


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

    int yes = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,&yes, sizeof(int)) == -1)
    {
        perror("setsocketopt()");
        exit(errno);
    }

    if(bind(sock, (SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
    {
        perror("bind()");
        exit(errno);
    }
    printf("Serveur li� !\n");

    if(listen(sock, MAX_C) == SOCKET_ERROR)
    {
        perror("listen()");
        exit(errno);
    }
    printf("Serveur listen !\n");

    return sock;
}
void remove_client(Client * tab, int index, int nbClients){
    int i;
    for(i=index; i< nbClients; i++){
        tab[i] = tab[i+1];
    }
}
SOCKET getSocket(char * name, Client * clients, int nbClients){
    int i;
    for(i=0; i<nbClients;i++){
        if(strcmp(name, clients[i].name) == 0) return clients[i].socket;
    }
    return 0;
}
void cmdManager(char buffer[], Client * clients, int nbClients, int id_client){
    if(buffer[0] == '-'){
        switch(buffer[1]){
            case 'w':
                ;
                int i = 3, j=0;
                char name[25];
                while(buffer[i] != ' '){
                    name[j]=buffer[i];
                    j++;
                    i++;
                }
                i++;
                char msg[200];
                j=0;
                while(buffer[i] != '\0'){
                    msg[j]=buffer[i];
                    i++;
                    j++;
                }
                strcpy(buffer,clients[id_client].name);
                strcat(buffer, "(whisper): ");
                strcat(buffer, msg);
                SOCKET sock = getSocket(name, clients, nbClients);
                for(j=0; j<nbClients; j++){
                    if(sock == clients[j].socket){
                        send_client(buffer, clients[j].socket);
                    }
                }
                clients[id_client].nbMessages++;
                break;
            case 'n':
                strcpy(buffer,"Les utilisateurs connectés sont : ");
                int k=0;
                for(k=0;k<nbClients;k++){
                    strcat(buffer,clients[k].name);
                    strcat(buffer,"\n");
                }
                send_client(buffer, clients[id_client].socket);
                break;
            case 'i':
                if(clients[id_client].isAdmin==1){
                    int l = 3, m=0;
                    char user[25];
                    while(buffer[l] != ' '){
                        user[m]=buffer[l];
                        m++;
                        l++;
                    }
                    for(m=0;m<nbClients;m++){
                        if(strcmp(clients[m].name,user)==0){
                            strcpy(buffer,"Les statistiques de ");
                            strcat(buffer,user);
                            strcat(buffer," sont : \n Nombre de messages : ");
                            strcat(buffer, clients[m].nbMessages);
                            send_client(buffer,clients[id_client].socket);
                        }
                    }
                }else{
                    strcpy(buffer,"Vous n'êtes pas autorisé à utiliser cette commande \n");
                    send_client(buffer,clients[id_client].socket);
                }
                break;
            case 'c':
                strcpy(buffer,"Entrez le mot de passe administrateur : \n");
                send_client(buffer,clients[id_client].socket);
                recv_client(buffer,clients[id_client].socket);
                if(strcmp(buffer,PASSWORD_ADMIN)==0){
                    clients[id_client].isAdmin=1;
                    strcpy(buffer,"\nVous êtes connecté en tant qu'administrateur.");
                }else{
                    strcpy(buffer,"\nMot de passe incorrect");
                }
                send_client(buffer,clients[id_client].socket);
                break;
            default :
                strcpy(buffer, "Error: cmd unknown");
                send_client(buffer, clients[id_client].socket);
                break;
        }
    }else{
        int j;
        char tmp[SIZE];
        strcpy(tmp, "");
        printf("???");
        for(j=0; j<nbClients; j++){
            if(clients[id_client].socket != clients[j].socket){
                strcat(tmp, clients[id_client].name);
                strcat(tmp, ": ");
                strcat(tmp, buffer);
                send_client(tmp, clients[j].socket);
            }
        }
        clients[id_client].nbMessages++;
    }
}
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

    char buffer[SIZE];

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
            printf("Client %s est connecté\n", buffer);
            send_client("Vous etez bien connecté\n", csock);
            clients[nbClients].socket = csock;
            clients[nbClients].isAdmin = 0;
            clients[nbClients].nbMessages = 0;
            strcpy(clients[nbClients].name, buffer);
            nbClients++;
            
            strcpy(buffer, clients[nbClients - 1].name);
            strcat(buffer, " vient de se connecter");
            
            int j;
            for(j=0; j<nbClients; j++){
                if(clients[nbClients - 1].socket != clients[j].socket){                    
                    send_client(buffer, clients[j].socket);
                }
            }
        }else{
            int i, j;
            for(i=0; i<nbClients; i++){
                if(FD_ISSET(clients[i].socket, &set)){
                    if(recv_client(buffer, clients[i].socket) == 1){
                        printf("%s c'est deco", clients[i].name);
                        close(clients[i].socket);
                        nbClients--;
                        remove_client(clients, i, nbClients);                        
                        break;
                    }else{
                        cmdManager(buffer, clients, nbClients, i);
                        fflush(stdout);
                    }
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
