#include "history.h"
#include "colors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char history_buf[HISTORY_MAX][MAX_CMD_LEN];
static int history_total = 0;       /* Total commands added */
static int history_start = 0;       /* Start index in circular buffer */
static int history_size = 0;        /* Current number of entries */
static int nav_pos = -1;            /* Navigation position (-1 = not navigating) */

static void get_history_path(char *path, int maxLen) {
    char *home = getenv("USERPROFILE");
    if (home) {
        snprintf(path, maxLen, "%s\\%s", home, HISTORY_FILE);
    } else {
        snprintf(path, maxLen, "%s", HISTORY_FILE);
    }
}

void history_init(void) {
    memset(history_buf, 0, sizeof(history_buf));
    history_total = 0;
    history_start = 0;
    history_size = 0;
    nav_pos = -1;
    history_load();
}

void history_add(const char *cmd) {
    if (cmd == NULL || cmd[0] == '\0') return;

    /* Don't add duplicates of the last command */
    if (history_size > 0) {
        int lastIdx = (history_start + history_size - 1) % HISTORY_MAX;
        if (strcmp(history_buf[lastIdx], cmd) == 0) return;
    }

    if (history_size < HISTORY_MAX) {
        int idx = (history_start + history_size) % HISTORY_MAX;
        strncpy(history_buf[idx], cmd, MAX_CMD_LEN - 1);
        history_buf[idx][MAX_CMD_LEN - 1] = '\0';
        history_size++;
    } else {
        /* Overwrite oldest entry */
        strncpy(history_buf[history_start], cmd, MAX_CMD_LEN - 1);
        history_buf[history_start][MAX_CMD_LEN - 1] = '\0';
        history_start = (history_start + 1) % HISTORY_MAX;
    }
    history_total++;
    nav_pos = -1;
}

const char *history_get(int index) {
    if (index < 0 || index >= history_size) return NULL;
    /* index 0 = oldest, history_size-1 = most recent */
    int realIdx = (history_start + index) % HISTORY_MAX;
    return history_buf[realIdx];
}

int history_count(void) {
    return history_size;
}

const char *history_navigate(int direction) {
    if (history_size == 0) return NULL;

    if (direction < 0) {
        /* Go older (Up arrow) */
        if (nav_pos < 0) {
            nav_pos = history_size - 1; /* Start from most recent */
        } else if (nav_pos > 0) {
            nav_pos--;
        }
    } else {
        /* Go newer (Down arrow) */
        if (nav_pos < 0) return NULL;
        nav_pos++;
        if (nav_pos >= history_size) {
            nav_pos = -1;
            return NULL; /* Past newest = empty line */
        }
    }

    if (nav_pos >= 0 && nav_pos < history_size) {
        return history_get(nav_pos);
    }
    return NULL;
}

void history_reset_nav(void) {
    nav_pos = -1;
}

void history_save(void) {
    char path[MAX_PATH];
    get_history_path(path, MAX_PATH);

    FILE *f = fopen(path, "w");
    if (!f) return;

    for (int i = 0; i < history_size; i++) {
        const char *cmd = history_get(i);
        if (cmd) {
            fprintf(f, "%s\n", cmd);
        }
    }
    fclose(f);
}

void history_load(void) {
    char path[MAX_PATH];
    get_history_path(path, MAX_PATH);

    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[MAX_CMD_LEN];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] != '\0') {
            history_add(line);
        }
    }
    fclose(f);
}

int msh_history(char **args) {
    (void)args;

    if (history_size == 0) {
        print_info("No command history");
        return MSH_CONTINUE;
    }

    printf("\n");
    set_color(CLR_HEADER);
    printf("  Command History\n");
    set_color(CLR_MUTED);
    printf("  ──────────────────────────────────────────────────\n");
    reset_color();

    int startNum = (history_total > history_size) 
                   ? (history_total - history_size + 1) 
                   : 1;

    for (int i = 0; i < history_size; i++) {
        const char *cmd = history_get(i);
        if (cmd) {
            set_color(CLR_ACCENT);
            printf("  %4d  ", startNum + i);
            reset_color();
            printf("%s\n", cmd);
        }
    }
    printf("\n");

    return MSH_CONTINUE;
}
