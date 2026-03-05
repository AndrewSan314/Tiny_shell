#ifndef AI_H
#define AI_H

#include "../include/common.h"

/* AI mode state */
int ai_is_mode_enabled(void);
void ai_set_mode(int enabled);

/* Send a prompt to AI provider */
int ai_chat_line(const char *prompt);

/* Builtins */
int msh_aimode(char **args);
int msh_ai(char **args);

#endif /* AI_H */
