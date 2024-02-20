#include <stdio.h>
#include <json-c/json.h> // 这个就包含了json-c所有头文件
#include <string.h>
#include <errno.h>


#define	GET_BIT(x, bit)	((x & (1 << bit)) >> bit)	/* 获取第bit位 */
/* integer bit operation */
#define BITS_MASK(bit)       ((bit) < 64 ? (1LLU << (bit)) : 0LLU)
#define BITS_SET(value, bit) ((value) |= (1LLU << (bit)))
#define BITS_CLR(value, bit) ((value) &= ~(1LLU << (bit)))
#define BITS_TST(value, bit) (!!((value) & (1LLU << (bit))))

enum COMMAND_CODE
{
    LOGIN       
        = 0b0001,
    LOGOUT      
        = 0b0010,
    REGISTER 
        = 0b0100,
};

void printJsonObject(json_object *jsonObj)
{
    const char * str = json_object_to_json_string(jsonObj);
    printf("%s", str);
}


int main()
{
#if 0
    /* 新建Json对象 */
    struct json_object * jsonObj = json_object_new_object();
    struct json_object * value = json_object_new_int64(32);
    json_object_object_add(jsonObj, "age", value);

    const char * str = json_object_to_json_string(jsonObj);
    printf("%s\n", str);
#endif

#if 0
    /* 将字符串转化成json对象 */
    struct json_object * ageObj = json_tokener_parse(str);

    struct json_object * key = json_object_object_get(ageObj, "age");
    if (key == NULL)
    {
        printf("\n");
    }
    
    int64_t val = json_object_get_int64(key);
    printf("value:%ld\n", val);
#endif
    /* 释放 */
    // json_object_put();

#if 0

/* 测试 */

    
    /* 新建 */
    struct json_object * obj1 = json_object_new_array();                // 新建json_object数组对象
    /* 添加 */
    json_object_array_add(obj1, json_object_new_string("jingdong"));
    json_object_array_add(obj1, json_object_new_string("pingduoduo"));
    json_object_array_add(obj1, json_object_new_string("taobao"));

    struct json_object * obj2 = json_object_new_object();               // 新建json_object对象
    json_object_object_add(obj2, "site1", json_object_new_string("www.runoob.com"));
    json_object_object_add(obj2, "site2", json_object_new_string("m.runoob.com"));
    
    struct json_object * obj = json_object_new_object();
    json_object_object_add(obj, "name", json_object_new_string("runoob"));  // 按照键值对形式向json_object添加，值的数据类型必须是json_object
    json_object_object_add(obj, "alexa", json_object_new_int64(10000));
    json_object_object_add(obj, "shopping", obj1);
    json_object_object_add(obj, "sites", obj2);
#endif


#if 0
    const char * str = json_object_to_json_string(obj);
    printf("%s\n", str);
#else
    //printJsonObject(obj);
    //printJsonObject(obj1);
    
#endif

#if 0
    printf("删除\n");
    json_object_object_del(obj2, "site2"); // 删除指定键的键值对
    json_object_array_del_idx(obj1, 2, 1); // 删除连续的n个
    printJsonObject(obj);
#endif

//typedef enum json_type
//{
//	/* If you change this, be sure to update json_type_to_name() too */
//	json_type_null,
//	json_type_boolean,
//	json_type_double,
//	json_type_int,
//	json_type_object,
//	json_type_array,
//	json_type_string
//} json_type;

#if 0
    struct json_object * tmpObj;
    printf("获取\n");

    // obj {"alexa":10000}  取出10000
    tmpObj = json_object_object_get(obj, "alexa");          // 根据“键”取得对应的“值”
    int typeCode = json_object_get_type(tmpObj);            // 获取类型
    int ret = json_object_is_type(tmpObj, json_type_int);   // 判断类型
    int val = json_object_get_int64(tmpObj);                // 转换json_object
    
    printf("tmpObj类型码为: %d\n", typeCode);
    printf("tmpObj类型码是否是json_type_int的返回值:%d\n", ret);
    printf("tmpObj值为: %d\n", val);
    printf("tmpObj值为: ");    printJsonObject(tmpObj);    printf("\n"); /* 转换成了字符串 */

    // obj {"name":"runoob"} 取出"runoob"
    tmpObj = json_object_object_get(obj, "name");
    int strLen = json_object_get_string_len(tmpObj);            // 获取字符串长度
    char *str = (char *)malloc(sizeof(char) * (strLen + 1));
    strncpy(str, json_object_get_string(tmpObj), strLen);       // json_object_get_string获取字符串返回值常量
    printf("tmpObj:%s\n", str);

    // obj {"shopping":[...]} 取出[...] 看不懂
    tmpObj = json_object_object_get(obj, "shopping");
    
    //array_list * arrayList = json_object_get_array(tmpObj); //返回值类型array_list 头文件"json-c/arraylist.h"


#endif

#if 1
/* 测试命令 */
    /* 创建主JSON */
    struct json_object * objAll = json_object_new_object();

    /* login */
    struct json_object * obj_login_array = json_object_new_array();
    struct json_object * obj_login_name = json_object_new_string("wxr");
    struct json_object * obj_login_passwords = json_object_new_string("1231");
    
    json_object_array_add(obj_login_array, obj_login_name);
    json_object_array_add(obj_login_array, obj_login_passwords);
    json_object_object_add(objAll, "login", obj_login_array);

    /* logout */
    struct json_object * obj_logout_array = json_object_new_array();
    struct json_object * obj_logout_name = json_object_new_string("WXR");
    struct json_object * obj_logout_passwords = json_object_new_string("313213");
    
    json_object_array_add(obj_logout_array, obj_logout_name);
    json_object_array_add(obj_logout_array, obj_logout_passwords);
    json_object_object_add(objAll, "logout", obj_logout_array);

    /* command */
    struct json_object * objCommad = json_object_new_int(LOGIN | LOGOUT);
    json_object_object_add(objAll, "command", objCommad);

/* json_obj对象转string */
    const char * str = json_object_to_json_string(objAll);
    printf("str = %s\n", str);

/* string对象转json_obj */
    struct json_object * messageObj = json_tokener_parse(str);
    const char * str_ = json_object_to_json_string(messageObj);
    printf("messageObj = %s\n", str_);

/* 解释 */
    struct json_object * commandObj = json_object_object_get(messageObj, "command");
    if (commandObj == NULL)
    {
        perror("json get command error");
        return -1;
    }
    else
    {
        int cmdCode = json_object_get_int(commandObj);
        printf("cmdCode = %d\n", cmdCode);
        /* 登陆 */
        if (GET_BIT(cmdCode, 0))
        {
            printf("执行LOGIN操作!\n");
        }

        /* 登出 */
        if (GET_BIT(cmdCode, 1))
        {
            printf("执行LOGOUT操作!\n");
        }

        /* 注册 */
        if (GET_BIT(cmdCode, 2))
        {
            printf("执行REGISTER操作!\n");
        }

        //if (cmdCode & LOGOUT == LOGOUT)
        //{
        //    printf("执行LOGOUT操作!\n");
        //}

    }


#if 0
    char *str1 = "login";
    char *str2 = "register";
    char *str3 = "logout";
    
    char *cmdList[] = {str1, str2, str3};
    int cmdLen = sizeof(cmdList) / sizeof(cmdList[0]);
    printf("cmdLen = %d\n", cmdLen);
    struct json_object * cmdObj = NULL;

    for (int idx = 0; idx < cmdLen; idx++)
    {
        cmdObj = json_object_object_get(messObj, (cmdList[idx]));
        if (cmdObj != NULL)
        {
            printf("执行%s命令!\n", (cmdList[idx]));
        }
    }
#endif
    
    
    





#endif




    return 0;
}