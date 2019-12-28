#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#define SA struct sockaddr
#define MAXLINE 1025
#define LISTENQ 10

char name[LISTENQ][MAXLINE];
char ipaddr[LISTENQ][20];
int port[LISTENQ];
int client[LISTENQ];
fd_set rset, allset;

void func(int sockfd, int cli){
    char msg[MAXLINE];
    int i=0;
    read(sockfd, msg, sizeof(msg));
    if(strncmp(msg, "exit", 4) == 0){
        char remsg[MAXLINE];
        sprintf(remsg, "[Server] %s is offline.", name[cli]);
        for(i=0; i<LISTENQ; i++){
            if(client[i] >= 0 && i != cli)
                write(client[i], remsg, sizeof(remsg));
        }
        close(sockfd);
        FD_CLR(sockfd, &allset);
        sprintf(name[cli], "anonymous");
        sprintf(ipaddr[cli], "");
        port[cli] = 0;
        client[cli] = -1;
    }
    else if(strncmp(msg, "name", 4) == 0){
        char oldname[MAXLINE], remsg[MAXLINE];
        sprintf(oldname, "%s", name[cli]);
        for(i=0; i<LISTENQ; i++){
            if(strncmp(name[i], msg+5, sizeof(name[i])) == 0)
                break;
        }
        if(i != LISTENQ){
            sprintf(remsg, "[Server] ERROR: %s has been used by others.", name[i]);
            write(sockfd, remsg, sizeof(remsg));
        }
        else{
            sprintf(name[cli], "%s", msg+5);
            for(i=0; i<LISTENQ; i++){
                if(i == cli)
                    sprintf(remsg, "[Server] You're now known as %s.", name[cli]);
                else
                    sprintf(remsg, "[Server] %s is now known as %s.", oldname, name[cli]);
                write(client[i], remsg, sizeof(remsg));
            }
        }
    }
    else if(strncmp(msg, "who", 3) == 0){
        char whoname[MAXLINE], remsg[MAXLINE];
        for(i=0; i<LISTENQ; i++){
            if(client[i] > 0){
                if(i == cli)
                    sprintf(remsg, "[Server] %s %s:%d ->me", name[i], ipaddr[i], port[i]);
                else
                    sprintf(remsg, "[Server] %s %s:%d", name[i], ipaddr[cli], port[i]);
                write(sockfd, remsg, sizeof(remsg));
            }
        }
    }
    else if(strncmp(msg, "tell", 4) == 0){
        char tellmsg[3][MAXLINE];   //[who tell] [tell who] [tell msg]
        char remsg[MAXLINE];
        char *token = strtok(msg, " ");
        sprintf(tellmsg[0], "%s", name[cli]);
        token = strtok(NULL, " ");
        sprintf(tellmsg[1], "%s", token);
        token = strtok(NULL, "");
        sprintf(tellmsg[2], "%s", token);
        for(i=0; i<LISTENQ; i++){
            if(strncmp(tellmsg[1], name[i], sizeof(tellmsg[1])) == 0)
                break;
        }
        if(strncmp(name[cli], "anonymous", sizeof("anonymous")) == 0){
            sprintf(remsg, "[Server] ERROR: You are anonymous.");
            write(sockfd, remsg, sizeof(remsg));
        }
        else if(strncmp(name[i], "anonymous", sizeof("anonymous")) == 0){
            sprintf(remsg, "[Server] ERROR: The client to which you sent is anonymous.");
            write(sockfd, remsg, sizeof(remsg));
        }
        else if( i == LISTENQ){
            sprintf(remsg, "[Server] ERROR: The receiver doesn't exist.");
            write(sockfd, remsg, sizeof(remsg));
        }
        else{
            sprintf(remsg, "[Server] SUCCESS: Your message has been sent.");
            write(sockfd, remsg, sizeof(remsg));
            sprintf(remsg, "[Server] %s tell you %s", tellmsg[0], tellmsg[2]);
            write(client[i], remsg, sizeof(remsg));
        }
    }
    else if(strncmp(msg, "group", 5) == 0){
        char group[3][MAXLINE];
        char *token = strtok(msg, " ");
        sprintf(group[0], "%s", name[cli]);
        token = strtok(NULL, " ");
        sprintf(group[1], "%s", token);
        token = strtok(NULL, "");
        sprintf(group[2], "%s", token);

        char *utoken = strtok(group[1], ",");
        while(utoken != NULL){
            for(i=0; i<LISTENQ; i++){
                if(strncmp(name[i], utoken, sizeof(name[i])) == 0)
                    write(client[i], group[2], sizeof(group[1]));
            }
            utoken = strtok(NULL, ",");
        }
    }
    else if(strncmp(msg, "yell", 4) == 0){
        char yellmsg[2][MAXLINE];   //[who yell] [yell msg]
        char remsg[MAXLINE];
        sprintf(yellmsg[0], "%s", name[cli]);
        sprintf(yellmsg[1], "%s", msg+5);
        sprintf(remsg, "[Server] %s yell %s", yellmsg[0], yellmsg[1]);
        for(i=0; i<LISTENQ; i++){
            if(client[i] >= 0)
                write(client[i], remsg, sizeof(remsg));
        }
    }
}

int main(int argc, char* argv[]){
    //Check argument number
    if(argc != 2 ){
        printf("./server <Port>\n");
        return 130;
    }

    //Initialize
    int listenfd, connfd;
    int nready;
    struct sockaddr_in servaddr, cliaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( atoi(argv[1]) );
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    //Prepare client array
    int maxfd, maxi, i;
    maxfd = listenfd;
    maxi = -1;
    for(i=0; i<LISTENQ; i++){
        client[i] = -1;
        sprintf(name[i], "anonymous");
        sprintf(ipaddr[i], "");
        port[i] = 0;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    //Loop
    while(1){
        rset = allset;
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        socklen_t clilen = sizeof(cliaddr);
        if(FD_ISSET(listenfd, &rset)){
            //Cew client connection
            int clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (SA*) &cliaddr, &clilen);
            if(connfd < 0)
                printf("Connected fail\n");

            //Add new user fd to array
            for(i=0; i<LISTENQ; i++){
                if(client[i] < 0)
                    break;
            }
            if(i == LISTENQ)
                printf("Too many client\n");
            else{
                int j;
                char tmp[MAXLINE];
                client[i] = connfd;
                inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ipaddr[i], sizeof(tmp));
                port[i] = ntohs(cliaddr.sin_port);
                for(j=0; j<LISTENQ; j++){
                    if(j == i)
                        sprintf(tmp, "[Server] Hello, anonymous! From: %s:%d", ipaddr[i], port[i]);
                    else
                        sprintf(tmp, "[Server] Someone is coming!");
                    write(client[j], tmp, sizeof(tmp));
                }
            }
            //Add new user to set
            FD_SET(connfd, &allset);
            if(connfd > maxfd)  maxfd = connfd;
            if(i > maxi)        maxi = i;
            if(--nready <= 0)   continue;
        }
        for(i=0; i<=maxi; i++){
            //Check all clients for data
            int sockfd = client[i];
            if(sockfd < 0)  //empty
                continue;
            if(FD_ISSET(sockfd, &rset)){
                func(sockfd, i);
                if(--nready <= 0)
                    break;
            }
        }
    }
    return 0;
}
