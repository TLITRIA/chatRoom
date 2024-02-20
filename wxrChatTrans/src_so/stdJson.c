#include <stdio.h>
#include <json-c/json.h> // 这个就包含了json-c所有头文件
#include <string.h>
#include <errno.h>

#include "common.h"
#include "stdJson.h"


/**json里面int型不要赋值0,0作为json_object_get_int的报错码使用
 * 
 * 
 * 
*/

/* 静态函数 */
/* 打印 */
void printJsonObject(json_object *jsonObj)
{
    printf("%s", json_object_to_json_string(jsonObj));
}


//static int judge_overflow(char * string);

/*
command:cmd_code
option:  {
    [length]:...
    [name]:...
}
*/

/* 发送前告知文件信息：文件大小、文件名称 */
char *json_pre_file(int length, char *name)
{
    if (length <= 0)
    {
        printf("%s:invalid args!\n", __func__);
        return NULL; //todo
    }

    /* ===创建主JSON objAll=== */
    struct json_object * objAll = json_object_new_object();

    /* command */
    json_object_object_add(objAll, "command", json_object_new_int(FILE_PRE_SEND));

    /* option:length name */ 
    struct json_object * optionObj = json_object_new_object();
    json_object_object_add(optionObj, "length", json_object_new_int(length));               /* length */
    json_object_object_add(optionObj, "name", json_object_new_string((const char *)name));  /* name */

    json_object_object_add(objAll, "option", optionObj);

    /* ===json_obj对象转string=== */
    const char * const_str = json_object_to_json_string(objAll);
    int message_length = (int)strlen(const_str);

    if (message_length > MAX_JSON_LENGTH)
    {
        printf("%s:message too long!\n", __func__);
        return NULL; // todo
    }
    
    char *retstr = (char *)malloc(sizeof(char) * strlen(const_str));
    memset(retstr, 0, sizeof(char) * strlen(const_str));
    strncpy(retstr, const_str, sizeof(char) * strlen(const_str));

    return retstr;
}

/*
command:cmd_code
option:  {
    [if_ready]:1/-1
    [pages]:...
    [page_size]:...
    [lastPageBytes]:...
}
*/

/* 发送文件前的确认 */
char *json_pre_check(int if_ready, int pages, int page_size, int lastPageBytes)
{
    if (if_ready == 0 || lastPageBytes == 0 || pages == 0)
    {
        printf("%s:invalid args!\n", __func__);
        return NULL; // todo
    }
    
    /* 创建主JSON */
    struct json_object * objAll = json_object_new_object();

    
    /* command */
    json_object_object_add(objAll, "command", json_object_new_int(FILE_PRE_CHECK));

    /* option */
    struct json_object * optionObj = json_object_new_object();
    json_object_object_add(optionObj, "if_ready", json_object_new_int(if_ready));           /* if_ready */
    json_object_object_add(optionObj, "pages", json_object_new_int(pages));                 /* pages */
    json_object_object_add(optionObj, "page_size", json_object_new_int(page_size));                 /* page_size */
    json_object_object_add(optionObj, "lastPageBytes", json_object_new_int(lastPageBytes)); /* lastPageBytes */

    json_object_object_add(objAll, "option", optionObj);


    /* json_obj对象转string */
    const char * const_str = json_object_to_json_string(objAll);
    int message_length = (int)strlen(const_str);

    if (message_length > MAX_JSON_LENGTH)
    {
        printf("%s:message too long!\n", __func__);
        return NULL; //todo
    }
    
    char *retstr = (char *)malloc(sizeof(char) * strlen(const_str));
    memset(retstr, 0, sizeof(char) * strlen(const_str));
    strncpy(retstr, const_str, sizeof(char) * strlen(const_str));

    return retstr;
}

/*
command:FILE_SEND
option:  {
    [page]: 1 2 3 4... 当前传输的页码，不能取0
    [context]:...
}
*/

