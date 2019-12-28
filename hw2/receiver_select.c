#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/time.h>
#include<netinet/in.h>
#define SA struct sockaddr
#define MAXLINE 1000
#define ROTATE 90
#define max(a,b) a>b?a:b
#define min(a,b) a<b?a:b
typedef struct msg_info{
    int seq;
    int len;
    char data[MAXLINE];
}rcvmsg;
FILE *output;
int notend = 1;
int begin_seq = 0;  //the begin of recv range
int last_seq = 0;   //the last ack seq
int total_seq = 0;  //the end of recv range
int rtt_usec = 0;
rcvmsg buffer[ROTATE];

int read_timeout(int sockfd, int sec, int usec){
    fd_set rset;
    struct timeval tv;
    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);

    tv.tv_sec = sec;
    tv.tv_usec = usec;
    return select(sockfd+1, &rset, NULL, NULL, &tv);
}
void check(rcvmsg msg){
    if(msg.seq < 0)
        return;
    if(msg.len < 0){
        total_seq = msg.seq-1;
        notend = 0;
        return;
    }
    if(msg.seq == last_seq+1){
        write(fileno(output), msg.data, msg.len);
        last_seq ++;
        begin_seq = last_seq;
        total_seq = max(msg.seq, total_seq);
    }
    else if(msg.seq > begin_seq){
        buffer[msg.seq%ROTATE] = msg;
        total_seq = max(msg.seq, total_seq);
    }
}
void rercv(int sockfd, SA *cliaddr, int clilen){
    char line[MAXLINE];
    rcvmsg msg;
    bzero(line, sizeof(line));
    bzero(&msg, sizeof(msg));

    while(last_seq < total_seq){
        sprintf(line, "%d", last_seq+1);
        sendto(sockfd, line, MAXLINE, 0, cliaddr, clilen);
        if(read_timeout(sockfd, 0, rtt_usec) == 0){
            continue;
        }
        else{
            recvfrom(sockfd, &msg, sizeof(msg), 0, cliaddr, &clilen);
            check(msg);
            if(last_seq == total_seq)
                break;
            while(buffer[(last_seq+1)%ROTATE].seq == last_seq+1){
                last_seq ++;
                msg = buffer[last_seq%ROTATE];
                write(fileno(output), msg.data, msg.len);
                if(last_seq == total_seq)
                    break;
            }
        }
    }
    begin_seq = total_seq;
    sprintf(line, "%d", -1*total_seq);
    for(int i=0; i<5; i++)
        sendto(sockfd, line, MAXLINE, 0, cliaddr, clilen);
}
void rcv(int sockfd, SA *cliaddr, int clilen){
    char line[MAXLINE];
    rcvmsg msg;
    bzero(line, sizeof(line));
    bzero(&msg, sizeof(msg));

    while(notend){
        int n;
        if(read_timeout(sockfd, 0, rtt_usec) == 0){
            int rotate_time = total_seq/ROTATE + 1;
            total_seq = rotate_time*ROTATE-1;
            rercv(sockfd, cliaddr, clilen);
        }
        else{
            recvfrom(sockfd, &msg, sizeof(msg), 0, cliaddr, &clilen);
            check(msg);
            if(total_seq%ROTATE == ROTATE-1){
                rercv(sockfd, cliaddr, clilen);
            }
            bzero(line, sizeof(line));
        }
    }
    rercv(sockfd, cliaddr, clilen);
}
void hand_shake(int sockfd, SA *cliaddr, int clilen){
    struct timeval timestamp;
    rcvmsg msg;
    bzero(&msg, sizeof(msg));
    int cnt = 0;

    while(cnt < 4){
        recvfrom(sockfd, &msg, sizeof(msg), 0, cliaddr, &clilen);
        gettimeofday(&timestamp, NULL);
        int time = (timestamp.tv_sec%100)*1000000 + timestamp.tv_usec;
        int rtt = time - atoi(msg.data);
        rtt /= -1*msg.seq;
        rtt_usec = max(rtt_usec, rtt);
        cnt ++;
    }
}
int main(int argc, char *argv[]){
    if(argc != 3){
        printf("./receiver_select <file> <port>\n");
        exit(1);
    }
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    bzero(buffer, sizeof(buffer));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sockfd, (SA *)&servaddr, sizeof(servaddr));

    output = fopen(argv[1], "wb");
    hand_shake(sockfd, (SA *)&cliaddr, sizeof(cliaddr));
    rcv(sockfd, (SA *)&cliaddr, sizeof(cliaddr));
    fclose(output);
    close(sockfd);
}
