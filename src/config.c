#include "config.h"
#include "core.h"
#include "colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void get_rc_path(char *path, int maxLen) {
    char *home = getenv("USERPROFILE");
    if (home) {
        snprintf(path, maxLen, "%s\\.mshrc", home);
    } else {
        snprintf(path, maxLen, ".mshrc");
    }
}

void config_load(void) {
    char path[MAX_PATH];
    get_rc_path(path, MAX_PATH);

    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[MAX_CMD_LEN];
    while (fgets(line, sizeof(line), f)) {
        /* Strip newline characters */
        line[strcspn(line, "\r\n")] = '\0';
        
        /* Skip empty lines and comments */
        if (line[0] == '\0' || line[0] == '#') continue;

        /* Execute the line as if it was typed */
        msh_execute_line(line);
    }
    fclose(f);
}

int msh_source(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: source <file>");
        return MSH_CONTINUE;
    }

    FILE *f = fopen(args[1], "r");
    if (!f) {
        char msg[512];
        snprintf(msg, sizeof(msg), "Cannot open file: %s", args[1]);
        print_error(msg);
        return MSH_CONTINUE;
    }

    char line[MAX_CMD_LEN];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;
        msh_execute_line(line);
    }
    fclose(f);

    return MSH_CONTINUE;
}
