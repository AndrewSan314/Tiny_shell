#include "readline.h"
#include "history.h"
#include "builtins.h"
#include "colors.h"
#include "hacker.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

/* Special key codes from _getch() */
#define KEY_EXTENDED    0
#define KEY_EXTENDED2   224
#define KEY_UP          72
#define KEY_DOWN        80
#define KEY_LEFT        75
#define KEY_RIGHT       77
#define KEY_HOME        71
#define KEY_END         79
#define KEY_DELETE      83
#define KEY_BACKSPACE   8
#define KEY_TAB         9
#define KEY_ENTER       13
#define KEY_ESCAPE      27

/* Redraw the current line buffer from scratch */
static void redraw_line(char *buf, int len, int cursor) {
    /* Move to start of line, clear it, reprint */
    printf("\r");
    /* Print prompt again */
    hacker_prompt();
    /* Print buffer */
    printf("%.*s", len, buf);
    /* Clear any leftover characters */
    printf("   \b\b\b");
    /* Move cursor to correct position */
    int back = len - cursor;
    for (int i = 0; i < back; i++) {
        printf("\b");
    }
    fflush(stdout);
}

/* Try to autocomplete the current input */
static int try_autocomplete(char *buf, int *len, int *cursor) {
    if (*len == 0) return 0;

    /* Find the last word (for file completion) */
    char *lastSpace = strrchr(buf, ' ');
    char *prefix;
    int prefixStart;

    if (lastSpace) {
        prefix = lastSpace + 1;
        prefixStart = (int)(prefix - buf);
    } else {
        prefix = buf;
        prefixStart = 0;
    }

    int prefixLen = (int)strlen(prefix);
    if (prefixLen == 0) return 0;

    char matches[20][MAX_PATH];
    int matchCount = 0;

    /* 1. Match against builtin commands (only if first word) */
    if (prefixStart == 0) {
        for (int i = 0; i < msh_num_builtins() && matchCount < 20; i++) {
            if (_strnicmp(builtin_str[i], prefix, prefixLen) == 0) {
                strncpy(matches[matchCount++], builtin_str[i], MAX_PATH - 1);
            }
        }
        /* Also match "history" */
        if (_strnicmp("history", prefix, prefixLen) == 0) {
            strncpy(matches[matchCount++], "history", MAX_PATH - 1);
        }
    }

    /* 2. Match against files in current directory */
    {
        WIN32_FIND_DATA fd;
        char searchPattern[MAX_PATH];
        snprintf(searchPattern, MAX_PATH, "%s*", prefix);

        HANDLE hFind = FindFirstFile(searchPattern, &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
                    continue;
                if (matchCount < 20) {
                    strncpy(matches[matchCount++], fd.cFileName, MAX_PATH - 1);
                }
            } while (FindNextFile(hFind, &fd));
            FindClose(hFind);
        }
    }

    if (matchCount == 0) return 0;

    if (matchCount == 1) {
        /* Single match: complete it */
        const char *completion = matches[0] + prefixLen;
        int compLen = (int)strlen(completion);

        /* Insert completion into buffer */
        memmove(buf + *cursor + compLen, buf + *cursor, *len - *cursor + 1);
        memcpy(buf + *cursor, completion, compLen);
        *len += compLen;
        *cursor += compLen;

        /* Add space after completion if it's a command */
        if (prefixStart == 0) {
            memmove(buf + *cursor + 1, buf + *cursor, *len - *cursor + 1);
            buf[*cursor] = ' ';
            (*len)++;
            (*cursor)++;
        }

        buf[*len] = '\0';
        return 1;
    } else {
        /* Multiple matches: show them all */
        printf("\n");
        for (int i = 0; i < matchCount; i++) {
            /* Color directories vs files */
            WIN32_FIND_DATA fd;
            HANDLE hf = FindFirstFile(matches[i], &fd);
            if (hf != INVALID_HANDLE_VALUE) {
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    set_color(CLR_DIR_COLOR);
                } else {
                    set_color(CLR_FILE_COLOR);
                }
                FindClose(hf);
            } else {
                set_color(CLR_BRIGHT_CYAN); /* builtin command */
            }
            printf("  %s", matches[i]);
            reset_color();
        }
        printf("\n");

        /* Find common prefix among all matches */
        int common = prefixLen;
        while (1) {
            char c = matches[0][common];
            if (c == '\0') break;
            int allMatch = 1;
            for (int i = 1; i < matchCount; i++) {
                if (matches[i][common] == '\0' || 
                    tolower((unsigned char)matches[i][common]) != tolower((unsigned char)c)) {
                    allMatch = 0;
                    break;
                }
            }
            if (!allMatch) break;
            common++;
        }

        /* Complete up to the common prefix */
        if (common > prefixLen) {
            int compLen = common - prefixLen;
            memmove(buf + *cursor + compLen, buf + *cursor, *len - *cursor + 1);
            memcpy(buf + *cursor, matches[0] + prefixLen, compLen);
            *len += compLen;
            *cursor += compLen;
            buf[*len] = '\0';
        }

        return 1;
    }
}

