#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define SA struct sockaddr
#define MAXLINE 1024
#define LISTENQ 10

char name[MAXLINE];
struct trans{
    int len;
    int ctl;    //0: file content; 1: file info; 2: text
    char file[MAXLINE];
    char data[MAXLINE];
};

void fun(int sockfd, char msg[]){
    char *token = strtok(msg, " ");
    if(strncmp(token, "put", 3) == 0){
        token = strtok(NULL, "");
        token = strtok(token, "\n");
        FILE *fp = fopen(token, "r");
        struct trans snddata;
        int check;

        //get file size
        if(fp == NULL){
            printf("No such file exist!!\n");
            return;
        }
        fseek(fp, 0, SEEK_END);
        int filesize = ftell(fp);
        snddata.len = filesize;
        snddata.ctl = 1;
        sprintf(snddata.file, "%s", token);
        write(sockfd, &snddata, sizeof(snddata));
        fclose(fp);
        fp = fopen(token, "r");

        //send data
        while(snddata.len = read(fileno(fp), snddata.data, MAXLINE)){
            snddata.ctl = 0;
            sprintf(snddata.file, "%s", token);
            check = write(sockfd, &snddata, sizeof(snddata));
            printf("check vlaue: %d\n", check);
        }
    }
    else if(strncmp(token, "sleep", 5) == 0){
        token = strtok(NULL, "");
        if(token == NULL){
            printf("Sleep <sec>\n");
            return;
        }
        int time = atoi(token);
        for(int i=1; i<=time; i++){
            printf("Sleep %d\n", i);
            sleep(1);
        }
        printf("Client wake up\n");
    }
    else if(strncmp(token, "exit", 4) == 0){
        close(sockfd);
        exit(0);
    }
    else{
        printf("Error Command\n");
    }
}
void rcvmsg(int sockfd){
    struct trans rcvdata;
    char msg[MAXLINE];
    int len;
    fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    while(1){
        if((len = read(fileno(stdin), msg, sizeof(msg))) > 0)
            fun(sockfd, msg);
        if((len = read(sockfd, &rcvdata, sizeof(rcvdata))) > 0){
            if(rcvdata.ctl == 2)
                printf("%s", rcvdata.data);
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
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( atoi(argv[2]) );
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    sprintf(name, "%s", argv[3]);


    if((connect(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0){
        printf("Connect error!!\n");
        return 1;
    }

    //connection built
    printf("Welcome to the dropbox-like server: %s\n", name);
    write(sockfd, name, MAXLINE);
    rcvmsg(sockfd);
}
