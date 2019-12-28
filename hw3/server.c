#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<fcntl.h>
#define SA struct sockaddr
#define LISTENQ 10
#define MAXLINE 1024
#define LEVEL 22

char username[LISTENQ][MAXLINE];
char progressbar[LEVEL+1][LEVEL+1];
int client[LISTENQ];
int size[LISTENQ];
int cnt[LISTENQ];
FILE* file[LISTENQ];
struct trans{
    int len;
    int ctl;    //0: file content; 1: file info; 2: text
    char file[MAXLINE];
    char data[MAXLINE];
};

void download(char filename[], int src){
    FILE *fp = fopen(filename, "rb");
    int target[LISTENQ], filesize = size[src];
    int tarnum = 0;
    char name[MAXLINE];
    struct trans snddata;

    size[src] = 0;
    cnt[src] = 0;

    sprintf(name, "%s", username[src]);
    for(int i=0; i<LISTENQ; i++){
        if(client[i] > 0){
            if(i != src && strncmp(name, username[i], sizeof(name)) == 0)
                target[tarnum ++] = i;
        }
    }
    //send file info
    snddata.len = filesize;
    snddata.ctl = 1;
    sprintf(snddata.file, "%s", filename);
    for(int i=0; i<tarnum; i++){
        int num = target[i];
        write(client[num], &snddata, sizeof(snddata));
    }

    //send file content
    while(snddata.len = read(fileno(fp), snddata.data, MAXLINE)){
        snddata.ctl = 0;
        sprintf(snddata.file, "%s", filename);
        for(int i=0; i<tarnum; i++){
            int num = target[i];
            write(client[num], &snddata, sizeof(snddata));
        }
    }
}

void rcvmsg(int sockfd, int num){
    struct trans rcvdata, snddata;
    char msg[MAXLINE];
    int len;

    len = read(sockfd, &rcvdata, sizeof(rcvdata));
    if(len == 0){
        printf("Someone exit\n");
        close(sockfd);
        client[num] = -1;
        sprintf(username[num], "");
    }
    else if(len > 0){
        if(rcvdata.ctl == 1){           //change data info
            size[num] = rcvdata.len;
            file[num] = fopen(rcvdata.file, "w");
            snddata.ctl = 2;
            sprintf(snddata.data, "[Upload] %s Start!\n", rcvdata.file);
            write(sockfd, &snddata, sizeof(snddata));
        }
        else if(rcvdata.ctl == 0){      //upload data to server
            float unit = size[num]/LEVEL;
            int per;
            int output = fileno(file[num]);

            write(output, rcvdata.data, rcvdata.len);
            cnt[num] += rcvdata.len;
            for(int j=0; j<=LEVEL; j++){
                if(unit*j >= cnt[num] || j == LEVEL){
                    per = j;
                    break;
                }
            }
            sprintf(snddata.data, "Progress : [%s]\r", progressbar[per]);
            snddata.ctl = 2;
            write(sockfd, &snddata, sizeof(snddata));
            if(cnt[num] == size[num]){
                printf("Trans end!!\n");
                sprintf(snddata.data, "Progress : [%s]\n", progressbar[LEVEL]);
                snddata.ctl = 2;
                write(sockfd, &snddata, sizeof(snddata));

                snddata.ctl = 2;
                sprintf(snddata.data, "[Upload] %s Finish!\n", rcvdata.file);
                write(sockfd, &snddata, sizeof(snddata));
                fclose(file[num]);

                //transmit file to other session
                download(rcvdata.file, num);
            }
        }
    }
}

void checkclient(int listenfd){
    int connfd;

    while(1){
        if((connfd = accept(listenfd, NULL, NULL)) > 0){
            int i;
            char name[MAXLINE];
            for(i=0; i<LISTENQ; i++)
                if(client[i] < 0)
                    break;
            client[i] = connfd;
            read(connfd, name, MAXLINE);
            printf("%s is connected!\n", name);
            fcntl(connfd, F_SETFL, O_NONBLOCK);
        }
        for(int i=0; i<LISTENQ; i++){
            if(client[i] > 0){
                rcvmsg(client[i], i);
            }
        }
    }
}

int main(int argc, char *argv[]){
    //check argument number
    if(argc != 2){
        printf("./server <port>\n");
        return 1;
    }

    //initialize
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( atoi(argv[1]) );
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    fcntl(listenfd, F_SETFL, O_NONBLOCK);

    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    bzero(cnt, sizeof(cnt));
    for(int i=0; i<LISTENQ; i++){
        client[i] = -1;
        sprintf(username[i], "");
    }
    for(int i=0; i<=LEVEL; i++){
        for(int j=0; j<i; j++)
            progressbar[i][j] = '#';
        for(int j=LEVEL-1; j>=i; j--)
            progressbar[i][j] = ' ';
    }

    //start receving data
    checkclient(listenfd);
}
