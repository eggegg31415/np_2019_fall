#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#define SA struct sockaddr
#define MAXLINE 1025
#define max(a,b) a>b?a:b

char name[MAXLINE];
fd_set rset;

void climsg(int sockfd){
    char msg[MAXLINE], command[10];
    int n;
    scanf("%s", command);
    if(strncmp(command, "exit", 4) == 0){
        msg[MAXLINE];
        write(sockfd, "exit", sizeof("exit"));
        bzero(msg, sizeof(msg));
        close(sockfd);
        exit(0);
    }
    else if(strncmp(command, "name", 4) == 0){
        char msg[MAXLINE];
        scanf(" ", msg);    //read the blank before msg
        fgets(msg, MAXLINE, stdin);
        int len = strlen(msg)-1;
        msg[len] = msg[len+1];      //clean '\n' before '\0'
        int flag = 0, i;
        for(i=0; i<len; i++){
            if( !(msg[i] <= 'z' && msg[i]>='a') &&  !(msg[i] <= 'Z' && msg[i] >= 'A')){
                flag = 1;
                break;
            }
        }
        if(len < 3 || len > 13 || flag == 1)
            printf("[Server] ERROR: Username can only consists of 2~12 English letters.\n");
        else if(strncmp(msg, "anonymous", 9) == 0)
            printf("[Server] ERROR: Username cannot be anonymous.\n");
        else{
            char tmp[MAXLINE];
            sprintf(tmp, "%s %s", command, msg);
            write(sockfd, tmp, sizeof(tmp));
        }
    }
    else if(strncmp(command, "who", 3) == 0){
        char tmp[MAXLINE];
        sprintf(tmp, "%s %s", command, msg);
        write(sockfd, tmp, sizeof(tmp));
    }
    else if(strncmp(command, "tell", 4) == 0){
        char msg[MAXLINE];
        scanf(" ", msg);
        fgets(msg, MAXLINE, stdin);
        int len = strlen(msg)-1;
        msg[len] = msg[len+1];
        char tmp[MAXLINE];
        sprintf(tmp, "%s %s", command, msg);
        write(sockfd, tmp, sizeof(tmp));
    }
    else if(strncmp(command, "yell", 4) == 0){
        char msg[MAXLINE];
        scanf(" ", msg);
        fgets(msg, sizeof(msg), stdin);
        int len = strlen(msg)-1;
        msg[len] = msg[len+1];
        char tmp[MAXLINE];
        sprintf(tmp, "%s %s", command, msg);
        write(sockfd, tmp, sizeof(tmp));
    }
    else if(strncmp(command, "group", 5) == 0){
        char msg[MAXLINE];
        scanf(" ", msg);
        fgets(msg, sizeof(msg), stdin);
        int len = strlen(msg)-1;
        msg[len] = msg[len+1];
        char tmp[MAXLINE];
        sprintf(tmp, "%s %s", command, msg);
        write(sockfd, tmp, sizeof(tmp));
    }
    else{
        printf("[Server] ERROR: Error command.\n");
    }
    bzero(msg, sizeof(msg));
    bzero(command, sizeof(command));
}

int main(int argc, char* argv[]){
    //Check argument
    if (argc != 3){
        printf("./client <IP Address> <Port>\n");
        return 130;
    }

    //Initialize
    int sockfd;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Socket Error\n");
        return 130;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( atoi(argv[2]) );
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    FD_ZERO(&rset);

    //Connect to server
    if((connect(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0){
        printf("Connect Error\n");
        return 130;
    }
    else{
        char tmp[MAXLINE];
        read(sockfd, tmp, sizeof(tmp));
        printf("%s\n", tmp);
    }

    //Loop
    while(1){
        int maxfd;
        FD_SET(0, &rset);
        FD_SET(sockfd, &rset);
        maxfd = max(0, sockfd)+1;
        select(maxfd, &rset, NULL, NULL, NULL);
        //scoket is readable
        if(FD_ISSET(sockfd, &rset)){
            char msg[MAXLINE];
            if(read(sockfd, msg, sizeof(msg)) > 0)
                printf("%s\n", msg);
        }
        //input is readable
        if(FD_ISSET(0, &rset)){
            climsg(sockfd);
        }
    }
    close(sockfd);
}
