#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "inport.h"

#define MAX_C 12
#define PASSWORD_ADMIN "May The Force Be With You"
typedef struct client{
    SOCKET socket;
    char name[20];
    int isAdmin;
    int nbMessages;
    int error;
    time_t time_connect;
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
    printf("Serveur lie !\n");

    if(listen(sock, MAX_C) == SOCKET_ERROR)
    {
        perror("listen()");
        exit(errno);
    }
    printf("Serveur listen !\n");

    return sock;
}
void remove_client(Client * tab, int index, int * nbClients, int * max){
    int i;
    for(i=index; i< *nbClients; i++){
        tab[i] = tab[i+1];
    }
    
    *nbClients = *nbClients - 1;
    printf("Nombre de client restant %d \n",*(nbClients));
    *max = 0;
    for(i=0; i< *nbClients; i++){
        if(*max<tab[i].socket){
            *max = tab[i].socket;
        }
    }
}
SOCKET getSocket(char * name, Client * clients, int * nbClients){
    int i;
    for(i=0; i< *nbClients;i++){
        if(strcmp(name, clients[i].name) == 0) return clients[i].socket;
    }
    return -1;
}
void cmdManager(char buffer[], Client * clients, int * nbClients, int id_client, int * max){
    if(strlen(buffer) > SIZE){
        send_client("Message trop volumineux", clients[id_client].socket);
    }
    if(buffer[0] == '@'){
        int cptcmd = 1;
        char cmd[10];
        while(buffer[cptcmd] != ' '){
            cmd[cptcmd - 1] = buffer[cptcmd];
            cptcmd++;
        }
        cmd[cptcmd-1] = '\0';
        if(strcmp(cmd, "exit") == 0){
            printf("%s c'est deco\n", clients[id_client].name);
            fflush(stdout);
            close(clients[id_client].socket);            
            remove_client(clients, id_client, nbClients, max);
        }else if(strcmp(cmd, "whisper") == 0){
            int i = cptcmd + 1, j=0;
            char name[25];
            while(buffer[i] != ' '){
                name[j]=buffer[i];
                j++;
                i++;
            }
            name[j] = '\0';
            i++;
            char msg[200];
            j=0;
            while(buffer[i] != '\0'){
                msg[j]=buffer[i];
                i++;
                j++;
            }
            msg[j] = '\0';
            strcpy(buffer,clients[id_client].name);
            strcat(buffer, "(whisper): ");
            strcat(buffer, msg);
            SOCKET sock = getSocket(name, clients, nbClients);
            if(sock != -1){
                for(j=0; j<*nbClients; j++){
                    if(sock == clients[j].socket){
                        send_client(buffer, clients[j].socket);
                    }
                }
            }else{
                send_client("Client inexistant", clients[id_client].socket);
                clients[id_client].error++;
            }
            clients[id_client].nbMessages++;
        }else if(strcmp(cmd, "list") == 0){
            strcpy(buffer,"Les utilisateurs connectés sont :\n");
            int k=0;
            for(k=0;k<*nbClients;k++){
                strcat(buffer,clients[k].name);
                strcat(buffer,"\n");
            }
            send_client(buffer, clients[id_client].socket);
        }else if(strcmp(cmd, "stats") == 0){
            if(clients[id_client].isAdmin==1){
                int m;
                for(m=0;m<*nbClients;m++){
                    buffer = strcpy(buffer,"\nLes statistiques de ");
                    strcat(buffer,clients[m].name);
                    strcat(buffer," sont : \n --Nombre de messages : ");
                    char buff[6];
                    sprintf(buff, "%d", clients[m].nbMessages);
                    strcat(buffer, buff);
                    sprintf(buff,"%f", difftime(time(NULL), clients[m].time_connect));
                    strcat(buffer,"\n --Temps de connexion (en sec) : ");
                    strcat(buffer,buff);
                    sprintf(buff,"%d", clients[m].error);
                    strcat(buffer,"\n --Nombre d'erreurs : ");
                    strcat(buffer,buff);
                    send_client(buffer,clients[id_client].socket);
                }
            }else{
                strcpy(buffer,"Vous n'êtes pas autorisé à utiliser cette commande \n");
                send_client(buffer,clients[id_client].socket);
                clients[id_client].error++;
            }
        }else if(strcmp(cmd, "kick") == 0){
            if(clients[id_client].isAdmin==1){
                int i2 = cptcmd + 1, j2=0;
                char name[25];
                while(buffer[i2] != ' '){
                    name[j2]=buffer[i2];
                    j2++;
                    i2++;
                }
                name[j2] = '\0';
                SOCKET sockKick = getSocket(name, clients, nbClients);
                if(sockKick!= -1){
                    int id_kick=-1;
                    for(j2=0;j2<*nbClients;j2++){
                        if(strcmp(clients[j2].name,name)==0){
                            id_kick=j2;
                        }
                    }
                    strcpy(buffer, "Vous avez ete kick");
                    send_client(buffer, sockKick);
                    close(sockKick);
                    remove_client(clients, id_kick, nbClients, max);
                    printf("Utilisateur %s kicked", name);
                }else{
                    strcpy(buffer, "L'utilisateur spécifié n'existe pas");
                    send_client(buffer, clients[id_client].socket);
                    clients[id_client].error++;
                }
                fflush(stdout);
            }else{
                strcpy(buffer,"Vous n'êtes pas autorisé à utiliser cette commande \n");
                send_client(buffer,clients[id_client].socket);
                clients[id_client].error++;
            }
        }else if(strcmp(cmd, "auth") == 0){
            strcpy(buffer,"Entrez le mot de passe administrateur : \n");
            send_client(buffer,clients[id_client].socket);
            recv_client(buffer,clients[id_client].socket);
            if(strcmp(buffer,PASSWORD_ADMIN)==0){
                clients[id_client].isAdmin=1;
                strcpy(buffer,"\nVous êtes connecté en tant qu'administrateur.\n Nouvelles commandes disponibles :\n @stats Affiche les stats de tous le monde\n @kick suivi du pseudo de l'utilisateur a deconnecter");
            }else{
                strcpy(buffer,"\nMot de passe incorrect");
                clients[id_client].error++;
            }
            send_client(buffer,clients[id_client].socket);
        }else{
            strcpy(buffer, "Error: cmd unknown");
            send_client(buffer, clients[id_client].socket);
            clients[id_client].error++;
        }
    }else{
        int j;
        char tmp[SIZE];
        strcpy(tmp, "");
        for(j=0; j<*nbClients; j++){
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
int duplicated_name(char buffer[], Client * clients, int * nbClients){
    int i;
    for(i=0; i< *nbClients; i++){
        if(strcmp(buffer, clients[i].name) == 0){
            return 1;
        }
    }
    return 0;
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
            if(duplicated_name(buffer, clients, &nbClients)){
                send_client("Pseudo non valide", csock);
            }else{
                printf("Client %s est connecté\n", buffer);
                send_client("Vous etez bien connecté\nCommande Disponible :\n @auth autentification en tant qu'admin\n @list afficher la liste des utilisateurs connectés\n @exit quitter le tchat\n @whispser suivi d'un pseudo et du message a envoyer", csock);
                clients[nbClients].socket = csock;
                clients[nbClients].isAdmin = 0;
                clients[nbClients].nbMessages = 0;
                clients[nbClients].error = 0;
                clients[nbClients].time_connect = time(NULL);
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
            }            
        }else{
            int i, j;
            for(i=0; i<nbClients; i++){
                if(FD_ISSET(clients[i].socket, &set)){
                    if(recv_client(buffer, clients[i].socket) == 1){
                        printf("%s connection lost", clients[i].name);
                        fflush(stdout);
                        close(clients[i].socket);
                        remove_client(clients, i, &nbClients, &max);                        
                        break;
                    }else{
                        cmdManager(buffer, clients, &nbClients, i, &max);
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
