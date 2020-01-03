#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<net/if.h>
#define SA struct sockaddr
#define ipsize 16
#define MAXLINE 100

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("ipurl <ip | hostname>\n");
        return 1;
    }
    struct sockaddr_in tmp;

    //ip -> hostname
    if(inet_pton(AF_INET, argv[1], &(tmp.sin_addr)) == 1){
        char ip[ipsize];
        char hostname[MAXLINE];
        struct sockaddr_in tmp;
        sprintf(ip, "%s", argv[1]);

        tmp.sin_family = AF_INET;
        tmp.sin_addr.s_addr = inet_addr(ip);
        getnameinfo((SA *)&tmp, sizeof(tmp), hostname, sizeof(hostname), NULL, 0, 0);
        printf("%s %s\n", ip, hostname);
    }
    //hostname -> ip
    else{
        char hostname[MAXLINE];
        char ip[ipsize];
        struct addrinfo sndinfo, *rcvinfo, *ptr;
        sprintf(hostname, "%s", argv[1]);

        bzero(&sndinfo, sizeof(sndinfo));
        sndinfo.ai_family = AF_INET;
        sndinfo.ai_socktype = SOCK_STREAM;
        sndinfo.ai_flags = AI_CANONNAME;
        getaddrinfo(hostname, NULL, &sndinfo, &rcvinfo);

        ptr = rcvinfo;
        while(ptr){
            void *addr;
            if(ptr->ai_family == AF_INET){
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)ptr->ai_addr;
                addr=&(ipv4->sin_addr);
            }
            inet_ntop(AF_INET, addr, ip, sizeof(ip));
            printf("%s %s\n",hostname ,ip);
            ptr = ptr->ai_next;
        }
    }
}
