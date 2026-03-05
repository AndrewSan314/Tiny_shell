#ifndef CORE_H
#define CORE_H

#include "../include/common.h"

void msh_loop(void);
char *msh_read_line(void);
char **msh_split_line(char *line);
int msh_execute(char **args);
int msh_execute_line(const char *raw_line);

#endif
