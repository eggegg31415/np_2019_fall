#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string>

int main() {
	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("socket() error");

	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
		perror("setsockopt() error");

	struct sockaddr_in srvAddr;
	bzero(&srvAddr, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(57345);
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockFd, (const struct sockaddr *) &srvAddr, sizeof(srvAddr)) < 0)
		perror("bind() error");

	while (1) {
		ssize_t len;
		char buf[4096];
		struct sockaddr_in cliAddr;
		socklen_t addrLen = sizeof(cliAddr);

		if ((len = recvfrom(sockFd, buf, 4096, 0, (struct sockaddr *) &cliAddr, &addrLen)) < 0) {
			if (errno == EWOULDBLOCK)
				continue;
			else
				perror("recvfrom() error");
		}

		int numA = strtol(buf, NULL, 10);

		if ((len = recvfrom(sockFd, buf, 4096, 0, (struct sockaddr *) &cliAddr, &addrLen)) < 0) {
			if (errno == EWOULDBLOCK)
				continue;
			else
				perror("recvfrom() error");
		}

		int numB = strtol(buf, NULL, 10);
		int numC = numA + numB;
		sprintf(buf, "%d", numC);

		if (sendto(sockFd, buf, strlen(buf) + 1, 0, (const struct sockaddr *) &cliAddr, addrLen) < 0)
			perror("sendto() error");
	}
}
