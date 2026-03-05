#ifndef AI_H
#define AI_H

#include "../include/common.h"

/*============================================================
 * AI ENGINE - Native WinHTTP integration (OpenRouter-compatible)
 * Features: conversation memory, system prompt, streaming output
 *============================================================*/

#define AI_MAX_HISTORY     10   /* Max conversation turns to remember */
#define AI_MAX_MSG_LEN     4096 /* Max length of a single message */
#define AI_SYSTEM_PROMPT_LEN 1024

/* Message role */
typedef enum {
    AI_ROLE_USER,
    AI_ROLE_MODEL
} AiRole;

/* Single conversation message */
typedef struct {
    AiRole role;
    char text[AI_MAX_MSG_LEN];
} AiMessage;

/* Conversation state */
typedef struct {
    AiMessage messages[AI_MAX_HISTORY * 2]; /* user + model pairs */
    int count;                               /* number of messages */
    char system_prompt[AI_SYSTEM_PROMPT_LEN];
} AiConversation;

/* Initialize / cleanup AI subsystem */
void ai_init(void);
void ai_cleanup(void);

/* AI mode state */
int  ai_is_mode_enabled(void);
void ai_set_mode(int enabled);

/* Conversation management */
void ai_clear_history(void);
void ai_set_system_prompt(const char *prompt);

/* Core AI interaction - sends prompt via native WinHTTP */
int ai_chat_line(const char *prompt);

/* Builtins */
int msh_aimode(char **args);
int msh_ai(char **args);

#endif /* AI_H */
