#ifndef SER_FUNC_H
#define SER_FUNC_H
#include <sqlite3.h>
int copyDictToDataBase();

int createUserTable();
int createHistoryRecord();

int recv_register(int newfd,char * name,char * password);
int recv_login(int newfd,char * name,char * password);
int recv_search(int,char *,char *);
int recv_history(int,char *);
int recv_quit(char *);

#endif
