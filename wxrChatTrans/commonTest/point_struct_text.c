#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct STU
{
    char *name;
    int age;
} STU;

void modify(STU *arg)
{
    arg->age++;
    arg->name = (char *)malloc(6);
    memset(arg->name, 0, 6);
    strncpy(arg->name, "jerry", 5);
}


int main()
{

    STU * stu1 = (STU *)malloc(sizeof(STU));
    stu1->age = 10;
    stu1->name = (char *)malloc(5);
    memset(stu1->name, 0, 5);
    strncpy(stu1->name, "tom", 4);

    printf("age:%d, name:%s\n", stu1->age, stu1->name);
    
    modify(stu1);
    printf("age:%d, name:%s\n", stu1->age, stu1->name);
    


    return 0;
}