#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#ifndef VARIABLE_LIST_H
#define VARIABLE_LIST_H

typedef struct NODE
{
        char *variable[40];
        struct NODE *next;
}NODE;

typedef struct LIST
{
        NODE *head;
}LIST;

LIST *CreateLIST(void);

NODE *MakeNODE(char **name);

void pushFRONT(LIST *l, NODE *n);

NODE *Search_by_var(LIST *l, char *name);

int Search_and_replace(LIST *l, char **name);

void Free_LIST(LIST *l);

#endif
