#include "utils.h"
#include "colors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/*============================================================
 * FILE UTILITIES
 *============================================================*/

int msh_cat(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: cat <file1> [file2] ...");
        return MSH_CONTINUE;
    }

    for (int f = 1; args[f] != NULL; f++) {
        FILE *file = fopen(args[f], "r");
        if (!file) {
            char msg[512];
            sprintf(msg, "Cannot open: %s", args[f]);
            print_error(msg);
            continue;
        }

        char line[4096];
        int lineNum = 1;

        /* Show filename header when multiple files */
        if (args[2] != NULL) {
            set_color(CLR_HEADER);
            printf("\n  === %s ===\n", args[f]);
            reset_color();
        }

        while (fgets(line, sizeof(line), file)) {
            set_color(CLR_MUTED);
            printf("%4d ", lineNum++);
            reset_color();
            printf("%s", line);
        }

        /* Ensure newline at end */
        if (line[strlen(line) - 1] != '\n') printf("\n");
        fclose(file);
    }
    return MSH_CONTINUE;
}

int msh_touch(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: touch <file1> [file2] ...");
        return MSH_CONTINUE;
    }

    for (int i = 1; args[i] != NULL; i++) {
        HANDLE hFile = CreateFile(args[i], GENERIC_WRITE, 0, NULL,
                                  OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            /* Update the file's timestamp */
            FILETIME ft;
            SYSTEMTIME st;
            GetSystemTime(&st);
            SystemTimeToFileTime(&st, &ft);
            SetFileTime(hFile, NULL, NULL, &ft);
            CloseHandle(hFile);

            char msg[512];
            sprintf(msg, "Touched: %s", args[i]);
            print_success(msg);
        } else {
            char msg[512];
            sprintf(msg, "Cannot touch: %s", args[i]);
            print_error(msg);
        }
    }
    return MSH_CONTINUE;
}

int msh_rm(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: rm <file1> [file2] ...");
        return MSH_CONTINUE;
    }

    for (int i = 1; args[i] != NULL; i++) {
        if (DeleteFile(args[i])) {
            char msg[512];
            sprintf(msg, "Deleted: %s", args[i]);
            print_success(msg);
        } else {
            /* Try as directory */
            if (RemoveDirectory(args[i])) {
                char msg[512];
                sprintf(msg, "Removed directory: %s", args[i]);
                print_success(msg);
            } else {
                char msg[512];
                sprintf(msg, "Cannot remove: %s", args[i]);
                print_error(msg);
            }
        }
    }
    return MSH_CONTINUE;
}

int msh_cp(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        print_info("Usage: cp <source> <destination>");
        return MSH_CONTINUE;
    }

    if (CopyFile(args[1], args[2], FALSE)) {
        char msg[512];
        sprintf(msg, "Copied: %s -> %s", args[1], args[2]);
        print_success(msg);
    } else {
        char msg[512];
        sprintf(msg, "Failed to copy: %s", args[1]);
        print_error(msg);
    }
    return MSH_CONTINUE;
}

int msh_mv(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        print_info("Usage: mv <source> <destination>");
        return MSH_CONTINUE;
    }

    if (MoveFile(args[1], args[2])) {
        char msg[512];
        sprintf(msg, "Moved: %s -> %s", args[1], args[2]);
        print_success(msg);
    } else {
        char msg[512];
        sprintf(msg, "Failed to move: %s", args[1]);
        print_error(msg);
    }
    return MSH_CONTINUE;
}

int msh_head(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: head <file> [lines]");
        return MSH_CONTINUE;
    }

    int numLines = 10;
    if (args[2] != NULL) {
        numLines = atoi(args[2]);
        if (numLines <= 0) numLines = 10;
    }

    FILE *file = fopen(args[1], "r");
    if (!file) {
        char msg[512];
        sprintf(msg, "Cannot open: %s", args[1]);
        print_error(msg);
        return MSH_CONTINUE;
    }

    set_color(CLR_HEADER);
    printf("\n  First %d lines of %s:\n", numLines, args[1]);
    set_color(CLR_MUTED);
    printf("  ──────────────────────────────────────────────────\n");
    reset_color();

    char line[4096];
    int count = 0;
    while (count < numLines && fgets(line, sizeof(line), file)) {
        set_color(CLR_MUTED);
        printf("  %4d ", ++count);
        reset_color();
        printf("%s", line);
    }
    printf("\n");
    fclose(file);
    return MSH_CONTINUE;
}

