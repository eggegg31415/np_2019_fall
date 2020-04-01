#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<netinet/in.h>
#define SA struct sockaddr
#define MAXLINE 1000
FILE *output;
typedef struct msg_info{
    int seq;
    int len;
    char data[MAXLINE];
}rcvmsg;

void rcv(int sockfd, SA *cliaddr, int clilen){
    int total_seq;
    char line[MAXLINE];
    rcvmsg msg;
    recvfrom(sockfd, &msg, sizeof(msg), 0, cliaddr, &clilen);
    total_seq = msg.len;
    sprintf(line, "%d", total_seq);

    sendto(sockfd, line, MAXLINE, 0, cliaddr, clilen);

    for(int seqn=1; seqn<=total_seq; seqn++){
        bzero(&msg, sizeof(msg));
        recvfrom(sockfd, &msg, sizeof(msg), 0, cliaddr, &clilen);
        printf("%d %d %d\n", msg.seq, msg.len, seqn);
        if(msg.seq == seqn){
            write(fileno(output), msg.data, msg.len);
            sprintf(line, "%d", seqn);
            sendto(sockfd, line, MAXLINE, 0, cliaddr, clilen);
        }
        else if(msg.seq < seqn){
            sprintf(line, "%d", msg.seq);
            sendto(sockfd, line, MAXLINE, 0, cliaddr, clilen);
            seqn --;
        }
        else
            seqn --;
    }
    for(int i=0; i<100; i++)
        sendto(sockfd, line, MAXLINE, 0, cliaddr, clilen);
}
int main(int argc, char *argv[]){
    if(argc != 3){
        printf("No enough argument\n");
        exit(1);
    }
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sockfd, (SA *)&servaddr, sizeof(servaddr));

    output = fopen(argv[1], "wb");
    rcv(sockfd, (SA *)&cliaddr, sizeof(cliaddr));
    fclose(output);
    close(sockfd);
}
