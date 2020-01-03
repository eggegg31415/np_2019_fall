#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

void handler(int signum) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
	return;
}

int main(int argc, char **argv) {
	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("socket() error");

	struct sockaddr_in srvAddr;
	bzero(&srvAddr, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(57345);
	if (inet_pton(AF_INET, argv[1], &srvAddr.sin_addr) <= 0)
		perror("inet_pton() error");

	if (connect(sockFd, (const struct sockaddr *) &srvAddr, sizeof(srvAddr)) < 0)
		perror("connect() error");

	struct sigaction act;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &act, NULL) < 0)
		perror("sigaction() error");

	pid_t pid;
	if ((pid = fork()) < 0)
		perror("fork() error");

	if (pid == 0) {
		while (1) {
			char buf[4096];
			if (recv(sockFd, buf, 4096, 0) < 0)
				perror("recv() error");
			printf("%s\n", buf);
		}
	}

	else {
		char buf[4096];
		while (fgets(buf, 4096, stdin) != NULL) {
			if (send(sockFd, buf, strlen(buf) + 1, 0) < 0)
				perror("send() error");
		}
		sprintf(buf, "kill %d", pid);
		system(buf);
		return 0;
	}
}
