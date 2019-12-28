#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#define SA struct sockaddr
#define MAXLINE 1024
#define max(a,b) a>b?a:b

int main(int argc, char *argv[]){
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    connect(sockfd, (SA *)&servaddr, sizeof(servaddr));
    fd_set rset;
    FD_ZERO(&rset);

    char msg[MAXLINE];
    while(1){
        int maxfd;
        FD_SET(sockfd, &rset);
        FD_SET(0, &rset);
        maxfd = max(0, sockfd) + 1;
        select(maxfd, &rset, NULL, NULL, NULL);
        if(FD_ISSET(sockfd, &rset)){
            if(read(sockfd, msg, sizeof(msg)) > 0)
                printf("%s\n", msg);
            else if(read(sockfd, msg, sizeof(msg)) == 0)
                break;
        }
        if(FD_ISSET(0, &rset)){
            fgets(msg, MAXLINE, stdin);
            if(strncmp(msg, "EXIT", 4) == 0){
                write(sockfd, msg, sizeof(msg));
                break;
            }
            write(sockfd, msg, sizeof(msg));
        }
    }
    close(sockfd);
    return 0;
}