#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "MyString.h"

// 函数声明
bool InsertString(MyStr *s, const char *string);
bool InsertStringByIndex(MyStr *s, int index, const char *string);
bool RemoveString(MyStr *s, const char *string);
void ClearString(MyStr *s);
int StringToInt(MyStr *s);
double StringToDouble(MyStr *s);
bool IsStringEqual(MyStr *s1, MyStr *s2);
MyStr CopyString(MyStr *s);
MyStrList Split(MyStr *s, const char *string);
bool ReSize(MyStr *s, long size);
bool InsertChar(MyStr *s, const char c);

bool PushBack(MyStrList *list, MyStr s);
bool RemoveMyString(MyStrList *list, MyStr s);
void DisplayList(MyStrList *list);
void ClearMyStrList(MyStrList *list);

void InitString(MyStr *s)
{
    s->str = NULL;
    s->len = 0;
    s->size = 0;
    // 将一个函数指针赋值指向一个函数
    s->InsertString = InsertString;
    s->Split = Split;
    s->ClearString = ClearString;
    s->CopyString = CopyString;
    s->InsertStringByIndex = InsertStringByIndex;
    s->IsStringEqual = IsStringEqual;
    s->RemoveString = RemoveString;
    s->StringToDouble = StringToDouble;
    s->StringToInt = StringToInt;
    s->ReSize = ReSize;
    s->InsertChar = InsertChar;
}

bool InsertString(MyStr *s, const char *string)
{
    if (s->str == NULL)
    {
        s->len = strlen(string);
        s->size = s->len * 2;
        s->str = (char *)malloc(s->size);
        if (s->str == NULL)
        {
            printf("InsertString malloc error!\n");
            s->len = 0;
            s->size = 0;
            return false;
        }
        strcpy(s->str, string);
    }
    else
    {
        int newLen = s->len + strlen(string);
        if (newLen >= s->size)
        {
            s->size = newLen * 2;
            char *newStr = (char *)malloc(s->size);
            if (newStr == NULL)
            {
                printf("InsertString malloc newStr error!\n");
                s->size = s->size / 2;
                return false;
            }
            strcpy(newStr, s->str);
            strcat(newStr, string);
            free(s->str);
            s->str = newStr;
            s->len = newLen;
        }
        else
        {
            s->len = newLen;
            strcat(s->str, string);
        }
    }
    return true;
}

bool InsertStringByIndex(MyStr *s, int index, const char *string)
{
    if (s->str == NULL)
    {
        printf("string is empty!\n");
        return false;
    }
    else if (index < 0 || index >= s->len)
    {
        printf("InsertStringByIndex invalid palce!\n");
        return false;
    }
    else
    {
        char tempStr[1024] = {0};
        strncpy(tempStr, s->str, index);
        strcat(tempStr, string);
        strcat(tempStr, s->str + index);
        int newLen = strlen(tempStr);
        if (newLen >= s->size)
        {
            s->size = newLen * 2;
            char *newStr = (char *)malloc(s->size);
            if (newStr == NULL)
            {
                printf("InsertStringByIndex malloc newStr error!\n");
                s->size = s->size / 2;
                return false;
            }
            strcpy(newStr, tempStr);
            free(s->str);
            s->str = newStr;
            s->len = newLen;
        }
        else
        {
            strcpy(s->str, tempStr);
            s->len = newLen;
        }
    }
    return true;
}

bool RemoveString(MyStr *s, const char *string)
{
    char *head = strstr(s->str, string);
    if (head == NULL)
    {
        printf("RemoveString cannot find string : %s", string);
        return false;
    }
    else
    {
        char *removeHead = head + strlen(string);
        while (*removeHead != '\0')
        {
            *head++ = *removeHead++;
        }
        *head = '\0';
    }
    return true;
}

void ClearString(MyStr *s)
{
    free(s->str);
    s->str = NULL;
    s->size = 0;
    s->len = 0;
}

int StringToInt(MyStr *s)
{
    return atoi(s->str);
}

double StringToDouble(MyStr *s)
{
    return atof(s->str);
}

