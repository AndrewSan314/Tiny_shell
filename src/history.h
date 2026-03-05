#ifndef HISTORY_H
#define HISTORY_H

#include "../include/common.h"

#define HISTORY_MAX     100
#define HISTORY_FILE    ".msh_history"

/* Initialize history system */
void history_init(void);

/* Add a command to history */
void history_add(const char *cmd);

/* Get command at index (0 = most recent) */
const char *history_get(int index);

/* Get total number of entries */
int history_count(void);

/* Navigate history: -1 = older, +1 = newer. Returns command string or NULL */
const char *history_navigate(int direction);

/* Reset navigation pointer */
void history_reset_nav(void);

/* Save history to file */
void history_save(void);

/* Load history from file */
void history_load(void);

/* Built-in: display history list */
int msh_history(char **args);

#endif /* HISTORY_H */
