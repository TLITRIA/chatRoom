#ifndef __SQLITE_H_
#define __SQLITE_H_

#include <sqlite3.h>

typedef struct sqlite
{
    sqlite3 *db;
}SQL;

/* 数据库初始化 */
int sqliteInit(SQL **s, const char *file);

/* 数据库查找数据是否存在 */
int sqliteSearchISInfo(SQL *s, const char *sql);

/* sql语句执行函数 */
int sqlExecute(SQL *s, const char *sql);

/* 获取数据库中指元素 */
int sqliteGetVal(SQL *s, const char *sql, char *ptr, int arrange, int pos);

/* 判断群聊是否为空 */
int judgeGroupEmpty(SQL *s, const char *sql);


#endif
