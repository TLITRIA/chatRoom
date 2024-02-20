#ifndef __STD_SQLITE3_H_
#define __STD_SQLITE3_H_


// 指令示例

extern char *cmd_drop;
extern char *cmd_create;
extern char *cmd_insert;
extern char *cmd_insert_many;
extern char *cmd_del;
extern char *cmd_add_col1;
extern char *cmd_add_col2;
extern char *cmd_del_col1;
extern char *cmd_change_col2;
extern char *cmd_get_id;
extern char *cmd_set;




/* 打开db工程 */
sqlite3 *sqlite_Open(char *path);

/* 输入指令 */
int sqlite_Input(sqlite3 *db, char *cmd);

/* 输入指令并获取返回值 */
char **sqlite_Get(sqlite3 *db, char *cmd, int *pRow, int *pCol);

#endif