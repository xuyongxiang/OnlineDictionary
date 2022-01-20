#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include "ser_func.h"
typedef struct
{
	char type;
	char name[30];
	char password[50];
	char word[30];
	char mean[128];
	char errmsg[20];
}__attribute__((packed)) agreeMent;
sqlite3 * copyDictToDataBase(sqlite3 * db)
{
	if(sqlite3_open("user.db",&db)!=0)
	{
		printf("%s\n",sqlite3_errmsg(db));
		return NULL;
	}
	
	char * errmsg = NULL;
	char * sql = "create table  if not exists dict (word char,mean char)";
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return NULL;
	}
	printf("create dict success\n");
	sql = "select * from dict";
	char ** presult = NULL;
	int row,column;
	if(sqlite3_get_table(db,sql,&presult,&row,&column,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return NULL;
	}
	sqlite3_free_table(presult);
	if(row==0)
	{
		FILE * fp = fopen("./dict.txt","r");
		if(fp==NULL)
		{
			perror("fopen");
			return NULL;
		}
		char buf[100];
		char word[50];
		char mean[50];
		printf("正在导入数据...\n");
		char op[128];
		char * fg = NULL;
		while(1)
		{
			bzero(buf,sizeof(buf));
			fg = fgets(buf,sizeof(buf),fp);
			if(fg==NULL)
			{
				printf("导入完毕\n");
				break;
			}
			bzero(word,sizeof(word));
			bzero(mean,sizeof(mean));
			sscanf(buf,"%s %[^'\n']",word,mean);
			sprintf(op,"insert into dict values(\"%s\",\"%s\")",word,mean);
			if(sqlite3_exec(db,op,NULL,NULL,&errmsg)!=0)
			{
				printf("%s\n",errmsg);
				return NULL;
			}
		}
		fclose(fp);
	}
	return db;
}
sqlite3 * createUserTable(sqlite3 * db)
{
	

	char * errmsg = NULL;
	char * sql = "create table  if not exists user (name char primary key,password char,state int)";
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return db;
	}
	printf("用户表创建成功\n");
	sql="update  user set state=0";
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return db;
	}

	return db;

}
sqlite3 * createHistoryRecord(sqlite3 * db)
{
	char * sql = "create table  if not exists history (name char,word char,mean char,time char)";
	char * errmsg = NULL;
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return NULL;
	}
	printf("历史记录表创建成功\n");

	return db;

}
int recv_register(int newfd,char * name,char * password,sqlite3 * db)
{
	printf("收到注册请求\n");
	char sql[128];
	char * errmsg = NULL;
	sprintf(sql,"select * from user where name = \"%s\"",name);
	char ** presult = NULL;
	int row,column;
	if(sqlite3_get_table(db,sql,&presult,&row,&column,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return -1;
	}
	char str[128];
	int state = 0;
	if(row==0)
	{
		char  sql[128];
		char * errmsg = NULL;
		sprintf(sql,"insert into user values(\"%s\",\"%s\",%d)",name,password,state);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0)
		{
			printf("%s\n",errmsg);
			return -1;
		}
		sprintf(str,"注册成功");
	}
	else
	{
		sprintf(str,"该用户已注册");
	}
	sqlite3_free_table(presult);
	if(send(newfd,str,sizeof(str),0)<0)
	{
		perror("send");
		return -1;
	}
	return 0;
}
int recv_login(int newfd,char * name,char * password,sqlite3 * db)
{
	printf("收到登录请求\n");
	char sql[128];
	char * errmsg = NULL;
	sprintf(sql,"select * from user where name = \"%s\"",name);
	char ** presult = NULL;
	int row,column;
	if(sqlite3_get_table(db,sql,&presult,&row,&column,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return -1;
	}
	agreeMent ag;
	if(row==0)
	{
		ag.type = 'N';
		strcpy(ag.errmsg,"用户不存在");
	}
	else
	{
		if(strcmp(name,presult[3])==0)
		{
			if(strcmp(password,presult[4])==0)
			{
				if(strcmp(presult[5],"0")==0)
				{
					ag.type = 'S';
					strcpy(ag.errmsg,"登录成功");
					strcpy(ag.name,name);
					char  sql[128];
					char * errmsg = NULL;
					sprintf(sql,"update  user set state=%d where name=\"%s\"",1,name);
					if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0)
					{
						printf("%s\n",errmsg);
						return -1;
					}
				}
				else
				{
					ag.type = 'N';
					strcpy(ag.errmsg,"重复登录");
				}
			}
			else
			{
				ag.type = 'N';
				strcpy(ag.errmsg,"密码错误");
			}
		}
	}
	if(send(newfd,&ag,sizeof(ag),0)<0)
	{
		perror("send");
		return -1;
	}
	return 0;
}
int recv_search(int newfd,char * name,char * word,sqlite3 * db)
{

	printf("收到搜索请求\n");
	char sql[128];
	char * errmsg = NULL;
	sprintf(sql,"select * from dict where word = \"%s\"",word);
	char ** presult = NULL;
	int row,column;
	if(sqlite3_get_table(db,sql,&presult,&row,&column,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return -1;
	}
	
	char insertime[128];
	time_t t;
	struct tm* info = NULL;
	t = time(NULL);
	info = localtime(&t);
	sprintf(insertime, "%4d-%02d-%02d %02d:%02d:%02d\n",\
				info->tm_year+1900, info->tm_mon+1, info->tm_mday,\
				info->tm_hour, info->tm_min, info->tm_sec);
	agreeMent ag;
	char mean[128] = "";
	if(row>0)
	{
		strcpy(ag.word,word);
		strcpy(ag.mean,presult[3]);
	        strcpy(mean,presult[3]);
	}
	else 
	{
		strcpy(ag.word,word);
		strcpy(ag.errmsg,"Not Found");
		strcpy(ag.mean,"Not Found");
	}
	bzero(sql,sizeof(sql));
	sprintf(sql,\
	"insert into history values(\"%s\",\"%s\",\"%s\",\"%s\")",\
	name,word,mean,insertime);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return -1;
	}
	sqlite3_free_table(presult);
	if(send(newfd,&ag,sizeof(ag),0)<0)
	{
		perror("send");
		return -1;
	}
    return 0;

}
int recv_history(int newfd,char * name,sqlite3 * db)
{
	printf("收到查询历史记录请求\n");
	char sql[128];
	char * errmsg = NULL;
	sprintf(sql,"select word, mean, time from history where name = \"%s\"",name);
	char ** presult = NULL;
	int row,column;
	if(sqlite3_get_table(db,sql,&presult,&row,&column,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return -1;
	}
	int i = 0,j=0;
	if(row>0)
	{	
		char buf[128];
		for(i=0;i<row;i++)
		{
			bzero(buf,sizeof(buf));
			j+=3;
			sprintf(buf,"\t%s\t%s\t\t\t\t%s",presult[j],presult[j+1],presult[j+2]);	
			if(send(newfd,buf,sizeof(buf),0)<0)
			{
				perror("send");
				return -1;
			}
		}
	}
	if(send(newfd,&"****OVER FOUND****",sizeof("****OVER FOUND****"),0)<0)
	{
		{
			perror("send");
			return -1;
		}	
	}
	sqlite3_free_table(presult);
	return 0;
}
int recv_quit(char * name,sqlite3 * db)
{
	char * errmsg = NULL;

	char sql[128];
	sprintf(sql,"update  user set state=%d where name=\"%s\"",0,name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0)
	{
		printf("%s\n",errmsg);
		return -1;
	}
	return 0;
}

