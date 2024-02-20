#ifndef __COMMON_H_
#define __COMMON_H_

/*
#ifndef 
#define 
#endif // 
*/

/* 获取第bit位 */
#define	GET_BIT(x, bit)	((x & (1 << bit)) >> bit)	

/* integer bit operation */
#define BITS_MASK(bit)       ((bit) < 64 ? (1LLU << (bit)) : 0LLU)
#define BITS_SET(value, bit) ((value) |= (1LLU << (bit)))
#define BITS_CLR(value, bit) ((value) &= ~(1LLU << (bit)))
#define BITS_TST(value, bit) (!!((value) & (1LLU << (bit))))


/* ====函数返回码==== */
enum RETURN_CODE
{
    NOT_FIND = -5,
    INVILID_ACCESS,
    NULL_PTR,
    MALLOC_ERROR,
    DEFAULT_ERROR = -1,
    ON_SUCCESS,
};

/* ====宏函数==== */
#define JUDGE_IFNULL(ptr)   \
if (NULL == ptr)            \
    return NULL_PTR;        \

#define JUDGE_MALLOC(ptr)   \
if (NULL == ptr)            \
    return MALLOC_ERROR;    \

#define JUDGE_IFNULL_RETURN_NULL(ptr)   \
if (NULL == ptr)                        \
    return NULL;                        \

#define FREE(ptr)   \
if (NULL == ptr)    \
{                   \
    free(ptr);      \
    ptr = NULL;     \
}                   \


/* log变色函数 */
char *logChangeGreen(char * str);



#endif // __COMMON_H_
