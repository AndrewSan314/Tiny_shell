#include "core.h"
#include "alias.h"
#include "ai.h"
#include "builtins.h"
#include "colors.h"
#include "config.h"
#include "hacker.h"
#include "history.h"
#include "launcher.h"
#include "pipe_redirect.h"
#include "process_manager.h"
#include "readline.h"
#include "utils.h"
#include <string.h>

static int is_ai_control_command(const char *line) {
  char cmd[32] = {0};

  if (!line)
    return 0;

  while (*line == ' ' || *line == '\t')
    line++;

  if (sscanf(line, "%31s", cmd) != 1)
    return 0;

  return (_stricmp(cmd, "ai") == 0 || _stricmp(cmd, "aimode") == 0);
}

static int is_ai_env_command(const char *line) {
  char cmd[32] = {0};

  if (!line)
    return 0;

  while (*line == ' ' || *line == '\t')
    line++;

  if (sscanf(line, "%31s", cmd) != 1)
    return 0;

  if (_stricmp(cmd, "export") != 0 && _stricmp(cmd, "unset") != 0)
    return 0;

  if (strstr(line, "OPENROUTER_API_KEY") || strstr(line, "GEMINI_API_KEY") ||
      strstr(line, "GOOGLE_API_KEY") ||
      strstr(line, "MSH_AI_MODEL") || strstr(line, "MSH_AI_URL") ||
      strstr(line, "MSH_AI_DELAY_MS"))
    return 1;

  return 0;
}


char *msh_read_line(void) { return msh_readline(); }

char **msh_split_line(char *line) {
  int bufsize = MSH_TOK_BUFSIZE;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    print_error("Memory allocation failed");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, MSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position++] = token;

    if (position >= bufsize) {
      bufsize += MSH_TOK_BUFSIZE;
      char **new_tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!new_tokens) {
        free(tokens);
        print_error("Memory allocation failed");
        exit(EXIT_FAILURE);
      }
      tokens = new_tokens;
    }

    token = strtok(NULL, MSH_TOK_DELIM);
  }

  tokens[position] = NULL;
  return tokens;
}

/* Expand !! and !n history shortcuts */
static char *expand_history_cmd(char *line) {
  if (line[0] != '!')
    return line;

  const char *expanded = NULL;

  if (strcmp(line, "!!") == 0) {
    int count = history_count();
    if (count > 0) {
      expanded = history_get(count - 1);
    }
  } else if (line[0] == '!' && line[1] >= '0' && line[1] <= '9') {
    int n = atoi(line + 1);
    int count = history_count();
    if (n > 0 && n <= count) {
      expanded = history_get(n - 1);
    }
  }

  if (expanded) {
    char *newLine = malloc(strlen(expanded) + 1);
    if (newLine) {
      strcpy(newLine, expanded);
      set_color(CLR_MUTED);
      printf("  -> %s\n", newLine);
      reset_color();
      free(line);
      return newLine;
    }
  }

  return line;
}

