#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in_systm.h>
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<unistd.h>
#include<errno.h>
#include<sys/time.h>
#define SA struct sockaddr
#define MAXLINE 100
#define bufsize 1500
#define datalen 56
char sndbuf[1500];
int seq = 0;
int sockfd;

int tvsub(struct timeval *left, struct timeval *right){
    int sec = (left->tv_sec - right->tv_sec)*1000;
    int usec = (left->tv_usec - right->tv_usec)/1000;

    return sec+usec;
}
int procicmp(char *ptr, int len, struct timeval *recvtv){
    struct icmp *remsg = (struct icmp *)(ptr+16);

    if(remsg->icmp_type = ICMP_ECHOREPLY){
        sendtv = (struct timeval *)remsg->icmp_data;
        int rtt = tvsub(&sendtv, &tval);
        printf("%s (%s) RTT= %.3f\n", ip, hostname, (float)rtt/1000);
        return 1;
    }
    return 0;
}
unsigned int checksum(unsigned short *buf, int len){
    unsigned int sum = 0;
    while(len > 1) {
        sum += *buf++;
        len -= 2;
    }
    if (len == 1) {
        sum += (unsigned char *)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return ~sum;
}
void sendicmp(){
    struct icmp *msg;

    msg = (struct msg *)buf;
    msg->msg_type = msg_ECHO;
    msg->msg_code = 0;
    msg->msg_id = pid;
    msg->msg_seq = seq++;
    memset(msg->msg_data, 0xA5, dataLen);
    gettimeofday((struct timeval *)msg->msg_data, NULL);

    msg->msg_cksum = checksum((unsigned short *) msg, datalen+8);

    sendto(sockfd, msg, len, 0, sock.saSend, sock.saLen);
}
int main(int argc, char *argv[]){
    if(argc != 2){
        printf("pingsub <subnet>\n");
        return 1;
    }
    for(int host = 1; host <= 254; host++){
        char ip[128];
        sprintf(ip, "%s.%d", argv[1], host);

        //get ip hostname name
        char hostname[MAXLINE];
        struct sockaddr_in tmp;
        tmp.sin_family = AF_INET;
        tmp.sin_addr.s_addr = inet_addr(ip);
        getnameinfo((SA *)&tmp, sizeof(tmp), hostname, sizeof(hostname), NULL, 0, 0);
        //printf("%s %s\n", ip, hostname);

        struct timeval timeout, sndtv;
        struct msghdr msg;
        struct iovec iov;
        char recvbuf[bufsize];

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        sendicmp(&sndtv);

        int result, waittime, n;
        struct timeval tval;
        do{
            n = recvmsg(sockfd, &msg, 0);
            result = procicmp(recvbuf, n, &tval);
            waittime = tvsub(&sndtv, &tval);
        }while(!result && waittime < 3000);
    }
}
