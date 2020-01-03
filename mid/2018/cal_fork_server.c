#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>
#define SA struct sockaddr
#define MAXLINE 1000000

void sig_child(int signo){
    int pid;
    int stat;

    waitpid(-1, &stat, WNOHANG);

    return;
}


int main(){
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    void sig_child(int);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    bzero(&cliaddr, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1235);
    servaddr.sin_addr.s_addr = (INADDR_ANY);

    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    listen(listenfd, 10);

    signal(SIGCHLD, sig_child);
    while(1){
        connfd = accept(listenfd, (SA *)NULL, NULL);
        int childpid;
        if((childpid = fork()) == 0){
            close(listenfd);    //child no need to listen
            while(1){
                char in[MAXLINE];
                read(connfd, in, sizeof(in));
                char msg[MAXLINE];
                if(strncmp(in, "ADD", 3) == 0){
                    char *token = strtok(in, " ");
                    token = strtok(NULL, " ");
                    int ans = 0;
                    while(token != NULL){
                        ans += atoi(token);
                        token = strtok(NULL, " ");
                    }
                    sprintf(msg, "ADD: %d\n", ans);
                    write(connfd, msg, sizeof(msg));
                }
                else if(strncmp(in, "MUL", 3) == 0){
                    char *token = strtok(in, " ");
                    token = strtok(NULL, " ");
                    int ans = 1;
                    while(token != NULL){
                        ans *= atoi(token);
                        token = strtok(NULL, " ");
                    }
                    sprintf(msg, "MUL: %d\n", ans);
                    write(connfd, msg, sizeof(msg));
                }
                else if(strncmp(in, "exit", 4) == 0){
                    close(connfd);
                    exit(0);
                }
                else{
                    sprintf(msg, "Invalid Command\n");
                    write(connfd, msg, sizeof(msg));
                }
            }
        }
        close(connfd);
    }
    return 0;
}