char *msh_readline(void) {
    int bufsize = MSH_RL_BUFSIZE;
    char *buf = malloc(bufsize);
    if (!buf) {
        print_error("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    memset(buf, 0, bufsize);
    int len = 0;
    int cursor = 0;

    history_reset_nav();

    while (1) {
        int ch = _getch();

        /* Handle special keys */
        if (ch == KEY_EXTENDED || ch == KEY_EXTENDED2) {
            int key = _getch();
            switch (key) {
                case KEY_UP: {
                    const char *histCmd = history_navigate(-1);
                    if (histCmd) {
                        strncpy(buf, histCmd, bufsize - 1);
                        len = (int)strlen(buf);
                        cursor = len;
                        redraw_line(buf, len, cursor);
                    }
                    break;
                }
                case KEY_DOWN: {
                    const char *histCmd = history_navigate(1);
                    if (histCmd) {
                        strncpy(buf, histCmd, bufsize - 1);
                    } else {
                        buf[0] = '\0';
                    }
                    len = (int)strlen(buf);
                    cursor = len;
                    redraw_line(buf, len, cursor);
                    break;
                }
                case KEY_LEFT:
                    if (cursor > 0) {
                        cursor--;
                        printf("\b");
                        fflush(stdout);
                    }
                    break;
                case KEY_RIGHT:
                    if (cursor < len) {
                        printf("%c", buf[cursor]);
                        cursor++;
                        fflush(stdout);
                    }
                    break;
                case KEY_HOME:
                    while (cursor > 0) {
                        printf("\b");
                        cursor--;
                    }
                    fflush(stdout);
                    break;
                case KEY_END:
                    while (cursor < len) {
                        printf("%c", buf[cursor]);
                        cursor++;
                    }
                    fflush(stdout);
                    break;
                case KEY_DELETE:
                    if (cursor < len) {
                        memmove(buf + cursor, buf + cursor + 1, len - cursor);
                        len--;
                        buf[len] = '\0';
                        /* Reprint from cursor to end */
                        printf("%s ", buf + cursor);
                        for (int i = 0; i <= len - cursor; i++) printf("\b");
                        fflush(stdout);
                    }
                    break;
            }
            continue;
        }

        /* Handle regular keys */
        switch (ch) {
            case KEY_ENTER:
                printf("\n");
                buf[len] = '\0';
                return buf;

            case KEY_BACKSPACE:
                if (cursor > 0) {
                    memmove(buf + cursor - 1, buf + cursor, len - cursor + 1);
                    cursor--;
                    len--;
                    /* Erase and reprint */
                    printf("\b%s \b", buf + cursor);
                    for (int i = 0; i < len - cursor; i++) printf("\b");
                    fflush(stdout);
                }
                break;

            case KEY_TAB:
                if (try_autocomplete(buf, &len, &cursor)) {
                    redraw_line(buf, len, cursor);
                }
                break;

            case KEY_ESCAPE:
                /* Clear the current line */
                buf[0] = '\0';
                len = 0;
                cursor = 0;
                redraw_line(buf, len, cursor);
                break;

            default:
                if (ch >= 32 && ch < 127) {
                    /* Grow buffer if needed */
                    if (len + 1 >= bufsize) {
                        bufsize += MSH_RL_BUFSIZE;
                        char *newbuf = realloc(buf, bufsize);
                        if (!newbuf) {
                            free(buf);
                            print_error("Memory allocation failed");
                            exit(EXIT_FAILURE);
                        }
                        buf = newbuf;
                    }
                    /* Insert character at cursor */
                    memmove(buf + cursor + 1, buf + cursor, len - cursor + 1);
                    buf[cursor] = (char)ch;
                    len++;
                    cursor++;
                    /* Print from cursor onward */
                    printf("%s", buf + cursor - 1);
                    for (int i = 0; i < len - cursor; i++) printf("\b");
                    fflush(stdout);
                }
                break;
        }
    }
}
