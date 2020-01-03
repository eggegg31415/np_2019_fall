#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

int dataLen = 56;
int sockFd;
pid_t pid;
char sendBuf[1500];
int nSent = 0;
int verbose = 0;
char tempWa[200];

struct proto {
	struct hostent *host;
	struct sockaddr *saSend;
	struct sockaddr *saRecv;
	socklen_t saLen;
	int icmpProto;
} sock;

struct addrinfo *hostServ(const char *host, const char *serv, int family, int sockType);
char *sockNtopHost(const struct sockaddr *sa, socklen_t saLen);
void tvSub(struct timeval *out, struct timeval *in);
int procIcmp(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvRecv);
void sendIcmp();
uint16_t inCksum(uint16_t *addr, int len);

int main(int argc, char **argv) {
	char *host;
	struct addrinfo *addrIn;
	char *h;

	host = argv[1];
	pid = getpid() & 0xFFFF;

	int subIp;
	for (subIp = 1; subIp <= 254; ++subIp) {
		char ip[128];
		sprintf(ip, "%s.%d", host, subIp);

		addrIn = hostServ(ip, NULL, 0, 0);
		h = sockNtopHost(addrIn->ai_addr, addrIn->ai_addrlen);

		sock.saSend = addrIn->ai_addr;
		sock.saRecv = calloc(1, addrIn->ai_addrlen);
		sock.saLen = addrIn->ai_addrlen;
		sock.icmpProto = IPPROTO_ICMP;
		in_addr_t ipAddr = inet_addr(ip);
		//sock.host = gethostbyaddr(&ipAddr, sizeof(ipAddr), AF_INET);

		struct sockaddr_in tempSa;
		tempSa.sin_family = AF_INET;
		inet_pton(AF_INET, ip, &tempSa.sin_addr);
		getnameinfo((const struct sockaddr *) &tempSa, sizeof(tempSa), tempWa, sizeof(tempWa), NULL, 0, 0);

		char recvBuf[1500];
		char ctrlBuf[1500];
		struct msghdr msg;

		struct iovec iov;
		ssize_t n;
		struct timeval tval, sendTval;

		sockFd = socket(sock.saSend->sa_family, SOCK_RAW, sock.icmpProto);
		if (sockFd < 0)
			perror("socket() error");
		int size = 60 * 1024;
		setsockopt(sockFd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
		struct timeval timeoutTv;
		timeoutTv.tv_sec = 3;
		timeoutTv.tv_usec = 0;
		setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &timeoutTv, sizeof(timeoutTv));

		sendIcmp(&sendTval);

		iov.iov_base = recvBuf;
		iov.iov_len = sizeof(recvBuf);
		msg.msg_name = sock.saRecv;
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_control = ctrlBuf;

		int result;
		struct timeval waitTv;
		do {
			msg.msg_namelen = sock.saLen;
			msg.msg_controllen = sizeof(ctrlBuf);
			n = recvmsg(sockFd, &msg, 0);

			if (n < 0) {
				if (errno == EINTR) {
					continue;
				}
				else if (errno == EAGAIN || errno == EWOULDBLOCK) {
					result = 0;
					break;
				}
				else {
					perror("recvmsg() error");
					exit(2);
				}
			}

			gettimeofday(&tval, NULL);
			result = procIcmp(recvBuf, n, &msg, &tval);

			waitTv = sendTval;
			tvSub(&waitTv, &tval);
		} while (!result && waitTv.tv_sec < 3);

		if (!result)
			printf("%s (%s) no response\n", ip, tempWa/*sock.host->h_name*/);

		sleep(1);
	}
	return 0;
}

struct addrinfo *hostServ(const char *host, const char *serv, int family, int sockType) {
	int n;
	struct addrinfo hints, *res;
	memset(&hints, 0x0, sizeof(struct addrinfo));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = family;
	hints.ai_socktype = sockType;

	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo() error\n");
		exit(1);
	}

	return res;
}

char *sockNtopHost(const struct sockaddr *sa, socklen_t saLen) {
	static char str[128];
	if (sa->sa_family == AF_INET) {
		struct sockaddr_in *sin = (struct sockaddr_in *) sa;
		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)))
			return str;
	}

	return NULL;
}

void tvSub(struct timeval *out, struct timeval *in) {
	if ((out->tv_usec -= in->tv_usec) < 0) {
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

int procIcmp(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvRecv) {
	int hLen1, icmpLen;
	double rtt;
	struct ip *ip;
	struct icmp *icmp;
	struct timeval *tvSend;
	char recvIp[128];
	inet_ntop(AF_INET, &((struct sockaddr_in *) sock.saRecv)->sin_addr, recvIp, sizeof(recvIp));

	ip = (struct ip *) ptr;
	hLen1 = ip->ip_hl << 2;
	if (ip->ip_p != IPPROTO_ICMP) {
		fprintf(stderr, "IPPROTO error\n");
		return 0;
	}

	icmp = (struct icmp *) (ptr + hLen1);
	if ((icmpLen = len - hLen1) < 8) {
		fprintf(stderr, "hLen1 error\n");
		return 0;
	}

	if (icmp->icmp_type == ICMP_ECHOREPLY) {
		if (icmp->icmp_id != pid) {
			fprintf(stderr, "pid error\n");
			return 0;
		}
		if (icmpLen < 16) {
			fprintf(stderr, "icmpLen error\n");
			return 0;
		}

		tvSend = (struct timeval *) icmp->icmp_data;
		tvSub(tvRecv, tvSend);
		rtt = tvRecv->tv_sec * 1000.0 + tvRecv->tv_usec / 1000.0;

		printf("%s (%s) RTT=%.3fms\n", recvIp, tempWa/*sock.host->h_name*/, rtt);
		return 1;
	}
	else {
		return 0;
	}
}

void sendIcmp() {
	int len;
	struct icmp *icmp;

	icmp = (struct icmp *) sendBuf;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = pid;
	icmp->icmp_seq = nSent++;
	memset(icmp->icmp_data, 0xA5, dataLen);
	gettimeofday((struct timeval *) icmp->icmp_data, NULL);

	len = 8 + dataLen;
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = inCksum((u_short *) icmp, len);

	sendto(sockFd, sendBuf, len, 0, sock.saSend, sock.saLen);
}

uint16_t inCksum(uint16_t *addr, int len) {
	int nLeft = len;
	uint32_t sum = 0;
	uint16_t *w = addr;
	uint16_t answer = 0;

	while (nLeft > 1) {
		sum += *w++;
		nLeft -= 2;
	}
	if (nLeft == 1) {
		*(u_char *) (&answer) = *(u_char *) w;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;
	return answer;
}
