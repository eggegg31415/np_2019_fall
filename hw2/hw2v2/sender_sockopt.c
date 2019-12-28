#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<netdb.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define SA struct sockaddr
#define MAXLINE 1000
FILE *input;

typedef struct msg_info{
    int seq;
    int len;
    char data[MAXLINE];
}sndmsg;

void snd(int sockfd, SA *servaddr, int servlen){
    int freg, n;
    int total_seq = 0;
    char line[MAXLINE];
    sndmsg msg;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 5000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    fseek(input, 0, SEEK_END);
    int size = ftell(input);
    rewind(input);
    total_seq = size%MAXLINE ? size/MAXLINE+1 : size/MAXLINE;


    msg.seq = -1;
    msg.len = total_seq;
    while(1){
        sendto(sockfd, &msg, sizeof(msg), 0, servaddr, servlen);
        if((n = recvfrom(sockfd, line, MAXLINE, 0, servaddr, &servlen)) < 0){
            if(errno == EWOULDBLOCK)
                continue;
        }
        else{
            if(atoi(line) == total_seq)
                break;
            else
                continue;
        }
    }


    for(int seqn=1; seqn<=total_seq; seqn++){
        bzero(&msg, sizeof(msg));
        freg = read(fileno(input), msg.data, MAXLINE);
        msg.len = freg;
        msg.seq = seqn;
        while(1){
            sendto(sockfd, &msg, sizeof(msg), 0, servaddr, servlen);
            if((n = recvfrom(sockfd, line, MAXLINE, 0, servaddr, &servlen)) < 0){
                if(errno == EWOULDBLOCK)
                    continue;
            }
            else{
                if(atoi(line) == seqn)
                    break;
                else
                    continue;
            }
        }
    }
}

int main(int argc, char *argv[]){
    if(argc != 4){
        printf("No enough argument\n");
        exit(1);
    }
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[3]));
    if((servaddr.sin_addr.s_addr = inet_addr(argv[2])) == -1){
        struct hostent *host;
        host = gethostbyname(argv[2]);
        servaddr.sin_addr.s_addr = inet_pton(AF_INET, host->h_addr_list[0], &servaddr);
    }

    input = fopen(argv[1], "rb");
    snd(sockfd, (SA *)&servaddr, sizeof(servaddr));
    fclose(input);
    close(sockfd);
}
