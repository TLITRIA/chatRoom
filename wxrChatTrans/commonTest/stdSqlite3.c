#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "stdSqlite3.h"

char *cmd_drop = "drop table if exists user";
char *cmd_create = "create table if not exists user (id int primary key not NULL, name text not null, passwords text not null)";
char *cmd_insert = "insert into user values(0, 'wc', '564654');";
char *cmd_insert_many = "insert into user values(1, '12ew', 'wqdwqd');"
                        "insert into user values(2, 'dwqd', 'wqdwqd');"
                        "insert into user values(3, '12', 'wqdwqd');"
                        "insert into user values(4, 'dwqd', '21');"
                        "insert into user values(5, 'weds', 'acs132');"
                        "insert into user values(6, 'dszs', '13rt312');";
char *cmd_del = "delete from user where id = 0";
char *cmd_add_col1 = "alter table user add address text";
char *cmd_add_col2 = "alter table user add phone text";
char *cmd_del_col1 = "alter table user drop address";
char *cmd_change_col2 = "alter table user alter column phone int";
char *cmd_get_id = "select id,name,passwords from user where passwords = 'wqdwqd' order by name desc";
char *cmd_set = "update user set name = 'WXR' where id = 6";


/* 打开db工程 */
sqlite3 *sqlite_Open(char *path)
{
    sqlite3 *db = NULL;
    int ret = sqlite3_open(path, &db);
    if (ret != SQLITE_OK)
    {
        perror("sqlite open error");
    }
    return db;
}


/* 输入指令 */
int sqlite_Input(sqlite3 *db, char *cmd)
{
    char *errormsg = NULL;
    const char *cmd_const = cmd;

    int ret = sqlite3_exec(db, cmd_const, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite exec error:%s\n", errormsg);
        printf("ret=%d\n", ret);
        // sqlite3_free(errormsg); 不知道有什么用
        #if 0
        sqlite3_close(db);  // 关闭db
        exit(-1);
        #endif
    }
    return ret;
}

/* 输入指令并获取返回值 */
char **sqlite_Get(sqlite3 *db, char *cmd, int *pRow, int *pCol)
{   
    char **result = NULL; 
    char *errormsg = NULL;
    int ret = sqlite3_get_table(db, cmd, &result, pRow, pCol, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table error:%s\n", errormsg);
        printf("ret=%d\n", ret);
    }

    return result;
}
