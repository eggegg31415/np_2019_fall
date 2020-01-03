#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<net/if.h>
#define MAXIF 10
#define ipsize 16

int main(){
    struct ifreq ifall[MAXIF];
    struct ifconf ioifconf;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    //get if vector
    ioifconf.ifc_buf = (void *)ifall;
    ioifconf.ifc_len = sizeof(ifall);
    //get all interfaces network info
    ioctl(sockfd, SIOCGIFCONF, &ioifconf);

    struct ifreq *ifpt;
    struct ifreq *ifend = (void *)((char *)ifall + ioifconf.ifc_len);
    for(ifpt =ifall; ifpt<ifend; ifpt++){
        struct ifreq ifr;
        if(ifpt->ifr_addr.sa_family != AF_INET)
            continue;

        //get interface name
        printf("[%s]\n", ifpt->ifr_name);

        //get mtu
        bzero(&ifr, sizeof(struct ifreq));
        memcpy(ifr.ifr_name, ifpt->ifr_name, sizeof(ifr.ifr_name));
        ioctl(sockfd, SIOCGIFMTU, &ifr);
        printf("MTU: %d\n", ifr.ifr_mtu);

        //get IP address
        uint32_t addri, maski;
        char addrc[ipsize], maskc[ipsize];
        addri = ((struct sockaddr_in *)&ifpt->ifr_addr)->sin_addr.s_addr;
        inet_ntop(AF_INET, &addri, addrc, ipsize);
        printf("IP Address: %s\n", addrc);

        //get netmask
        bzero(&ifr, sizeof(struct ifreq));
        memcpy(ifr.ifr_name, ifpt->ifr_name, sizeof(ifr.ifr_name));
        ioctl(sockfd, SIOCGIFNETMASK, &ifr);
        maski = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
        inet_ntop(AF_INET, &maski, maskc, ipsize);
        printf("Netmask: %s\n", maskc);

        //get mac address
        ioctl(sockfd, SIOCGIFFLAGS, &ifr);
        if(! ifr.ifr_flags & IFF_LOOPBACK){   //skip loopback interface
            bzero(&ifr, sizeof(struct ifreq));
            memcpy(ifr.ifr_name, ifpt->ifr_name, sizeof(ifr.ifr_name));
            ioctl(sockfd, SIOCGIFHWADDR, &ifr);
            printf("Hwaddr: ");
            for (int i=0; i<6; ++i){
                if(i != 0)
                    printf(":");
                printf("%02x", (unsigned char) ifr.ifr_addr.sa_data[i]);
            }
            printf("\n");
        }

        printf("\n");
    }
}
