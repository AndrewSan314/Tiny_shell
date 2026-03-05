#ifndef BUILTINS_H
#define BUILTINS_H

#include "../include/common.h"

extern char *builtin_str[];
extern int (*builtin_func[])(char **);

int msh_num_builtins(void);

int msh_cd(char **args);
int msh_pwd(char **args);
int msh_dir(char **args);

int msh_datetime(char **args);
int msh_cls(char **args);
int msh_help(char **args);
int msh_exit(char **args);

int msh_path(char **args);
int msh_addpath(char **args);

int msh_systeminfo(char **args);
int msh_grep(char **args);
int msh_search(char **args);
int msh_diff(char **args);
int msh_demo(char **args);

#endif
