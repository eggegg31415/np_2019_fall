#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#define SA struct sockaddr
#define MAXLINE 1000000
#define max(a,b) a>b?a:b
int main(){
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1235);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
    fd_set rset;
    FD_ZERO(&rset);

    char msg[MAXLINE];
    while(1){
        int maxfd;
        FD_SET(0, &rset);
        FD_SET(sockfd, &rset);
        maxfd = max(0, sockfd)+1;

        select(maxfd, &rset, NULL, NULL, NULL);
        if(FD_ISSET(sockfd, &rset)){
            char msg[MAXLINE];
            if(read(sockfd, msg, sizeof(msg)) > 0)
                printf("%s", msg);
            else if(read(sockfd, msg, sizeof(msg)) == 0){
                printf("The server has closed the connection\n");
                close(sockfd);
                break;
            }
        }
        if(FD_ISSET(0, &rset)){
            char msg[MAXLINE];
            fgets(msg, MAXLINE, stdin);
            write(sockfd, msg, sizeof(msg));
            if(strncmp(msg, "exit", 4) == 0)
                break;
        }
    }
    close(sockfd);
    return 0;
}
