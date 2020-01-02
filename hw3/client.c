#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define max(a, b) a>b?a:b
#define SA struct sockaddr
#define MAXLINE 1000
#define LISTENQ 10
#define LEVEL 22
#define DELAY 20000

char name[MAXLINE];
char progressbar[LEVEL+1][LEVEL+1];
struct trans{
    int len;
    int ctl;    //0: file content; 1: file info; 2: text
    char file[MAXLINE];
    char data[MAXLINE];
};

void fun(int sockfd, char msg[], int pid){
    char *token = strtok(msg, " ");
    if(strncmp(token, "put", 3) == 0){
        token = strtok(NULL, "");
        token = strtok(token, "\n");
        FILE *fp = fopen(token, "r");
        struct trans snddata, rcvdata;

        //get file size
        if(fp == NULL){
            printf("No such file exist!!\n");
            return;
        }
        fseek(fp, 0, SEEK_END);
        int filesize = ftell(fp);
        int unit = filesize/LEVEL;
        int cnt = 0, per = 0;

        snddata.len = filesize;
        snddata.ctl = 1;
        sprintf(snddata.file, "%s", token);
        write(sockfd, &snddata, sizeof(snddata));
        fclose(fp);
        fp = fopen(token, "r");

        //send data
        printf("Pid: %d [Upload] %s Start!\n", pid, snddata.file);
        while(snddata.len = read(fileno(fp), snddata.data, MAXLINE)){
            snddata.ctl = 0;
            sprintf(snddata.file, "%s", token);
            write(sockfd, &snddata, sizeof(snddata));

            //print progressbar
            cnt += snddata.len;
            for(int i=0; i<=LEVEL; i++){
                if(unit*i >= cnt || i == LEVEL){
                    if(i > per){
                        per = i;
                        printf("Pid: %d Progress : [%s]\r", pid, progressbar[per]);
                        fflush(stdout);
                    }
                    break;
                }
            }
            usleep(DELAY);
        }
        printf("Pid: %d Progress : [%s]\n", pid, progressbar[LEVEL]);
    }
    else if(strncmp(token, "sleep", 5) == 0){
        token = strtok(NULL, "");
        if(token == NULL){
            printf("Pid: %d Sleep <sec>\n");
            return;
        }
        int time = atoi(token);
        for(int i=1; i<=time; i++){
            printf("Pid: %d Sleep %d\n", pid, i);
            sleep(1);
        }
        printf("Pid: %d Client wake up\n", pid);

        //struct trans snddata;
        //snddata.ctl = 2;
        //write(sockfd, &snddata, sizeof(snddata));
    }
    else if(strncmp(token, "exit", 4) == 0){
        close(sockfd);
        exit(0);
    }
    else{
        printf("Error Command\n");
    }
}
void rcvmsg(int sockfd, int pid){
    struct trans rcvdata;
    char msg[MAXLINE], rcvfile[MAXLINE];
    int len, rcvlen, cnt = 0;
    FILE *fp;
    fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    fd_set rset;
    FD_ZERO(&rset);

    while(1){
        FD_SET(0, &rset);
        FD_SET(sockfd, &rset);
        int maxfd = max(0, sockfd)+1;

        select(maxfd, &rset, NULL, NULL, NULL);
        if(FD_ISSET(0, &rset)){
            len = read(fileno(stdin), msg, sizeof(msg));
            fun(sockfd, msg, pid);
        }
        if(FD_ISSET(sockfd, &rset)){
            len = read(sockfd, &rcvdata, sizeof(rcvdata));
            if(rcvdata.ctl == 1){       //receive file info
                fp = fopen(rcvdata.file, "wb");
                rcvlen = rcvdata.len;
                sprintf(rcvfile, "%s", rcvdata.file);
                printf("Pid: %d [Download] %s Start!\n", pid, rcvfile);
            }
            else if(rcvdata.ctl == 0){  //receive file content
                write(fileno(fp), rcvdata.data, rcvdata.len);
                //generate progress bar
                float unit = rcvlen/LEVEL;
                int per = 0;
                for(int i=0; i<=LEVEL; i++){
                    if(unit*i >= cnt || i == LEVEL){
                        per = i;
                        break;
                    }
                }
                if(! (cnt%(1024*16))){
                    printf("Pid: %d Progress : [%s]\r", pid, progressbar[per]);
                    fflush(stdout);
                }

                //check eof
                cnt += rcvdata.len;
                if(cnt == rcvlen){
                    printf("Pid: %d Progress : [%s]\n", pid, progressbar[LEVEL]);
                    fflush(stdout);
                    printf("Pid: %d [Download] %s Finish!\n", pid, rcvfile);
                    cnt = 0;
                    rcvlen = 0;
                }
            }
            else if(rcvdata.ctl == 2){  //receive output msg
                printf("Pid: %d %s", pid, rcvdata.data);
                fflush(stdout);
            }
        }
    }
}

int main(int argc, char *argv[]){
    //check argument
    if(argc != 4){
        printf("./client <ip> <port> <username>\n");
        return 1;
    }

    //initialize
    int sockfd;
    int pid = getpid();
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( atoi(argv[2]) );
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    sprintf(name, "%s", argv[3]);
    for(int i=0; i<=LEVEL; i++){
        for(int j=0; j<i; j++)
            progressbar[i][j] = '#';
        for(int j=LEVEL-1; j>=i; j--)
            progressbar[i][j] = ' ';
        progressbar[i][LEVEL] = 0;
    }

    if((connect(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0){
        printf("Connect error!!\n");
        return 1;
    }

    //connection built
    printf("Pid: %d Welcome to the dropbox-like server: %s\n", pid, name);
    write(sockfd, name, MAXLINE);
    rcvmsg(sockfd, pid);
}
