#define BUFFER_SIZE 1024
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include<ctype.h>
#include "client.h"

char* convert_to_uppercase(char input[1024]){
    for(int i=0;input[i]!='\0';i++){
        input[i]=(char)toupper(input[i]);
    }
    return input;
}

void send_to_server(int connect_client_fd, char buffer_in[1024]){ 
    if(send(connect_client_fd,buffer_in,strlen(buffer_in),0)!=-1){
        printf("Sent %s to the server\n",buffer_in);
    }
    
}

char* recv_from_server(int connect_client_fd, char buffer_out[1024]){
    recv(connect_client_fd,&buffer_out,strlen(buffer_out)-1,0);
    return convert_to_uppercase(buffer_out);    
}
char* select_username(){
    static char *username;
    printf("Choose a username: \n");        
    if (fgets(username, sizeof(username), stdin) != NULL) {
        username[strcspn(username, "\n")] = '\0';
    }
    return username;          
}

void main(int argc, char* argv[]){
    struct sockaddr_in server_addr;
    char *username;
    int connect_client_fd;
    char buffer[1024],buffer_output[1024];
    struct Client client;

    if(argc<2){
        printf("Usage: %s <IP_ADDRESS> <PORT>\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    if((connect_client_fd=socket(AF_INET,SOCK_STREAM,0))<0){
        printf("[-] Error when creating the socket \n");
        exit(EXIT_FAILURE);
    }
    
    if((inet_pton(AF_INET,argv[1],&server_addr.sin_addr))<1){
        printf("[-] Invalid IP address \n");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(8000);
    
    if(connect(connect_client_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        printf("[-] Could not connect to the server through the socket \n");
        exit(EXIT_FAILURE);
    }
    
    username=select_username();
    if(username!=NULL){
         printf("Username selected: %s\n",username);
    }
  
    
    send_to_server(connect_client_fd, username);
    printf("After calling send \n");

    if(strcmp(recv_from_server(connect_client_fd, buffer_output),"USERNAME AVAILABLE")!=0){            
        strcpy(client.username,username);
    }
    else{

    }        
    printf("\nclient's username: %s\n",client.username);



    do{ 
        printf("%s> ",client.username);
        fflush(stdout);
        
        fgets(buffer,sizeof(buffer),stdin);
        buffer[sizeof(buffer)-1]='\0';
        buffer[strcspn(buffer,"\n")]='\0';

        send(connect_client_fd,&buffer,sizeof(buffer),0);
    } while(strcmp(buffer,"/close")!=0);

    close(connect_client_fd);
    printf("--------------------------------------\n");
    printf("[*] Leaving the client application \n");
    printf("--------------------------------------\n");
    exit(EXIT_SUCCESS);   
}
