#ifndef __STD_JSON_H_
#define __STD_JSON_H_

/**json相关的功能有：
 * 服务端打包消息发送给客户端
 * 服务端解释客户端发来的消息
 * 客户端打包消息发送给服务端
 * 客户端解释服务端发来的消息
 * 
 * 所有消息都挂载在一个键上
 * 
 * 传输的信息分为简单信息和超长信息
 * 简单信息不超过256个字节，可以携带简单命令，也可以为超长信息导引。
 * 
 * 目前只实现简单命令的封装、传输、解释
*/
#define MAX_HEAD_MESSAGE 64 // 传输头消息的最长长度，要比客户端、服务端接受区默认缓冲小
#define MAX_NORMAL_MESSAGE 1024 // 正常消息的最大长度，也是接收区缓存大小
#define MAX_PAGES_SIZE 128 // 每页消息最大的长度

enum CMD_CODE
{
    DEFAULTCODE = -1,
    LOGIN = 0,
    LOGOUT,
    REGISTER,
    FILE_PRE,               // 文件传输前的消息
    FILE_PRE_SENDBACK,           // 对头消息的回应
    FILE_SEND,         // 消息主体 循环
    FILE_RECV_STATUS,          // 发回接收的状态 等待下一次 循环
    FILE_END_SEND,
    FILE_END_CONFIRM,
};

/* json规则 */
/*
加括号的是根据需要添加的，其他就是必须要有的
{
    command:cmd_code
    option:  {
        [text]:...
        [name]:...
        [password]:...
        [length]:...
        [pages]:...
        [context]:...
        ...
    }
}
头消息就是
*/

/* 打印 */
void printJsonObject(json_object *jsonObj);

/* 包装 */
/* 发送文件前的准备 */
char *json_pre_file(int length, char *name);

/* 发送文件前的确认 */
char *json_pre_check(int if_ready, int pages, int lastPageBytes);

/* 发送过程中的数据 */
char *json_send_file(int page, char *context);

/* 确认接收情况 */
char *json_send_check(int recv_status, int page);







/* 解释 */
/* option提取int */
int json_parse_int(struct json_object * optionObj, char *key_str);


/* option提取str */
char *json_parse_str(struct json_object * optionObj, char *key_str);






#endif // __STD_JSON_H_