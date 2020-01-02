#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<dirent.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#define SA struct sockaddr
#define LISTENQ 10
#define MAXLINE 1000
#define LEVEL 22
#define DELAY 20000

char username[LISTENQ][MAXLINE];
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

void download(char filename[], int num, int filesize){
    FILE *fp;
    struct trans snddata;
    char path[MAXLINE];
    int sockfd = client[num];

    sprintf(path, "%s/%s", username[num], filename);    //<username>/<filename>
    fp = fopen(path, "rb");

    //send file info
    snddata.len = filesize;
    snddata.ctl = 1;
    sprintf(snddata.file, "%s", filename);
    write(sockfd, &snddata, sizeof(snddata));

    //send file content
    while(snddata.len = read(fileno(fp), snddata.data, MAXLINE)){
        snddata.ctl = 0;
        sprintf(snddata.file, "%s", filename);
        write(sockfd, &snddata, sizeof(snddata));
        usleep(DELAY);
    }
    fclose(fp);
}

void broadcast(char filename[], int src){
    FILE *fp;
    int target[LISTENQ], filesize = size[src];
    int tarnum = 0;
    char name[MAXLINE], path[MAXLINE];
    struct trans snddata;
    size[src] = 0;
    cnt[src] = 0;

    sprintf(name, "%s", username[src]);
    sprintf(path, "%s/%s", name, filename);
    fp = fopen(path, "rb");

    //check sessions using the same username
    for(int i=0; i<LISTENQ; i++){
        if(client[i] > 0){
            if(i != src && strncmp(name, username[i], sizeof(name)) == 0)
                target[tarnum ++] = i;
        }
    }
    for(int i=0; i<tarnum; i++)
        download(filename, target[i], filesize);
}

void rcvmsg(int sockfd, int num){
    struct trans rcvdata, snddata;
    char msg[MAXLINE];
    int len;

   len = read(sockfd, &rcvdata, sizeof(rcvdata));
    if(len == 0){
        //printf("Someone exit\n");
        close(sockfd);
        client[num] = -1;
        sprintf(username[num], "");
    }
    else if(len > 0){
        if(rcvdata.ctl == 1){           //change data info
            char path[MAXLINE];
            sprintf(path, "%s/%s", username[num], rcvdata.file);   //<username>/<filename>
            printf("path: %s\n", path);

            size[num] = rcvdata.len;
            file[num] = fopen(path, "wb");
            snddata.ctl = 2;
        }
        else if(rcvdata.ctl == 0){      //upload data to server
            float unit = size[num]/LEVEL;
            int per;

            write(fileno(file[num]), rcvdata.data, rcvdata.len);
            cnt[num] += rcvdata.len;
            printf("%s: %d/%d\n", rcvdata.file, cnt[num], size[num]);

            if(cnt[num] == size[num]){
                printf("Trans end!!\n");

                snddata.ctl = 2;
                sprintf(snddata.data, "[Upload] %s Finish!\n", rcvdata.file);
                write(sockfd, &snddata, sizeof(snddata));
                fclose(file[num]);

                //transmit file to other session
                broadcast(rcvdata.file, num);
            }
        }
    }
}

void checkclient(int listenfd){
    int connfd;
    int maxfd = listenfd, maxi = -1;
    fd_set allset, rset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    while(1){
        rset = allset;
        int nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if(FD_ISSET(listenfd, &rset)){
            int i;
            char name[MAXLINE];

            connfd = accept(listenfd, NULL, NULL);
            for(i=0; i<LISTENQ; i++)
                if(client[i] < 0)
                    break;
            client[i] = connfd;
            read(connfd, name, MAXLINE);
            printf("%s is connected!\n", name);
            sprintf(username[i], "%s", name);
            fcntl(connfd, F_SETFL, O_NONBLOCK);

            FD_SET(connfd, &allset);
            maxfd = connfd > maxfd ? connfd : maxfd;
            maxi = i > maxi ? i : maxi;

            //build user directory
            DIR* dir = opendir(name);
            struct dirent *file;
            if(dir){
                while((file = readdir(dir)) != NULL){
                    if(strncmp(file->d_name, ".", 1) == 0)
                        continue;
                    char path[MAXLINE];
                    sprintf(path, "%s/%s", name, file->d_name);
                    FILE *fp = fopen(path, "r");
                    fseek(fp, 0, SEEK_END);
                    int filesize = ftell(fp);
                    fclose(fp);
                    download(file->d_name, i, filesize);
                }
                closedir(dir);
            }
            else{
                mkdir(name, 0777);
            }
            if(--nready <= 0)
                continue;
        }
        for(int i=0; i<=maxi; i++){
            int sockfd = client[i];
            if(sockfd < 0)  //empty
                continue;
            if(FD_ISSET(sockfd, &rset)){
                rcvmsg(sockfd, i);
                if(--nready <= 0)
                    break;
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

    //start receving data
    checkclient(listenfd);
}
