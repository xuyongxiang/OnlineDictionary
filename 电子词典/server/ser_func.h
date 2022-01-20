#ifndef SER_FUNC_H
#define SER_FUNC_H
#include <sqlite3.h>
sqlite3 * copyDictToDataBase(sqlite3 *db);

sqlite3 * createUserTable(sqlite3 *db);
sqlite3 * createHistoryRecord(sqlite3 *db);

int recv_register(int newfd,char * name,char * password,sqlite3 *db);
int recv_login(int newfd,char * name,char * password,sqlite3 *db);
int recv_search(int,char *,char *,sqlite3 *);
int recv_history(int,char *,sqlite3 *);
int recv_quit(char *,sqlite3 *);

#endif
