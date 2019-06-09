#include "builtin.h"
#include <termios.h>

#ifndef EXECUTION_H
#define EXECUTION_H

typedef struct{
    int num_cmds;
    char ***arglists;
    char in_fname[20];
    char out_fname[20];
    int background;
}PIPE_LINE;

void signal_ignore(void);

void signal_default(void);

int execution(PIPE_LINE *cmd_seq, list *process_list);

int exec_wrapper(PIPE_LINE *cmd_seq, list *process_list);

int find_path(int i, char ***arglists);

#endif
