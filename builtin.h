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

#ifndef BUILTIN_H
#define BUILTIN_H

int builtin_fg1(const char *path_name);

int builtin_setenv(const char **argv, const char *in_fname, const char *out_fname);

int builtin_setenv1(const char **argv);

int setenv_wrapper(const char **argv, const char *in_fname, const char *out_fname);

int builtin_unsetenv(const char *path_name, const char *in_fname, const char *out_fname);

int builtin_unsetenv1(const char *path_name);

int unsetenv_wrapper(const char *path_name, const char *in_fname, const char *out_fname);

int builtin_cd(const char *path_name, const char *in_fname, const char *out_fname);

int cd_wrapper(const char *path_name, const char *in_fname, const char *out_fname);

int cd_wrapper1(const char *path_name);

#endif
