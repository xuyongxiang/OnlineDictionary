#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "cli_func.h"
#include <stdlib.h>
#include <unistd.h>
int main(int argc, const char *argv[])
{
	if(argc<3)
	{
		printf("请输入IP和端口号\n");
		return -1;
	}
	int sfd = socket(AF_INET,SOCK_STREAM,0);
	if(sfd<0)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(atoi(argv[2]));
	sin.sin_addr.s_addr = inet_addr(argv[1]);
	if(connect(sfd,(struct sockaddr*)&sin,sizeof(sin))<0)
	{
		perror("connect");
		return -1;
	}
	displaymain(sfd);	
	return 0;
}

