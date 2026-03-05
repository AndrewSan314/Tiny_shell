#ifndef CONFIG_H
#define CONFIG_H

#include "../include/common.h"

/* Load configuration from ~/.mshrc */
void config_load(void);

/* Source a file (execute lines as commands) */
int msh_source(char **args);

#endif /* CONFIG_H */
