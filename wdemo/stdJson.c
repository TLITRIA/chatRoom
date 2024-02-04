#include <stdio.h>
#include <json-c/json.h> // 这个就包含了json-c所有头文件
#include <string.h>
#include <errno.h>

#include "common.h"
#include "stdJson.h"


/**json里面int型不要赋值0,0是json_object_get_int的报错码
 * 
 * 
 * 
*/

/* 静态函数 */
/* 打印 */
void printJsonObject(json_object *jsonObj)
{
    const char * str = json_object_to_json_string(jsonObj);
    printf("%s", str);
}


/*
command:cmd_code
option:  {
    [length]:...
    [name]:...
}
*/

/* 发送文件前的准备 */
char *json_pre_file(int length, char *name)
{
    if (length <= 0)
    {
        printf("invalid json_pre_file!\n");
        return NULL;
    }
    int cmd_code = FILE_PRE;

    /* 创建主JSON */
    struct json_object * objAll = json_object_new_object();

    /* command */
    struct json_object * objCommad = json_object_new_int(cmd_code);
    json_object_object_add(objAll, "command", objCommad);


    /* option */ 
    struct json_object * optionObj = json_object_new_object();
    /* length */
    struct json_object * obj_length = json_object_new_int(length);
    json_object_object_add(optionObj, "length", obj_length);

    /* 文件名 */
    struct json_object * obj_name = json_object_new_string((const char *)name);
    json_object_object_add(optionObj, "name", obj_name);

    json_object_object_add(objAll, "option", optionObj);


    /* json_obj对象转string */
    const char * const_str = json_object_to_json_string(objAll);

    int message_length = (int)strlen(const_str);
    //printf("头消息的长度:%d\n%s\n", message_length, const_str);

    if (message_length > MAX_NORMAL_MESSAGE)
    {
        printf("message too long1\n");
        return NULL;
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
    [lastPageBytes]:...
}
*/

/* 发送文件前的确认 */
char *json_pre_check(int if_ready, int pages, int lastPageBytes)
{
    if (if_ready == 0 || lastPageBytes == 0 || pages == 0)
    {
        printf("invalid json_pre_check!\n");
        return NULL;
    }
    
    /* 创建主JSON */
    struct json_object * objAll = json_object_new_object();

    
    /* command */
    struct json_object * objCommad = json_object_new_int(FILE_PRE_SENDBACK);
    json_object_object_add(objAll, "command", objCommad);


    /* option */
    struct json_object * optionObj = json_object_new_object();
    /* if_ready */
    struct json_object * obj_id_ready = json_object_new_int(if_ready);
    json_object_object_add(optionObj, "if_ready", obj_id_ready);

    /* pages */
    struct json_object * obj_pages = json_object_new_int(pages);
    json_object_object_add(optionObj, "pages", obj_pages);

    /* lastPageBytes */
    struct json_object * obj_excess = json_object_new_int(lastPageBytes);
    json_object_object_add(optionObj, "lastPageBytes", obj_excess);

    json_object_object_add(objAll, "option", optionObj);


    /* json_obj对象转string */
    const char * const_str = json_object_to_json_string(objAll);

    int message_length = (int)strlen(const_str);
    //printf("打包数据 %s\n", const_str);

    if (message_length > MAX_NORMAL_MESSAGE)
    {
        printf("message too long2\n");
        return NULL;
    }
    
    char *retstr = (char *)malloc(sizeof(char) * strlen(const_str));
    memset(retstr, 0, sizeof(char) * strlen(const_str));
    strncpy(retstr, const_str, sizeof(char) * strlen(const_str));

    return retstr;

}

/*
command:FILE_SEND
option:  {
    [page]:...
    [context]:...
}
*/

/* 发送过程中的数据 */
char *json_send_file(int page, char *context)
{
    if (page <= 0)
    {
        printf("invalid json_send_file!\n");
        return NULL;
    }

    /* 创建主JSON */
    struct json_object * objAll = json_object_new_object();

    /* command */
    struct json_object * objCommad = json_object_new_int(FILE_SEND);
    json_object_object_add(objAll, "command", objCommad);


    /* option */ 
    struct json_object * optionObj = json_object_new_object();
    /* page */
    struct json_object * obj_page = json_object_new_int(page);
    json_object_object_add(optionObj, "page", obj_page);

    /* context */
    struct json_object * obj_context = json_object_new_string((const char *)context);
    json_object_object_add(optionObj, "context", obj_context);

    json_object_object_add(objAll, "option", optionObj);


    /* json_obj对象转string */
    const char * const_str = json_object_to_json_string(objAll);

    int message_length = (int)strlen(const_str);
    //printf("消息的长度:%d\n%s\n\n", message_length, const_str);

    if (message_length > MAX_NORMAL_MESSAGE)
    {
        printf("message too long3\n");
        return NULL;
    }
    
    char *retstr = (char *)malloc(sizeof(char) * strlen(const_str) + 1);
    memset(retstr, 0, sizeof(char) * strlen(const_str) + 1);
    strncpy(retstr, const_str, sizeof(char) * strlen(const_str)); // 解决了消息传出后有乱码的问题，其他函数也要检查一下

    return retstr;
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
        printf("invalid json_send_check!\n");
        return NULL;
    }
    
    
    /* 创建主JSON */
    struct json_object * objAll = json_object_new_object();

    /* command */
    struct json_object * objCommad = json_object_new_int(FILE_RECV_STATUS);
    json_object_object_add(objAll, "command", objCommad);


    /* option */ 
    struct json_object * optionObj = json_object_new_object();
    /* recv_status */
    struct json_object * obj_recv_status = json_object_new_int(recv_status);
    json_object_object_add(optionObj, "recv_status", obj_recv_status);
    /* page */
    struct json_object * obj_page = json_object_new_int(page);
    json_object_object_add(optionObj, "page", obj_page);

    /* option加入ALL */
    json_object_object_add(objAll, "option", optionObj);


    /* json_obj对象转string */
    const char * const_str = json_object_to_json_string(objAll);

    int message_length = (int)strlen(const_str);
    //printf("消息的长度:%d\n%s\n\n", message_length, const_str);

    if (message_length > MAX_NORMAL_MESSAGE)
    {
        printf("message too long4\n");
        return NULL;
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