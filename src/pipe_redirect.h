#ifndef PIPE_REDIRECT_H
#define PIPE_REDIRECT_H

#include "../include/common.h"

/* Check if a command line contains a pipe operator */
int has_pipe(const char *line);

/* Check if a command line contains redirection operators */
int has_redirect(const char *line);

/* Execute a piped command chain: cmd1 | cmd2 | cmd3 */
int execute_piped(const char *line);

/* Execute a command with I/O redirection (>, >>, <) */
int execute_redirected(char *line);

/* Built-in: export VAR=value */
int msh_export(char **args);

/* Built-in: unset VAR */
int msh_unset(char **args);

/* Built-in: env - list all environment variables */
int msh_env(char **args);

/* Expand $VAR references in a string. Returns newly allocated string */
char *expand_env_vars(const char *line);

#endif /* PIPE_REDIRECT_H */
