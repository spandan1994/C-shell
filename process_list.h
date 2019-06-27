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

#ifndef PRO_LIST_H
#define PRO_LIST_H

typedef struct node
{
        char p_name[20];
        pid_t p_pid;
	int status;
	int job_id;
        struct node *next;
}node;

typedef struct list
{
        node *head;
}list;

list *Createlist(void);

node *Makenode(char *name, pid_t pid, int job_id);

void pushfront(list *l, node *n);

int Search_by_name(list *l, char *name, int situ);

int Search_by_pid(list *l, pid_t pid);

void Update(list *l);

void free_list(list *l);

int change_status(list *l, int pid, int value);

#endif
