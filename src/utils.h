#ifndef UTILS_H
#define UTILS_H

#include "../include/common.h"

/* File utilities */
int msh_cat(char **args);
int msh_touch(char **args);
int msh_rm(char **args);
int msh_cp(char **args);
int msh_mv(char **args);
int msh_head(char **args);
int msh_tail(char **args);
int msh_wc(char **args);
int msh_mkdir(char **args);

/* System utilities */
int msh_whoami(char **args);
int msh_hostname(char **args);
int msh_uptime(char **args);
int msh_echo(char **args);
int msh_clear(char **args);

#endif /* UTILS_H */
