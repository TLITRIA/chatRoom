#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "stdSqlite3.h"

#define TESTDB_PATH "test.db"


/*
用户信息表
用户ID  用户名  密码

聊天记录表（群聊、私聊）
用户ID  用户名  发表时间    内容

*/
// todo两个表共享数据？？

int main()
{

    printf("sqlite3_libversion:%s\n", sqlite3_libversion());

    sqlite3 *testdb = sqlite_Open(TESTDB_PATH);
    char *errormsg = NULL;
    int ret = -1;

    // 删除表
    sqlite_Input(testdb, cmd_drop);
    // 创建表
    sqlite_Input(testdb, cmd_create);
    // 插入行
    sqlite_Input(testdb, cmd_insert);
    sqlite_Input(testdb, cmd_insert_many);
    // 删除行
    sqlite_Input(testdb, cmd_del);
    // 新增列
    sqlite_Input(testdb, cmd_add_col1);
    sqlite_Input(testdb, cmd_add_col2);
    // 删除列
    sqlite_Input(testdb, cmd_del_col1);
    //sqlite_Input(testdb, cmd_del_col1);
    // todo修改列的属性 sqlite3不支持，要创建新表
    //sqlite_Input(testdb, cmd_change_col2);

    // 模糊查询数据，结果排序
    char **result = NULL; 
    int row = 0;
    int col = 0;
    
    result = sqlite_Get(testdb, cmd_get_id, &row, &col);

    for (int idx = 0; idx <= row; idx++)
    {
        for (int jdx = 0; jdx < col; jdx++)
        {
            printf("%s\t", result[idx * col + jdx]);
        }
        printf("\n");
    }

    // 更新数据库数据
    sqlite_Input(testdb, cmd_set);



    /* 关闭数据库 */
    sqlite3_close(testdb);

    return 0;
}