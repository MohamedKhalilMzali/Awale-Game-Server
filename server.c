#define BUFFER_SIZE 1024
#define MAX_PLAYERS 100
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "player.h"

static Player player_c[MAX_PLAYERS];
static int player_count=0;
static char *online_players[MAX_PLAYERS];
static int online_players_count=0;


void recv_from_client(int connect_socket_fd, char buffer_out[1024]){    
    ssize_t nbBytes=recv(connect_socket_fd,buffer_out,BUFFER_SIZE-1,0);
    buffer_out[nbBytes] = '\0';
}

int username_exists(const char *username)
{
    int exists=0;
    if(player_count>0){
        for(int i=0;i<player_count;i++){
            if( (strcmp(player_c[i].username,username)) == 0 ){
                exists=1;
            }
            else{
                exists=0;
            }
        }
    }
    return exists;
}    


char* register_username(const char *username)
{
    if(username==NULL){
        return NULL;
    }
    
    if(username_exists(username)!=0){
        return "USERNAME_NOT_AVAILABLE";
    }
    if(player_count>MAX_PLAYERS){
        return NULL;
    }
    
    Player player={0} ;       
    snprintf(player.username,sizeof(player.username),"%s",username);
    player.state=ONLINE_IDLE;
    player_c[player_count]=player;
    
    player_count++; 
    return "USERNAME_AVAILABLE";
}

Player* search_player(const char* username){
    Player* player={0};
    if(username==NULL){
        return NULL;
    }
    if(player_count>0){
        for(int i=0;i<player_count;i++){
            if( (strcmp(player_c[i].username,username)) == 0){
                return &player_c[i];
            }
        }
    }
    return NULL;
}

void display_player_info(const char* username){
    Player* player=search_player(username);
    char *state;
    if(player!=NULL){
        printf("\n------------------------\n");
        printf("Username: %s \n",player->username);
        printf("Bio: %s \n",player->bio);
        switch(player->state){
            case OFFLINE:
            {
                state="OFFLINE";               
                break;
            }
            case ONLINE_IDLE:
            {
                state="ONLINE_IDLE";   
                break;
            }
            case ONLINE_IN_GAME:
            {
                state="ONLINE_IN_GAME";
                break;
            }
        }
        printf("State: %s \n",state);
        printf("\n------------------------\n");
    }
}

char** display_online_players(){
    if(player_count>0){
        for(int i=0;i<player_count;i++){
            if(player_c[i].state==ONLINE_IDLE){
                online_players[online_players_count]=player_c[i].username;
                online_players_count++;
            }
        }
    }
    return online_players;
}


void add_player_bio(const char *username, const char *bio){
    Player *player=search_player(username);
    if(player!=NULL){
        strcpy(player->bio,bio);
    }
}

void main(){
    
    int listen_socket_fd, connect_socket_fd;
    struct sockaddr_in socket_addr, remote_addr;
    socklen_t remote_addr_len;
    int opt=1;
    char buffer_in[1024],buffer_out[1024];
    ssize_t size_buffer;
    int pid;

    if((listen_socket_fd=socket(AF_INET,SOCK_STREAM,0))<0){
        printf("[-] Error when creating the socket \n");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(listen_socket_fd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&opt,sizeof(opt))<0){
        printf("[-] Error when trying to set the socket options \n");
        exit(EXIT_FAILURE);
    }
    socket_addr.sin_family=AF_INET;
    socket_addr.sin_port=htons(8000);
    socket_addr.sin_addr.s_addr=INADDR_ANY;
    if(bind(listen_socket_fd,(struct sockaddr*)&socket_addr,sizeof(socket_addr))<0){
        printf("[-] Could not bind the socket to a port \n");
        exit(EXIT_FAILURE);
    }
    if(listen(listen_socket_fd,5)<0){
        printf("[-] Could not listen to the incoming connection \n");
        exit(EXIT_FAILURE);
    }
    signal(SIGCHLD,SIG_IGN);
    while(1){        
        connect_socket_fd=accept(listen_socket_fd,(struct sockaddr*)&remote_addr,&remote_addr_len);
        if(connect_socket_fd<0){
            printf("[-] Could not accept the incoming connection from the client \n");
            exit(EXIT_FAILURE);            
        }
        pid=fork();
        if(pid==0){
            close(listen_socket_fd);
            do{                                
                recv_from_client(connect_socket_fd,buffer_out);
                if(sizeof(buffer_out)>0){
                    
                    printf("Received buffer from client: %s\n",buffer_out);                    
                    
                    char *command=strtok(buffer_out," ");
                    
                    printf("command from client: %s\n",command);
                    
                    if(strcmp(command,"/register")==0){
                        char *username=strtok(NULL," ");
                        printf("Username: %s \n",username);
                        if(username==NULL){
                            printf("Usage: /register [username] \n");
                        }
                        else{
                            char* msg=register_username(username);     
                            printf("Message from registration: %s \n",msg);                       
                            send(connect_socket_fd,msg,strlen(msg),0);                            
                        }            
                    }

                    if(strcmp(command,"/list-online-users")==0){
                        char **online_p=display_online_players();                        
                        // for testing only
                        for(int i=0;i<online_players_count;i++){                        
                            printf("%s \n",online_p[i]);
                        }
                        send(connect_socket_fd,online_p,sizeof(online_p),0);                        
                    }

                    if(strcmp(command,"/add-bio")==0){

                    }

                    if(strcmp(command,"/quit")==0){
                        printf("[+] Closing the communication with the client \n");
                        close(connect_socket_fd);
                        exit(EXIT_SUCCESS);
                    }                    
                                        
                }                
                
            }while(strcmp(buffer_in,"/quit")!=0);

            printf("[+] Closing the communication with the client \n");
            close(connect_socket_fd);
            exit(EXIT_SUCCESS);
        }
        close(connect_socket_fd);
    } 
}