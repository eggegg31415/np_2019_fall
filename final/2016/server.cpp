#include<stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>
#include <fcntl.h>
#include <string>
#include <fstream>
#include <vector>
using namespace std;


int main(int argc, char** argv){
		
	int fd_listen[5];
	struct sockaddr_in servaddr;
	int port[5] = {10788,10155,10123,10589,10952};

	//set server address
	
	for (int i =0 ; i<5; i++){
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port[i]);

	//prepare socket
	if ((fd_listen[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error" << endl;
		exit(-1);
	}
        int on=1;
        if(setsockopt(fd_listen[i],SOL_SOCKET,SO_REUSEADDR,(void*)&on,sizeof(on))<0)
	cout<<"false";
	//bind socket with addr
	if (bind(fd_listen[i], (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		cout << "bind error" << endl;
		exit(-1);
	}

	//listen
	if (listen(fd_listen[i], 100) < 0)
	{
		cout << "listen error" << endl;
		exit(-1);
	}
	}
	int fd_accept[5];
	for (int j =0 ; j<5; j++){
		if((fd_accept[j] = accept(fd_listen[j],(struct sockaddr *)NULL,NULL)) < 0)
		{
			if(errno == EINTR)
				continue;
			else
				cout << "accept error" << endl;
		}
	}
	
	return 0;
 }
