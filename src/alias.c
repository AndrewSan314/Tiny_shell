#include "alias.h"
#include "colors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char name[128];
    char command[MAX_CMD_LEN];
    int active;
} AliasEntry;

static AliasEntry aliases[MAX_ALIASES];
static int alias_count = 0;

static void get_alias_path(char *path, int maxLen) {
    char *home = getenv("USERPROFILE");
    if (home) {
        snprintf(path, maxLen, "%s\\%s", home, ALIAS_FILE);
    } else {
        snprintf(path, maxLen, "%s", ALIAS_FILE);
    }
}

void alias_init(void) {
    memset(aliases, 0, sizeof(aliases));
    alias_count = 0;
    alias_load();
}

void alias_set(const char *name, const char *command) {
    /* Update existing alias */
    for (int i = 0; i < alias_count; i++) {
        if (aliases[i].active && strcmp(aliases[i].name, name) == 0) {
            strncpy(aliases[i].command, command, MAX_CMD_LEN - 1);
            return;
        }
    }
    /* Add new alias */
    if (alias_count < MAX_ALIASES) {
        strncpy(aliases[alias_count].name, name, 127);
        strncpy(aliases[alias_count].command, command, MAX_CMD_LEN - 1);
        aliases[alias_count].active = 1;
        alias_count++;
    } else {
        print_error("Alias list full (max 50)");
    }
}

void alias_remove(const char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (aliases[i].active && strcmp(aliases[i].name, name) == 0) {
            aliases[i].active = 0;
            char msg[256];
            sprintf(msg, "Removed alias: %s", name);
            print_success(msg);
            return;
        }
    }
    char msg[256];
    sprintf(msg, "Alias not found: %s", name);
    print_warning(msg);
}

const char *alias_get(const char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (aliases[i].active && strcmp(aliases[i].name, name) == 0) {
            return aliases[i].command;
        }
    }
    return NULL;
}

void alias_save(void) {
    char path[MAX_PATH];
    get_alias_path(path, MAX_PATH);

    FILE *f = fopen(path, "w");
    if (!f) return;

    for (int i = 0; i < alias_count; i++) {
        if (aliases[i].active) {
            fprintf(f, "%s=%s\n", aliases[i].name, aliases[i].command);
        }
    }
    fclose(f);
}

void alias_load(void) {
    char path[MAX_PATH];
    get_alias_path(path, MAX_PATH);

    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[MAX_CMD_LEN + 128];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = '\0';
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            alias_set(line, eq + 1);
        }
    }
    fclose(f);
}

int msh_alias(char **args) {
    if (args[1] == NULL) {
        /* List all aliases */
        printf("\n");
        set_color(CLR_HEADER);
        printf("  Aliases\n");
        set_color(CLR_MUTED);
        printf("  ──────────────────────────────────────────────────\n");
        reset_color();

        int found = 0;
        for (int i = 0; i < alias_count; i++) {
            if (aliases[i].active) {
                set_color(CLR_BRIGHT_CYAN);
                printf("  %-15s", aliases[i].name);
                set_color(CLR_MUTED);
                printf(" -> ");
                reset_color();
                printf("%s\n", aliases[i].command);
                found++;
            }
        }
        if (found == 0) {
            print_info("No aliases defined. Use: alias name=command");
        }
        printf("\n");
        return MSH_CONTINUE;
    }

    /* Parse alias assignment: alias name=command */
    char *eq = strchr(args[1], '=');
    if (eq) {
        *eq = '\0';
        char *name = args[1];
        char *cmd = eq + 1;

        /* If command continues in next args, concatenate */
        char fullCmd[MAX_CMD_LEN] = {0};
        strncpy(fullCmd, cmd, MAX_CMD_LEN - 1);
        for (int i = 2; args[i] != NULL; i++) {
            strncat(fullCmd, " ", MAX_CMD_LEN - strlen(fullCmd) - 1);
            strncat(fullCmd, args[i], MAX_CMD_LEN - strlen(fullCmd) - 1);
        }

        alias_set(name, fullCmd);
        char msg[256];
        sprintf(msg, "Alias set: %s -> %s", name, fullCmd);
        print_success(msg);
    } else {
        /* Show specific alias */
        const char *cmd = alias_get(args[1]);
        if (cmd) {
            set_color(CLR_BRIGHT_CYAN);
            printf("  %s", args[1]);
            set_color(CLR_MUTED);
            printf(" -> ");
            reset_color();
            printf("%s\n", cmd);
        } else {
            char msg[256];
            sprintf(msg, "Alias not found: %s", args[1]);
            print_warning(msg);
        }
    }

    return MSH_CONTINUE;
}

int msh_unalias(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: unalias <name>");
        return MSH_CONTINUE;
    }
    alias_remove(args[1]);
    return MSH_CONTINUE;
}

char *alias_expand(const char *line) {
    /* Get the first word */
    char firstWord[128] = {0};
    int i = 0;
    while (line[i] && line[i] != ' ' && line[i] != '\t' && i < 127) {
        firstWord[i] = line[i];
        i++;
    }
    firstWord[i] = '\0';

    const char *expanded = alias_get(firstWord);
    if (expanded) {
        /* Replace the first word with the alias command */
        int expLen = (int)strlen(expanded);
        int restLen = (int)strlen(line + i);
        char *result = malloc(expLen + restLen + 2);
        if (result) {
            strcpy(result, expanded);
            strcat(result, line + i);
            return result;
        }
    }
    return NULL; /* No expansion */
}
