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
char name[LISTENQ][MAXLINE];
fd_set allset;

void func(int sockfd, int cli){
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
		client[cli] = -2;
		close(sockfd);
		FD_CLR(sockfd, &allset);
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
	maxfd = listenfd;
	maxi = -1;
	for(i=0; i<LISTENQ; i++){
		client[i] = -1;
		balance[i] = 0;
		count[i] = 0;
		sprintf(name[i], " ");
	}
	int nready;
	fd_set rset;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	while(1){
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(listenfd, &rset)){
			connfd = accept(listenfd, NULL, NULL);
			char n[MAXLINE];
			while(1){
				read(connfd, n, sizeof(n));
				if(strncmp(n, "CLIENTID:", 8) == 0)
					break;
			}
			char *token = strtok(n, " ");
			token = strtok(NULL, "");
			token[5] = 0;
			for(i=0; i<LISTENQ; i++)
				if(strncmp(name[i], token, 5) == 0)
					break;
			if(i == LISTENQ){
				for(i=0; i<LISTENQ; i++)
					if(client[i] == -1){
						write(connfd, token, sizeof(token));
						sprintf(name[i], "%s", token);
						break;
					}
			}
			client[i] = connfd;

			// write(connfd, "jizz", sizeof("jizz"));
			FD_SET(connfd, &allset);
			//printf("client in %d %d\n", connfd, i);
			if(connfd > maxfd) maxfd = connfd;
			if(i > maxi) maxi = i;
			if(--nready <= 0) continue;
		}	
		for(i=0; i<=maxi; i++){
			int sockfd = client[i];
			if(sockfd < 0)
				continue;
			if(FD_ISSET(sockfd, &rset)){
				func(sockfd, i);
				if(--nready <= 0)
					break;
			}
		}
	}
}