bool IsStringEqual(MyStr *s1, MyStr *s2)
{
    if (strcmp(s1->str, s2->str) == 0)
    {
        return true;
    }
    return false;
}

MyStr CopyString(MyStr *s)
{
    if (s->str == NULL)
    {
        printf("copy src string is NULL!\n");
        InitMyStr(str);
        return str;
    }
    InsertMyString(str, s->str);
    return str;
}

MyStrList Split(MyStr *s, const char *string)
{
    // 初始化字符串数组
    InitMyStrList(strlist);
    // 将要切割的字符串进行拷贝
    // 因为strtok会修改源字符串
    MyStr s_bak = CopyString(s);
    // 拷贝失败，直接返回空结构体，不进行切割处理
    if (s_bak.str == NULL)
    {
        printf("split copy str error!\n");
        InitMyStrList(strlist) return strlist;
    }
    // 切割备份字符串
    char *str = strtok(s_bak.str, string);
    // 循环切割，直到切割结束位置
    while (str != NULL)
    {
        // 切割获取的字符串尾插到字符串数组中
        InsertMyString(newStr, str);
        PushBack(&strlist, newStr);
        str = strtok(NULL, string);
    }
    // 清空字符串备份
    ClearString(&s_bak);
    // 返回字符串数组
    return strlist;
}

bool ReSize(MyStr *s, long size)
{
    char *newPtr = realloc(s->str, size);
    if (newPtr == NULL)
    {
        printf("realloc error!\n");
        return false;
    }
    else
    {
        s->size = size;
        s->str = newPtr;
        return true;
    }
}

bool InsertChar(MyStr *s, const char c)
{
    if (s->len >= s->size)
    {
        if (s->size == 0)
            s->size = 1;
        if (s->ReSize(s, s->size * 2) == false)
        {
            return false;
        }
    }
    s->str[s->len++] = c;
    s->str[s->len] = '\0';
    return true;
}

void InitStringList(MyStrList *list)
{
    list->strlist = NULL;
    list->len = 0;
    list->size = 0;

    list->ClearMyStrList = ClearMyStrList;
    list->DisplayList = DisplayList;
    list->RemoveMyString = RemoveMyString;
    list->PushBack = PushBack;
}

bool PushBack(MyStrList *list, MyStr s)
{
    if (list->strlist == NULL)
    {
        list->size = 10;
        list->strlist = (MyStr *)malloc(list->size * sizeof(MyStr));
        if (list->strlist == NULL)
        {
            printf("PushBack malloc error!\n");
            list->size = 0;
            return false;
        }
        list->strlist[0] = s;
        list->len++;
    }
    else
    {
        if (list->len >= list->size)
        {
            list->size *= 2;
            MyStr *newMyStr = (MyStr *)malloc(sizeof(MyStr) * list->size);
            if (newMyStr == NULL)
            {
                printf("PushBack malloc error!\n");
                list->size /= 2;
                return false;
            }
            for (int i = 0; i < list->len; i++)
            {
                newMyStr[i] = list->strlist[i];
            }
            free(list->strlist);
            list->strlist = newMyStr;
            list->strlist[list->len] = s;
            list->len++;
        }
        else
        {
            list->strlist[list->len] = s;
            list->len++;
        }
    }
    return true;
}

void RemoveMyStringByIndex(MyStrList *list, int index)
{
    for (int i = index; i < list->len - 1; i++)
    {
        list->strlist[i] = list->strlist[i + 1];
    }
    list->len--;
}

bool RemoveMyString(MyStrList *list, MyStr s)
{
    for (int i = 0; i < list->len; i++)
    {
        if (IsStringEqual(&list->strlist[i], &s) == true)
        {
            RemoveMyStringByIndex(list, i);
            i--;
        }
    }
}

void DisplayList(MyStrList *list)
{
    for (int i = 0; i < list->len; i++)
    {
        printf("%s\n", list->strlist[i].str);
    }
}

void ClearMyStrList(MyStrList *list)
{
    for (int i = 0; i < list->len; i++)
    {
        ClearString(&list->strlist[i]);
    }
    free(list->strlist);
    InitStringList(list);
}