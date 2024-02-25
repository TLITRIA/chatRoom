#include "sqlite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum status
{
    FLASE , //失败
    TRUE,   //成功
    MALLOC_ERROR,
};

static int callback(void *data, int argc, char **argv, char **azColName)
{
    int *count = (int *)data;
    *count = atoi(argv[0]);
    return 0;
}


/* 数据库初始化 */
int sqliteInit(SQL **s, const char *file)
{
    SQL *d = (SQL *)malloc(sizeof(SQL));
    if (d == NULL)
    {
        return MALLOC_ERROR;
    }
    int ret = sqlite3_open(file, &d->db);
    if (ret != SQLITE_OK)
    {
        printf("open sql error\n");
        free(s);
        return FLASE;
    }
    *s = d;
    return TRUE;
}

/* 数据库查找数据是否存在 */
int sqliteSearchISInfo(SQL *s, const char *sql)
{
    char **result = NULL;
    int row = 0;
    int column = 0;
    char *errormsg = NULL;

    int ret = sqlite3_get_table(s->db, sql, &result, &row, &column, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite exec error:%s\n", errormsg);
        return FLASE;
    }
    sqlite3_free_table(result);
    return row;
}

/* sql语句执行函数 */
int sqlExecute(SQL *s, const char *sql)
{ 
    char *errormsg = NULL;
    int ret = sqlite3_exec(s->db, sql, NULL, NULL, &errormsg);
    printf("ret:%d\n", ret);
    if (ret != SQLITE_OK)
    {
        printf("sqlite exec error:%s\n", errormsg);
        return FLASE;
    }
    return TRUE;
}

/* 获取数据库中指元素 */
int sqliteGetVal(SQL *s, const char *sql, char *ptr, int arrange, int pos)
{
    if (arrange < 0 || pos < 0)
    {
        return FLASE;
    }
    char **result = NULL;
    int row = 0;
    int column = 0;
    char *errormsg = NULL;
    int ret = sqlite3_get_table(s->db, sql, &result, &row, &column, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("get val error:%s\n", errormsg);
        exit(-1);
    }
    if (ptr != NULL && pos == 0)
    {
        strcpy(ptr, result[row * column + arrange]);
    }
    else if (ptr != NULL && pos != 0)
    {
        strcpy(ptr, result[pos * column]);
    }
    sqlite3_free_table(result);
    return row;
}

/* 判断群聊是非为空 */
int judgeGroupEmpty(SQL *s, const char *sql)
{
    char *errMsg = 0;
    int count = 0;
    if(sqlite3_exec(s->db, sql, callback, &count, &errMsg) != SQLITE_OK)
    {
        printf("error msg:%s\n", sqlite3_errmsg(s->db));
        sqlite3_free(errMsg);
        return FLASE;
    }

    //printf("行数是%d\n", count);
    if (count == 0)
    {
        return TRUE;
    }
    
    return FLASE;
}
