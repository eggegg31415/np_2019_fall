#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string>

int main(int argc, char **argv) {
	int sockFd;
	if ((sockFd = socket(AF_LOCAL, SOCK_DGRAM, 0)) < 0)
		perror("socket() error");

	struct ifconf ifc;
	struct ifreq *ifr;
	char buf[2048];

	ifc.ifc_len = 2048;
	ifc.ifc_buf = buf;
	ifr = (struct ifreq *) buf;

	if (ioctl(sockFd, SIOCGIFCONF, &ifc) < 0)
		perror("ioctl() error");

	if (argc == 2) {
		unsigned newMtu = strtoul(argv[1], NULL, 10);
		ifr->ifr_mtu = newMtu;

		if (ioctl(sockFd, SIOCSIFMTU, ifr) < 0)
			perror("ioctl() error");
	}

	else {
		if (ioctl(sockFd, SIOCGIFMTU, ifr) < 0)
			perror("ioctl() error");

		printf("MTU of loopback (lo): %d\n", ifr->ifr_mtu);
	}

	return 0;
}
