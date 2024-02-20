#ifndef __STDSQLITE_H_
#define __STDSQLITE_H_
#include <stdbool.h>
#include "MyString.h"
#include "DoubleLinkList.h" //后加
#include <string.h>
struct TableInfoList
{
    MyStrList list;
    int column; // 一行的字符个数
};
typedef struct TableInfoList TIlist;
struct StdSqlite;
typedef struct StdSqlite SQL;

SQL * InitSqlite(const char *filepath);
bool SqliteExec(SQL *s,const char *sql);
TIlist GetTableInfo(SQL *s,const char *sql);
int searchIsExist(SQL *s,const char *sql); // 判断数据库表里是否有这个用户
int GetTableVal(SQL *s,const char *sql, char *ptr, int *val,int pos, int Row); //获取结果集中的某个元素
int GetTablefd(SQL *s,const char *sql, int *val, int pos); //获取结果集中的某个元素
DLlist GetTablelist(SQL *s,const char *sql); //后加
// int search1(char *n, SQL *s,const char *sql)//后加
int findpassword(char *n, SQL *s, const char *sql); // 自己
int findusername(char *n, SQL *s, const char *sql);
void ClearSqlite(SQL *s);
void FreeTableInfo(TIlist *l);



bool judgeGroupEmpty(SQL *s, const char *sql);

#endif