int msh_tail(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: tail <file> [lines]");
        return MSH_CONTINUE;
    }

    int numLines = 10;
    if (args[2] != NULL) {
        numLines = atoi(args[2]);
        if (numLines <= 0) numLines = 10;
    }

    FILE *file = fopen(args[1], "r");
    if (!file) {
        char msg[512];
        sprintf(msg, "Cannot open: %s", args[1]);
        print_error(msg);
        return MSH_CONTINUE;
    }

    /* Count total lines */
    char line[4096];
    int totalLines = 0;
    while (fgets(line, sizeof(line), file)) {
        totalLines++;
    }

    /* Rewind and skip to the right position */
    rewind(file);
    int startLine = totalLines - numLines;
    if (startLine < 0) startLine = 0;

    set_color(CLR_HEADER);
    printf("\n  Last %d lines of %s:\n", numLines, args[1]);
    set_color(CLR_MUTED);
    printf("  ──────────────────────────────────────────────────\n");
    reset_color();

    int currentLine = 0;
    while (fgets(line, sizeof(line), file)) {
        if (currentLine >= startLine) {
            set_color(CLR_MUTED);
            printf("  %4d ", currentLine + 1);
            reset_color();
            printf("%s", line);
        }
        currentLine++;
    }
    printf("\n");
    fclose(file);
    return MSH_CONTINUE;
}

int msh_wc(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: wc <file1> [file2] ...");
        return MSH_CONTINUE;
    }

    printf("\n");
    set_color(CLR_HEADER);
    printf("  %-8s %-8s %-10s %s\n", "LINES", "WORDS", "CHARS", "FILE");
    set_color(CLR_MUTED);
    printf("  ──────────────────────────────────────────────────\n");
    reset_color();

    int totalLines = 0, totalWords = 0, totalChars = 0;

    for (int f = 1; args[f] != NULL; f++) {
        FILE *file = fopen(args[f], "r");
        if (!file) {
            char msg[512];
            sprintf(msg, "Cannot open: %s", args[f]);
            print_error(msg);
            continue;
        }

        int lines = 0, words = 0, chars = 0;
        int inWord = 0;
        int c;

        while ((c = fgetc(file)) != EOF) {
            chars++;
            if (c == '\n') lines++;
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                inWord = 0;
            } else if (!inWord) {
                inWord = 1;
                words++;
            }
        }

        set_color(CLR_ACCENT);
        printf("  %-8d %-8d %-10d ", lines, words, chars);
        set_color(CLR_BRIGHT_CYAN);
        printf("%s\n", args[f]);
        reset_color();

        totalLines += lines;
        totalWords += words;
        totalChars += chars;
        fclose(file);
    }

    /* Show total if multiple files */
    if (args[2] != NULL) {
        set_color(CLR_MUTED);
        printf("  ──────────────────────────────────────────────────\n");
        reset_color();
        set_color(CLR_BRIGHT_WHITE);
        printf("  %-8d %-8d %-10d total\n", totalLines, totalWords, totalChars);
        reset_color();
    }
    printf("\n");
    return MSH_CONTINUE;
}

int msh_mkdir(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: mkdir <directory>");
        return MSH_CONTINUE;
    }

    for (int i = 1; args[i] != NULL; i++) {
        if (CreateDirectory(args[i], NULL)) {
            char msg[512];
            sprintf(msg, "Created directory: %s", args[i]);
            print_success(msg);
        } else {
            DWORD err = GetLastError();
            if (err == ERROR_ALREADY_EXISTS) {
                char msg[512];
                sprintf(msg, "Directory already exists: %s", args[i]);
                print_warning(msg);
            } else {
                char msg[512];
                sprintf(msg, "Failed to create: %s", args[i]);
                print_error(msg);
            }
        }
    }
    return MSH_CONTINUE;
}

/*============================================================
 * SYSTEM UTILITIES
 *============================================================*/

int msh_whoami(char **args) {
    (void)args;
    char userName[256];
    DWORD userSize = sizeof(userName);
    GetUserName(userName, &userSize);
    set_color(CLR_BRIGHT_CYAN);
    printf("%s\n", userName);
    reset_color();
    return MSH_CONTINUE;
}

int msh_hostname(char **args) {
    (void)args;
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD compSize = sizeof(computerName);
    GetComputerName(computerName, &compSize);
    set_color(CLR_BRIGHT_CYAN);
    printf("%s\n", computerName);
    reset_color();
    return MSH_CONTINUE;
}

int msh_uptime(char **args) {
    (void)args;
    ULONGLONG uptimeSec = GetTickCount64() / 1000;
    int days = (int)(uptimeSec / 86400);
    int hours = (int)((uptimeSec % 86400) / 3600);
    int mins = (int)((uptimeSec % 3600) / 60);
    int secs = (int)(uptimeSec % 60);

    set_color(CLR_ACCENT);
    printf("  Uptime: ");
    reset_color();
    printf("%d days, %d hours, %d minutes, %d seconds\n", days, hours, mins, secs);
    return MSH_CONTINUE;
}

int msh_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        if (i > 1) printf(" ");
        printf("%s", args[i]);
    }
    printf("\n");
    return MSH_CONTINUE;
}

int msh_clear(char **args) {
    (void)args;
    system("cls");
    return MSH_CONTINUE;
}
