#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>

std::vector<int> fds;
std::vector<int> ports;
int maxFd = 0;

int main(int argc, char **argv) {
	for (int port = 10000; port <= 11000; ++port) {
		int sockFd;
		if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			perror("socket() error");

		int flags = fcntl(sockFd, F_GETFL, 0);
		if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) < 0)
			perror("fcntl() error");

		struct sockaddr_in srvAddr;
		bzero(&srvAddr, sizeof(srvAddr));
		srvAddr.sin_family = AF_INET;
		srvAddr.sin_port = htons(port);
		if (inet_pton(AF_INET, argv[1], &srvAddr.sin_addr) <= 0)
			perror("inet_pton() error");

		if (connect(sockFd, (const struct sockaddr *) &srvAddr, sizeof(srvAddr)) < 0) {
			if (errno == EINPROGRESS) {
				fds.push_back(sockFd);
				ports.push_back(port);
				maxFd = sockFd > maxFd ? sockFd : maxFd;
			}
		}
		else
			printf("TCP port number %d is open.\n", port);
	}

	while (1) {
		fd_set fs;
		FD_ZERO(&fs);
		for (auto fd: fds)
			FD_SET(fd, &fs);

		if (select(maxFd + 1, NULL, &fs, NULL, NULL) < 0)
			perror("select() error");

		for (int i = 0; i < fds.size(); ++i) {
			int fd = fds[i];
			if (FD_ISSET(fd, &fs)) {
				int ret;
				socklen_t len;
				getsockopt(fd, SOL_SOCKET, SO_ERROR, &ret, &len);
				if (ret == 0)
					printf("TCP port number %d is open.\n", ports[i]);
				fds.erase(fds.begin() + i);
				ports.erase(ports.begin() + i);
			}
		}

		if (fds.empty())
			break;
	}

	return 0;
}
