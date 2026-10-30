#define BUFFER_SIZE 1024
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include<ctype.h>


char* convert_to_uppercase(char input[1024]){
    for(int i=0;input[i]!='\0';i++){
        input[i]=(char)toupper(input[i]);
    }
    return input;
}

char* recv_from_server(int connect_client_fd, char buffer_out[1024]){
    recv(connect_client_fd,&buffer_out,strlen(buffer_out)-1,0);
    return convert_to_uppercase(buffer_out);    
}

void display_banner(){
    printf("\n------------------------------------------------\n");
    printf("\n-----------------AWALE GAME---------------------\n");
    printf("\n------------------------------------------------\n");
}

void display_menu(){
    printf("/register [username] \n");
    printf("/add-bio [username] [bio] \n");
    printf("/list-bio [username] \n");
    printf("/invite [username] \n");
    printf("/list-online-users \n");
    printf("/list-current-games \n");
    printf("/quit \n");
}


void main(int argc, char* argv[]){
    struct sockaddr_in server_addr;
    int connect_client_fd;
    char buffer_in[1024],buffer_out[1024];
    char username[256];
    char *prompt="Awale (not registered)";
    int nbBytes;

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

    printf("\n-------------------------------------\n");
    printf("Type /help for available commands");
    printf("\n-------------------------------------\n");

    do{      
        printf("%s> ",prompt);
        fflush(stdout);
        
        fgets(buffer_in,sizeof(buffer_in),stdin);
        buffer_in[sizeof(buffer_in)-1]='\0';
        buffer_in[strcspn(buffer_in,"\n")]='\0';
        
        if(strcmp(buffer_in,"/help")==0){
            display_menu();
            continue;
        }
        else{
            send(connect_client_fd,&buffer_in,strlen(buffer_in),0);
            printf("Sent Buffer: %s \n",buffer_in);
        }

        int nbBytes=recv(connect_client_fd,buffer_out,sizeof(buffer_out),0);        
        if(nbBytes>0){
            printf("nbBytes: %d \n",nbBytes);   
        }
        if(strcmp(buffer_out,"USERNAME_AVAILABLE")==0){
            char *command=strtok(buffer_in," ");
            char *username=strtok(NULL, " ");
            prompt=username;
        }
        printf("Returned: %s \n",buffer_out);                      

    } while(strcmp(buffer_in,"/quit")!=0);

    close(connect_client_fd);
    printf("--------------------------------------\n");
    printf("You left the server");
    printf("--------------------------------------\n");
    exit(EXIT_SUCCESS);   
}
