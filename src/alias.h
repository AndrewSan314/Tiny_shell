#ifndef ALIAS_H
#define ALIAS_H

#include "../include/common.h"

#define MAX_ALIASES     50
#define ALIAS_FILE      ".msh_aliases"

/* Initialize alias system */
void alias_init(void);

/* Add or update an alias: alias name=command */
void alias_set(const char *name, const char *command);

/* Remove an alias */
void alias_remove(const char *name);

/* Look up an alias. Returns the command or NULL */
const char *alias_get(const char *name);

/* Save aliases to file */
void alias_save(void);

/* Load aliases from file */
void alias_load(void);

/* Built-in: manage aliases */
int msh_alias(char **args);

/* Built-in: remove alias */
int msh_unalias(char **args);

/* Expand aliases in a command line. Returns newly allocated string */
char *alias_expand(const char *line);

#endif /* ALIAS_H */
