#include "StdSqlite.h"
#include "MyString.h"
#include <sqlite3.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "DoubleLinkList.h" //后加
struct StdSqlite
{
    sqlite3 *db;
};

SQL * InitSqlite(const char *filepath)
{
    SQL * s = (SQL*)malloc(sizeof(SQL));
    if(s == NULL)
             return NULL;
    if(sqlite3_open(filepath,&s->db) != SQLITE_OK)
    {
       //sqlite3_errmsg:返回最近的错误信息
        printf("open error:%s\n",sqlite3_errmsg(s->db));
        free(s);
        return NULL;
    }
    return s;
}


bool SqliteExec(SQL *s,const char *sql)
{
    if(sqlite3_exec(s->db,sql,NULL,NULL,NULL) != SQLITE_OK)
    {
        printf("error msg:%s\n",sqlite3_errmsg(s->db));
        return false;
    }
    return true;
}

TIlist GetTableInfo(SQL *s,const char *sql)
{
    TIlist list;
    
    InitStringList(&list.list);
    list.column = 0;

    char **result;
    int row = 0, column = 0;
    if(sqlite3_get_table(s->db,sql,&result,&row,&column,NULL) != SQLITE_OK)
    {
        printf("error msg:%s\n",sqlite3_errmsg(s->db));
        return list;
    }
    list.column = column;

    for(int i = 1; i <= row ;i++)
    {
        for(int j = 0;j < column; j++)
        {
            InsertMyString(str,result[i*column + j]);
            list.list.PushBack(&list.list,str);
        }
    }
    sqlite3_free_table(result);
    return list;
}

int searchIsExist(SQL *s,const char *sql) // 判断数据库表里是否有这个用户
{
    char **result;
    int row = 0, column = 0;
    if(sqlite3_get_table(s->db,sql,&result,&row,&column,NULL) != SQLITE_OK)
    {
        printf("error msg:%s\n",sqlite3_errmsg(s->db));
        return -1;
    }
    
    sqlite3_free_table(result);
    return row;
}

int GetTableVal(SQL *s,const char *sql, char *ptr, int *val, int pos, int Row) //获取结果集中的某个元素
{
    char **result;
    int row = 0, column = 0;
    if(sqlite3_get_table(s->db,sql,&result,&row,&column,NULL) != SQLITE_OK)
    {
        printf("error msg:%s\n", sqlite3_errmsg(s->db));
        return -1;
    }
    if(val != NULL)
    {
        *val = strtol(result[row * column + pos], NULL, 0);
    }
    printf("row = %d, column = %d\n", row, column);
    if(ptr != NULL && Row == 0)
    {
        strcpy(ptr, result[row * column + pos]);
    }
    else if(ptr != NULL && Row != 0)
    {
        strcpy(ptr, result[Row * column]);
    }
    // printf("column = %d\n", column);
    //printf("%s\n", result[row * column + 1]);
    
    // if(row == 0)
    // {
    //    sqlite3_free_table(result);
    //    return 0;
    // }
    sqlite3_free_table(result);
    return row;
}

int GetTablefd(SQL *s,const char *sql, int *val, int pos) //获取结果集中的某个元素
{
    char **result;
    int row = 0, column = 0;
    if(sqlite3_get_table(s->db,sql,&result,&row,&column,NULL) != SQLITE_OK)
    {
        printf("error msg:%s\n", sqlite3_errmsg(s->db));
        return -1;
    }
    if(val != NULL)
    {
        *val = strtol(result[pos * column], NULL, 0);
    }
    printf("row = %d, column = %d\n", row, column);
    // printf("column = %d\n", column);
    //printf("%s\n", result[row * column + 1]);
    
    // if(row == 0)
    // {
    //    sqlite3_free_table(result);
    //    return 0;
    // }
    sqlite3_free_table(result);
    return row;
}

DLlist GetTablelist(SQL *s,const char *sql) //后加
{
    DLlist list;
    
    InitDLlist(&list);

    char **result;
    int row = 0, column = 0;
    if(sqlite3_get_table(s->db,sql,&result,&row,&column,NULL) != SQLITE_OK)
    {
        printf("error msg:%s\n",sqlite3_errmsg(s->db));
        return list;
    }


    for(int i = 1; i <= row ;i++)
    {
        for(int j = 0;j < column; j++)
        {
            InsertDLlistTail(&list,result[i*column + j]);
        }
    }
    sqlite3_free_table(result);
    return list;
}

// int search1(char *n, SQL *s,const char *sql)//搜索用户名是否存在
// {
//     DLlist list;
    
//     InitDLlist(&list);

//     char **result;
//     int row = 0, column = 0;
//     if(sqlite3_get_table(s->db,sql,&result,&row,&column,NULL) != SQLITE_OK)
//     {
//         printf("error msg:%s\n",sqlite3_errmsg(s->db));
//         return -1;
//     }


//     for(int i = 1; i <= row ;i++)
//     {
//         for(int j = 0;j < column; j++)
//         {
//           InsertDLlistTail(&list,result[i*column + j]);
//         }
//     }
//     sqlite3_free_table(result);
// int count = 0;
//     struct Node *node =list.head;
//     while (node->next != NULL)
//     {
//         node = node->next;
//         if (strcmp(n, node->value) == 0)
//         {
//             count++;
//             //printf("count:%d\n", count);
//         }
//     }
//     return count;
// }

int findusername(char * n,SQL *s, const char *sql)//自己
{
    DLlist list;
    
    InitDLlist(&list);

    TIlist result = GetTableInfo(s,sql);
    
    for(int i = 0; i< result.list.len; i++)
    {
       InsertDLlistTail(&list,result.list.strlist[i].str);
    }
    int count = 0 ;
    struct Node *node = list.head;
    while(node->next != NULL)
    {
        node = node->next;
        if(strcmp(n,node->value) == 0)
        {
            count ++;
        }

    }
    return count;
}
int findpassword(char *n,SQL *s,const char *sql)//自己
{
    DLlist list;
    
    InitDLlist(&list);

    TIlist result = GetTableInfo(s,sql);
    
    for(int i = 0; i< result.list.len; i++)
    {
       char *s1 = result.list.strlist[i].str;
       char *s2 = result.list.strlist[i+1].str;
       
       strcat(s1,s2);
       InsertDLlistTail(&list,s1);
       i = i+1;
    }
    int count = 0 ;
    struct Node *node = list.head;
    while(node->next != NULL)
    {
        node = node->next;
        if(strcmp(n,node->value) == 0 )
        {
            count++;
        }
    }
    return count;
}
void FreeTableInfo(TIlist *l)
{
    l->list.ClearMyStrList(&l->list);
}

void ClearSqlite(SQL *s)
{
    sqlite3_close(s->db);
    free(s);
}