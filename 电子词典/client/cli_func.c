#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "cli_func.h"
#include <stdlib.h>
#include <unistd.h>
typedef struct
{
	char type;
	char name[30];
	char password[50];
	char word[30];
	char mean[128];
	char errmsg[20];
}__attribute__((packed)) agreeMent;
void displaymain(int sfd)
{

	char choose = 0;
	while(1)
	{
		printf("---------------------------------\n");
		printf("-------------1.注册--------------\n");
		printf("-------------2.登录--------------\n");
		printf("-------------3.退出--------------\n");
		printf("---------------------------------\n");
		choose = getchar();
		while(getchar()!=10);
		switch (choose)
		{
			case '1':
				send_register(sfd);
				break;
			case '2':
				send_login(sfd);
				break;
			case '3':
				return;
				break;
			default:
				printf("输入有误，请重新输入\n");
				continue;
		}
		printf("输入任意字符清屏\n");
		while(getchar()!=10);
		system("clear");
	}

}
int send_register(int sfd)
{
	agreeMent ag;
	ag.type = 'R';
	printf("请输入用户名>>>");
	fgets(ag.name,sizeof(ag.name),stdin);
	ag.name[strlen(ag.name)-1] = 0;
	printf("请输入密码>>>");
	fgets(ag.password,sizeof(ag.password),stdin);
	ag.password[strlen(ag.password)-1] = 0;
	if(send(sfd,&ag,sizeof(ag),0)<0)
	{
		perror("send");
		return -1;
	}
	char buf[128];
	if(recv(sfd,buf,sizeof(buf),0)<0)
	{
		perror("recv");
		return -1;
	}
	printf("%s\n",buf);
	return 0;

}
void displayNext(int sfd,char * name)
{
	char choose = 0;
	while(1)
	{
		printf("---------------------------------\n");
		printf("-----------1.查单词--------------\n");
		printf("-----------2.历史记录------------\n");
		printf("-----------3.返回上一级----------\n");
		printf("---------------------------------\n");
		choose = getchar();
		while(getchar()!=10);
		switch (choose)
		{
			case '1':
				send_search(sfd,name);
				break;
			case '2':
				send_history(sfd,name);
				break;
			case '3':
				send_back(sfd,name);
				return;
				break;
			default:
				printf("输入有误，请重新输入\n");
				continue;
		}
		printf("输入任意字符清屏\n");
		while(getchar()!=10);
		system("clear");

	}

}
int send_login(int sfd)
{
	agreeMent ag;
	ag.type = 'L';
	printf("请输入用户名>>>");
	fgets(ag.name,sizeof(ag.name),stdin);
	ag.name[strlen(ag.name)-1] = 0;
	printf("请输入密码>>>");
	fgets(ag.password,sizeof(ag.password),stdin);
	ag.password[strlen(ag.password)-1] = 0;
	if(send(sfd,&ag,sizeof(ag),0)<0)
	{
		perror("send");
		return -1;
	}
	memset(&ag,0,sizeof(ag));
	if(recv(sfd,&ag,sizeof(ag),0)<0)
	{
		perror("recv");
		return -1;
	}
	if(ag.type=='S')
	{
		displayNext(sfd,ag.name);
	}
	else
	{
		printf("%s\n",ag.errmsg);
	}

	return 0;
}
int send_search(int sfd,char * name)
{
	char ch[7] ="";
	agreeMent ag;
	printf("%d\n",sfd);
	while(1)
	{
		memset(&ag,0,sizeof(ag));
		ag.type = 'S';
		strcpy(ag.name,name);
		printf("请输入要查询的单词(输入#结束)>>>");
		bzero(ag.word,sizeof(ag.word));
		fgets(ag.word,sizeof(ag.word),stdin);
		ag.word[strlen(ag.word)-1]=0;
		if(strcmp(ag.word,"#")==0)
		{
			return -1;
		}
		if(send(sfd,&ag,sizeof(ag),0)<0)
		{
			perror("send");
			return -1;
		}
		memset(&ag,0,sizeof(ag));
		if(recv(sfd,&ag,sizeof(ag),0)<0)
		{
			perror("recv");
			return -1;
		}
		if(strcmp(ag.mean,"")==0)
		{
			printf("\t\t%s\n",ag.errmsg);
		}
		else
		{
			printf("\t\t%s %s\n",ag.word,ag.mean);
		}
		putchar(10);
		printf("\t\t****OVER FOUND****\n");
	}
	return 0;
}
int send_history(int sfd,char * name)
{
	agreeMent ag;
	ag.type = 'H';
	strcpy(ag.name,name);
	if(send(sfd,&ag,sizeof(ag),0)<0)
	{
		perror("send");
		return -1;
	}
	char buf[128];
	while(1)
	{
		bzero(buf,sizeof(buf));
		if(recv(sfd,buf,sizeof(buf),0)<0)
		{
			perror("recv");
			return -1;
		}
		printf("%s\n",buf);
		if(strcmp(buf,"****OVER FOUND****")==0)
		{
			break;
		}
	}
	return 0;
}
int send_back(int sfd,char * name)
{
	agreeMent ag;
	ag.type = 'Q';
	strcpy(ag.name,name);
	if(send(sfd,&ag,sizeof(ag),0)<0)
	{
		perror("send");
		return -1;
	}
	return 0;
}

