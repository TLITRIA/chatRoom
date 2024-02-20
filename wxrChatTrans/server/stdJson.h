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
#define MAX_JSON_LENGTH 1024 // 正常消息的最大长度，也是接收区缓存大小
#define MAX_PAGES_SIZE 128 // 每页消息最大的长度

/* 这个要保证服务端客户端文件夹下一致 */
enum CMD_CODE
{
    DEFAULTCODE = -1,
    FILE_PRE_SEND,  // 传输前的文件信息
    FILE_PRE_CHECK, // 对文件信息的回应
    FILE_MAIN_SEND, // 文件主体，分页
    FILE_MAIN_CHECK,// 对文件主体的回应
};

/* json规则 */
/*
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

/* 打印测试 */
void printJsonObject(json_object *jsonObj);

/* =====封装消息===== */

/* 登陆注册客户端打包 */




/* ===文件传输=== */
/* 发送文件前的准备 */
char *json_pre_file(int length, char *name);

/* 发送文件前的确认 */
char *json_pre_check(int if_ready, int pages, int page_size, int lastPageBytes);

/* 发送过程中的数据 */
int json_send_page(int page, char *context, char **sendBuffer);

/* 确认接收情况 */
char *json_send_check(int recv_status, int page);




/* =====以上为封装消息的函数===== */





/* 解释 */
/* option提取int */
int json_parse_int(struct json_object * optionObj, char *key_str);


/* option提取str */
char *json_parse_str(struct json_object * optionObj, char *key_str);






#endif // __STD_JSON_H_