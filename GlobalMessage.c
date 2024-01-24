#include "GlobalMessage.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
CInfo * CreateInfo(const char *name,int sock)
{
    CInfo *c = (CInfo *)malloc(sizeof(CInfo));
    if(c == NULL)
    {
        perror("info malloc");
        return NULL;
    }
    strcpy(c->Name,name);
    c->sock = sock;
    return c;
}

void ClearInfo(CInfo *c)
{
    free(c);
}

bool IsNameSame(void *ptr1,void *ptr2)
{
    char *c1 = (char*)ptr1;
    CInfo *c2 = (CInfo *)ptr2;
      if(strcmp(c1,c2->Name) == 0)
      {
               return true;
      }
      return false;

}

CSignup *CreateSignup(const char *name, const char *password)
{
    CSignup *c = (CSignup *)malloc(sizeof(CSignup));
    if(c == NULL)
    {
        perror("signup  malloc");
        return NULL;
    }
    strcpy(c->name,name);
    strcpy(c->password,password);
    return c;

}

RT *CreateRoot(const char *name)
{
    RT *r = (RT *)malloc(sizeof(RT));
    if(r == NULL)
    {
        perror("root malloc");
        return NULL;
    }
    strcpy(r->rootname,name);
    
    return r;
}