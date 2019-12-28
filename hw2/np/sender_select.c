#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define SA struct sockaddr
#define ROTATE 90
#define MAXMSG 1024
#define MAXLINE 1000
typedef struct msg_info{
    int seq;
    int len;
    char data[MAXLINE];
}sndmsg;
FILE *input;
int total_seq = 1;
int begin_seq = 0;
sndmsg buffer[ROTATE];

void resnd(int sockfd, SA *servaddr, int servlen){
    char line[MAXMSG];
    int n, seq;
    bzero(line, sizeof(line));

    while(1){
        n = recvfrom(sockfd, line, MAXMSG, 0, servaddr, &servlen);
        if(n > 0){
            seq = atoi(line);
            if(seq > begin_seq && total_seq != 0){
                seq %= ROTATE;
                sendto(sockfd, &buffer[seq], sizeof(sndmsg), 0, servaddr, servlen);
            }
            else if(seq == -1*total_seq)
                return;
            bzero(line, sizeof(line));
        }
    }
    begin_seq = total_seq;
}
void snd(int sockfd, SA *servaddr, int servlen){
    sndmsg msg;
    bzero(&msg, sizeof(msg));

    while((msg.len = read(fileno(input), msg.data, MAXLINE)) > 0){
        msg.seq = total_seq;
        buffer[total_seq%ROTATE] = msg;
        sendto(sockfd, &msg, sizeof(msg), 0, servaddr, servlen);
        if(total_seq%ROTATE == ROTATE-1){
            resnd(sockfd, servaddr, servlen);
            bzero(buffer, sizeof(buffer));
        }
        bzero(&msg, sizeof(msg));
        total_seq ++;
    }

    //end of file
    msg.seq = total_seq;
    msg.len = -1;
    for(int i=0; i<5; i++)
        sendto(sockfd, &msg, sizeof(msg), 0, servaddr, servlen);
    total_seq --;
    resnd(sockfd, servaddr, servlen);
}
void hand_shake(int sockfd, SA *servaddr, int servlen){
    sndmsg msg;
    struct timeval timestamp;
    bzero(&msg, sizeof(msg));

    for(int i=1; i<=15; i++){
        msg.seq = -1*i;
        gettimeofday(&timestamp, NULL);
        sprintf(msg.data, "%d", (timestamp.tv_sec%100)*1000000 + timestamp.tv_usec);
        sendto(sockfd, &msg, sizeof(msg), 0, servaddr, servlen);
        bzero(&msg, sizeof(msg));
    }
}
int main(int argc, char *argv[]){
    if(argc != 4){
        printf("sender_select <file> <ip addr> <port>\n");
        exit(1);
    }
    int sockfd;
    struct sockaddr_in servaddr;
    bzero(buffer, sizeof(buffer));

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
    hand_shake(sockfd, (SA *)&servaddr, sizeof(servaddr));
    snd(sockfd, (SA *)&servaddr, sizeof(servaddr));
    fclose(input);
    close(sockfd);
}
