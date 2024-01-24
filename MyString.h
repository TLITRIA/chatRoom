#ifndef __MYSTRING_H_
#define __MYSTRING_H_

#include <stdbool.h>
#define  InitMyStr(str)   MyStr str;InitString(&str);
#define  InitMyStrList(strlist) MyStrList strlist;InitStringList(&strlist);
#define  InsertMyString(str,string)  InitMyStr(str);str.InsertString(&str,string);


struct MyString;
typedef struct MyString MyStr;

struct MyStringList;
typedef struct MyStringList MyStrList;

struct MyString
{
    char *str;
    int len;
    int size;
    //定义一个函数指针，作为结构体的成员变量
   bool (*InsertString)(MyStr *s,const char*string);
   bool (*InsertChar)(MyStr *s,const char c);
   MyStrList (*Split)(MyStr *s, const char *string);
   bool (*InsertStringByIndex)(MyStr *s,int index,const char* string);
   bool (*RemoveString)(MyStr *s,const char *string);
   void (*ClearString)(MyStr *s);
   int (*StringToInt)(MyStr *s);
   double (*StringToDouble)(MyStr *s);
   bool (*IsStringEqual)(MyStr *s1,MyStr *s2);
   MyStr (*CopyString)(MyStr *s);
   bool (*ReSize)(MyStr *s,long size);
};

void InitString(MyStr *s);


struct MyStringList
{
    MyStr *strlist;
    int len;
    int size;

    bool (*PushBack)(MyStrList *list,MyStr s);
    bool (*RemoveMyString)(MyStrList*list,MyStr s);
    void (*DisplayList)(MyStrList *list);
    void (*ClearMyStrList)(MyStrList *list);
};
void InitStringList(MyStrList *list);

#endif