/* 发送过程中的数据 */
int json_send_page(int page, char *context, char **sendBuffer)
{
    if (page <= 0 || context == NULL)
    {
        printf("%s:invalid args!\n", __func__);
        return -1; // todo
    }

    /* 创建主JSON */
    struct json_object * objAll = json_object_new_object();

    /* command */
    json_object_object_add(objAll, "command", json_object_new_int(FILE_MAIN_SEND));

    /* option */ 
    struct json_object * optionObj = json_object_new_object();
    json_object_object_add(optionObj, "page", json_object_new_int(page));        /* page */
    json_object_object_add(optionObj, "context", json_object_new_string((const char *)context));  /* context */

    json_object_object_add(objAll, "option", optionObj);

    /* json_obj对象转string */
    const char * const_str = json_object_to_json_string(objAll);
    int message_length = (int)strlen(const_str);

    if (message_length > MAX_JSON_LENGTH)
    {
        printf("%s:message too long!\n", __func__);
        return -1; //todo
    }
    
    char *retstr = (char *)malloc(sizeof(char) * strlen(const_str) + 1);
    memset(retstr, 0, sizeof(char) * strlen(const_str) + 1);
    strncpy(retstr, const_str, sizeof(char) * strlen(const_str)); // 解决了消息传出后有乱码的问题，其他函数也要检查一下

    *sendBuffer = retstr;
    return ON_SUCCESS;
}



/*
command:cmd_code
option:  {
    [recv_status]:1/-1 接收正常，不需要重发/接收不正常需要重发 0 作为报错码不能去
    [page]:...
}
*/


/* 确认接收情况,  */
char *json_send_check(int recv_status, int page)
{
    if (recv_status == 0 || page == 0)
    {
        printf("%s:invalid args!\n", __func__);
        return NULL; //todo
    }
    /* 创建主JSON */
    struct json_object * objAll = json_object_new_object();

    /* command */
    json_object_object_add(objAll, "command", json_object_new_int(FILE_MAIN_CHECK));/* command */


    /* option */ 
    struct json_object * optionObj = json_object_new_object();
    json_object_object_add(optionObj, "recv_status", json_object_new_int(recv_status)); /* recv_status */
    json_object_object_add(optionObj, "page", json_object_new_int(page));        /* page */

    /* option加入ALL */
    json_object_object_add(objAll, "option", optionObj);


    /* json_obj对象转string */
    const char * const_str = json_object_to_json_string(objAll);
    int message_length = (int)strlen(const_str);

    if (message_length > MAX_JSON_LENGTH)
    {
        printf("%s:message too long!\n", __func__);
        return NULL; //todo
    }
    
    char *retstr = (char *)malloc(sizeof(char) * strlen(const_str) + 1);
    memset(retstr, 0, sizeof(char) * strlen(const_str) + 1);
    strncpy(retstr, const_str, sizeof(char) * strlen(const_str)); // 解决了消息传出后有乱码的问题，其他函数也要检查一下

    return retstr;
}




/* option提取int */
int json_parse_int(struct json_object * optionObj, char *key_str)
{
    int error = -1;
    struct json_object * intObj = json_object_object_get(optionObj, key_str);
    if (intObj == NULL)
    {
        printf("未找到键:\"%s\"\n", key_str);
        printJsonObject(optionObj);
        return error;
    }

    // todo判断一下类型是不是jsonint

    int checkLength = json_object_get_int(intObj); // todo 失败的返回值是0，所以正常不能取值0，以上全部要查
    if (checkLength == 0)
    {
        printf("int读取错误\n");
        return error;
    }
    return checkLength;
}


/* option提取str */
char *json_parse_str(struct json_object * optionObj, char *key_str)
{
    char *error = NULL;
    struct json_object * strObj = json_object_object_get(optionObj, key_str);
    if (strObj == NULL)
    {
        printf("未找到键:\"%s\"\n", key_str);
        printJsonObject(optionObj);
        return error;
    }

    const char * const_str = json_object_get_string(strObj);
    if (const_str == NULL)
    {
        printf("str读取错误\n");
        return error;
    }

    int const_len = strlen(const_str);
    char * retstr = (char *)malloc(sizeof(char) * const_len);
    if (retstr == NULL)
    {
        printf("malloc error\n");
        return error;
    }
    memset(retstr, 0, sizeof(char) * const_len);

    strncpy(retstr, const_str, sizeof(char) * const_len);
    return retstr;
}