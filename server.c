#define BUFFER_SIZE 1024
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <stdio.h>
#include <string.h>

void recv_from_client(int connect_socket_fd, char buffer_out[1024]){    
    ssize_t nbBytes=recv(connect_socket_fd,buffer_out,BUFFER_SIZE-1,0);
    buffer_out[nbBytes] = '\0';
}


void register_username(const char *username){
    FILE *file = fopen("awales_usernames.txt", "a");
    if (file == NULL) {
        return; 
    }
    fprintf(file,"%s\n",username);    
    fclose(file);    
}

int username_exists(const char *username)
{
    FILE *file = fopen("awales_usernames.txt", "r");
    char line[100];

    if (file == NULL) {
        file=fopen("awales_usernames.txt", "a");
        printf("Creating the file ... \n");  
        if(file==NULL){
            return 0;
        }
         
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, username) == 0) {
            fclose(file);
            return 1; 
        }
    }

    fclose(file);
    return -1;
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
                printf("Received username: %s \n",buffer_out);


                if(sizeof(buffer_out)>0){
                    int verif=username_exists(buffer_out);
                    printf("Verif: %d \n",verif);

                    switch(verif){
                        case -1:{
                            printf("Username available \n");
                            strcpy(buffer_in,"USERNAME AVAILABLE");
                            register_username(buffer_out);
                            printf("Registered the username \n");
                            break;
                        }
                        
                        case 1:{
                            printf("Username not available \n");
                            strcpy(buffer_in,"USERNAME NOT AVAILABLE");                            
                            break;
                        }
                    }                
                    send(connect_socket_fd,&buffer_in,sizeof(buffer_in)-1,0);
                                        
                }                  
            }while(strcmp(buffer_in,"/close")!=0);

            printf("[+] Closing the communication with the client \n");
            close(connect_socket_fd);
            exit(EXIT_SUCCESS);
        }
        close(connect_socket_fd);
    } 
}