int msh_execute(char **args) {
  if (args[0] == NULL) {
    return MSH_CONTINUE;
  }

  /* Check for shell-specific commands */
  if (_stricmp(args[0], "history") == 0)
    return msh_history(args);
  if (_stricmp(args[0], "alias") == 0)
    return msh_alias(args);
  if (_stricmp(args[0], "unalias") == 0)
    return msh_unalias(args);
  if (_stricmp(args[0], "export") == 0)
    return msh_export(args);
  if (_stricmp(args[0], "unset") == 0)
    return msh_unset(args);
  if (_stricmp(args[0], "env") == 0)
    return msh_env(args);
  if (_stricmp(args[0], "source") == 0 || strcmp(args[0], ".") == 0)
    return msh_source(args);

  /* Check for utility commands */
  if (_stricmp(args[0], "cat") == 0)
    return msh_cat(args);
  if (_stricmp(args[0], "touch") == 0)
    return msh_touch(args);
  if (_stricmp(args[0], "rm") == 0)
    return msh_rm(args);
  if (_stricmp(args[0], "cp") == 0)
    return msh_cp(args);
  if (_stricmp(args[0], "mv") == 0)
    return msh_mv(args);
  if (_stricmp(args[0], "head") == 0)
    return msh_head(args);
  if (_stricmp(args[0], "tail") == 0)
    return msh_tail(args);
  if (_stricmp(args[0], "wc") == 0)
    return msh_wc(args);
  if (_stricmp(args[0], "mkdir") == 0)
    return msh_mkdir(args);
  if (_stricmp(args[0], "whoami") == 0)
    return msh_whoami(args);
  if (_stricmp(args[0], "hostname") == 0)
    return msh_hostname(args);
  if (_stricmp(args[0], "uptime") == 0)
    return msh_uptime(args);
  if (_stricmp(args[0], "echo") == 0)
    return msh_echo(args);
  if (_stricmp(args[0], "clear") == 0)
    return msh_clear(args);
  if (_stricmp(args[0], "tree") == 0)
    return msh_tree(args);
  if (_stricmp(args[0], "calc") == 0)
    return msh_calc(args);
  if (_stricmp(args[0], "color") == 0)
    return msh_color(args);

  /* Check builtin commands */
  for (int i = 0; i < msh_num_builtins(); i++) {
    if (_stricmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return msh_launch(args);
}

int msh_execute_line(const char *raw_line) {
  char *line = malloc(strlen(raw_line) + 1);
  if (!line)
    return MSH_CONTINUE;
  strcpy(line, raw_line);

  char **args;
  int status = MSH_CONTINUE;

  if (line[0] == '\0') {
    free(line);
    return MSH_CONTINUE;
  }

  /* 1. Expand history shortcuts (!! and !n) */
  line = expand_history_cmd(line);

  /* 2. Expand aliases */
  char *aliasExpanded = alias_expand(line);
  if (aliasExpanded) {
    free(line);
    line = aliasExpanded;
  }

  /* 3. Expand environment variables ($VAR) */
  char *envExpanded = expand_env_vars(line);
  if (envExpanded) {
    free(line);
    line = envExpanded;
  }

  /* 4. AI mode: chat by default, run shell commands with !<cmd> */
  if (ai_is_mode_enabled()) {
    char *trim = line;
    while (*trim == ' ' || *trim == '\t')
      trim++;

    if (*trim == '!') {
      trim++;
      while (*trim == ' ' || *trim == '\t')
        trim++;
      if (*trim == '\0') {
        print_info("AI mode: use !<command> to run shell commands");
        free(line);
        return MSH_CONTINUE;
      }
      if (trim != line) {
        memmove(line, trim, strlen(trim) + 1);
      }
    } else if (!is_ai_control_command(trim) && !is_ai_env_command(trim)) {
      int aiStatus = ai_chat_line(trim);
      free(line);
      return aiStatus;
    }
  }

  /* 5. Check for pipe operator */
  if (has_pipe(line)) {
    execute_piped(line);
    free(line);
    return MSH_CONTINUE;
  }

  /* 6. Check for I/O redirection */
  if (has_redirect(line)) {
    execute_redirected(line);
    free(line);
    return MSH_CONTINUE;
  }

  /* 7. Normal execution */
  args = msh_split_line(line);
  status = msh_execute(args);

  free(line);
  free(args);
  return status;
}

void msh_loop(void) {
  char *line;
  int status = MSH_CONTINUE;

  do {
    cleanup_zombies();

    hacker_prompt();

    line = msh_read_line();

    /* Skip empty lines */
    if (line[0] == '\0') {
      free(line);
      continue;
    }

    /* Add to history before processing (so raw input is saved) */
    char *expanded_for_history = expand_history_cmd(
        malloc(strlen(line) + 1) ? strcpy(malloc(strlen(line) + 1), line)
                                 : line);
    history_add(expanded_for_history);
    free(expanded_for_history);

    status = msh_execute_line(line);

    free(line);

  } while (status);

  /* Save state on exit */
  history_save();
  alias_save();
}
