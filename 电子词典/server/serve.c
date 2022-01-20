#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ser_func.h"
#include <sqlite3.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define ERR_MSG(msg) do{\
	printf("%d",__LINE__);\
	perror(msg);\
}while(0)
typedef struct
{
	char type;
	char name[30];
	char password[50];
	char word[30];
}__attribute__((packed)) agreeMent;
struct nfdSeq
{
	sqlite3 * db;
	int newfd;
};
void * recv_func(void * arg)
{
	agreeMent ag;
	int res = 0;
	struct nfdSeq seq = *(struct nfdSeq * )arg;
	while(1)
	{
		memset(&ag,0,sizeof(ag));
		res = recv(seq.newfd,&ag,sizeof(ag),0);
		if(res<0)
		{
			ERR_MSG("recv");
			return NULL;
		}
		else if (res ==0)
		{
			printf("客户端关闭\n");
			return NULL;
		}
		char type = ag.type;
		switch (type)
		{
		case 'R':
			recv_register(seq.newfd,ag.name,ag.password,seq.db);
			break;
		case 'L':
			recv_login(seq.newfd,ag.name,ag.password,seq.db);
			break;
		case 'S':
			recv_search(seq.newfd,ag.name,ag.word,seq.db);
			break;
		case 'H':
			recv_history(seq.newfd,ag.name,seq.db);
			break;
		case 'Q':
			recv_quit(ag.name,seq.db);
			break;
		}
	}
	pthread_exit(NULL);
}
int main(int argc, const char *argv[])
{
	if(argc<3)
	{
		printf("请输入IP和端口号\n");
		return -1;
	}
	sqlite3 * db = NULL;;
	//导入数据库
 	db = copyDictToDataBase(db);
	//创建用户表
	db = createUserTable(db);
	//创建历史记录表
	db = createHistoryRecord(db);
	//创建网络连接
	int sfd = socket(AF_INET,SOCK_STREAM,0);
	if(sfd<0)
	{
		ERR_MSG("socket");
		return -1;
	}
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(atoi(argv[2]));
	sin.sin_addr.s_addr = inet_addr(argv[1]);
	int reuse = 1;
	if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0)
	{
		ERR_MSG("setsockopt");
		return -1;
	}
	if(bind(sfd,(struct sockaddr*)&sin,sizeof(sin))<0)
	{
		ERR_MSG("bind");
		return -1;
	}
	if(listen(sfd,10)<0)
	{
		ERR_MSG("listen");
		return -1;
	}
	struct sockaddr_in cin;
	int newfd = 0;
	int len = sizeof(cin);
	pthread_t tid;
	struct nfdSeq seq;
	seq.db = db;
	while(1)
	{
		newfd = accept(sfd,(struct sockaddr *)&cin,&len);
		seq.newfd = newfd;
		pthread_create(&tid,NULL,recv_func,&seq);
		pthread_detach(tid);
	}
	close(newfd);
	close(sfd);
	return 0;
}
