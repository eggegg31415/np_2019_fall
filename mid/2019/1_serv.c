#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#define SA struct sockaddr
#define LISTENQ 10
#define MAXLINE 1024

int balance[LISTENQ];
int count[LISTENQ];
int client[LISTENQ];
char history[LISTENQ][MAXLINE][MAXLINE];

int func(int sockfd, int cli){
	char in[MAXLINE];
	int i;
	read(sockfd, in, sizeof(in));
	//write(sockfd, in, sizeof(in));
	if(strncmp(in, "HISTORY", 7) == 0){
		char msg[MAXLINE];
		if(count[cli] > 0){
			for(i=0; i<count[cli]; i++)
				write(sockfd, history[cli][i], sizeof(history[cli][i]));
			sprintf(msg, "$$$BALANCE: %d NTD", balance[cli]);
			write(sockfd, msg, sizeof(msg));	
		}
		else{
			char nomoney[MAXLINE] = "%%% No commands received so far";
			write(sockfd, nomoney, sizeof(nomoney));
			sprintf(msg, "$$$BALANCE: 0 NTD");
			write(sockfd, msg, sizeof(msg));
		}
	}
	else if(strncmp(in, "EXIT", 4) == 0){
		balance[cli] = 0;
		count[cli] = 0;
		return 0;
	}
	else if(strncmp(in, "DEPOSIT", 7) == 0){
		char *token = strtok(in, " ");
		int cnt = count[cli]++;
		int addm;
		token = strtok(NULL, " ");
		addm = atoi(token);
		token = strtok(NULL, " ");
		if(strncmp(token, "NTD", 3) == 0){
			balance[cli] += addm;
			sprintf(history[cli][cnt], "### DEPOSIT %d NTD", addm);
		}
		else if(strncmp(token, "USD", 3) == 0){
			balance[cli] += (addm * 30);
			sprintf(history[cli][cnt], "### DEPOSIT %d USD", addm);
		}
		char msg[MAXLINE];
		sprintf(msg, "### BALANCE: %d NTD", balance[cli]);
		write(sockfd, msg, sizeof(msg));
	}
	else if(strncmp(in, "WITHDRAW", 8) == 0){
		char *token = strtok(in, " ");
		int cnt = count[cli]++;
		int addm;
		token = strtok(NULL, " ");
		addm = atoi(token);
		token = strtok(NULL, " ");
		if(strncmp(token, "NTD", 3) == 0){
			if(balance[cli] >= addm){
				balance[cli] -= addm;
				sprintf(history[cli][cnt], "### WITHDRAW %d NTD", addm);
			}
			else{
				sprintf(history[cli][cnt], "### WITHDRAW %d NTD (FAILD)", addm);
				char nomoney[MAXLINE] = "!!! FAILED: Not enough money in the account";
				write(sockfd, nomoney, sizeof(nomoney));
			}
		}
		else if(strncmp(token, "USD", 3) == 0){
			if(balance[cli] >= (addm*30)){
				balance[cli] -= (addm*30);
				sprintf(history[cli][cnt], "### WITHDRAW %d USD", addm);
			}
			else{
				sprintf(history[cli][cnt], "### WITHDRAW %d USD (FAILD)", addm);
				char nomoney[MAXLINE] = "!!! FAILED: Not enough money in the account";
				write(sockfd, nomoney, sizeof(nomoney));
			}
		}
		char msg[MAXLINE];
		sprintf(msg, "### BALANCE: %d NTD", balance[cli]);
		write(sockfd, msg, sizeof(msg));
	}
	return 1;
}
int main(int argc, char *argv[]){
	int listenfd, connfd;
	struct sockaddr_in servaddr, cliaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	bzero(&cliaddr, sizeof(cliaddr)); 
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);
	
	int maxfd, maxi, i;
	for(i=0; i<LISTENQ; i++){
		client[i] = -1;
		balance[i] = 0;
		count[i] = 0;
	}
	

	while(1){
		connfd = accept(listenfd, NULL, NULL);
		i = 0;
		while(1){
			int re = func(connfd, i);
			if(re == 0)
				break;
		}
	}
}
