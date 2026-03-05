#include "pipe_redirect.h"
#include "colors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int has_pipe(const char *line) {
    /* Simple check - not inside quotes */
    return strchr(line, '|') != NULL;
}

int has_redirect(const char *line) {
    return strchr(line, '>') != NULL || strchr(line, '<') != NULL;
}

int execute_piped(const char *line) {
    /* Count pipes */
    char lineCopy[MAX_CMD_LEN];
    strncpy(lineCopy, line, MAX_CMD_LEN - 1);
    lineCopy[MAX_CMD_LEN - 1] = '\0';

    /* Split by pipe */
    char *commands[10];
    int cmdCount = 0;
    char *token = strtok(lineCopy, "|");
    while (token && cmdCount < 10) {
        /* Trim whitespace */
        while (*token == ' ') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ') { *end = '\0'; end--; }
        commands[cmdCount++] = token;
        token = strtok(NULL, "|");
    }

    if (cmdCount < 2) {
        print_error("Invalid pipe syntax");
        return MSH_CONTINUE;
    }

    /* For piped commands, we use CreateProcess with pipe handles */
    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    HANDLE hPrevRead = NULL;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    PROCESS_INFORMATION piArr[10];
    int processCount = 0;

    for (int i = 0; i < cmdCount; i++) {
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;

        /* Set up stdin from previous pipe */
        if (i > 0 && hPrevRead) {
            si.hStdInput = hPrevRead;
        } else {
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        }

        /* Set up stdout to next pipe (except last command) */
        if (i < cmdCount - 1) {
            if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
                print_error("Failed to create pipe");
                break;
            }
            /* Ensure read handle is not inherited */
            SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);
            si.hStdOutput = hWritePipe;
        } else {
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            hReadPipe = NULL;
            hWritePipe = NULL;
        }

        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        /* Build command string - prefix with cmd /c for Windows compatibility */
        char cmdStr[MAX_CMD_LEN];
        snprintf(cmdStr, MAX_CMD_LEN, "cmd /c %s", commands[i]);

        ZeroMemory(&piArr[processCount], sizeof(PROCESS_INFORMATION));

        if (!CreateProcess(NULL, cmdStr, NULL, NULL, TRUE,
                          0, NULL, NULL, &si, &piArr[processCount])) {
            char msg[512];
            sprintf(msg, "Failed to run: %s", commands[i]);
            print_error(msg);
            break;
        }
        processCount++;

        /* Close write end of pipe (parent no longer needs it) */
        if (i < cmdCount - 1 && hWritePipe) {
            CloseHandle(hWritePipe);
            hWritePipe = NULL;
        }

        /* Close previous read handle */
        if (hPrevRead) {
            CloseHandle(hPrevRead);
        }

        /* Save current read handle for next command */
        hPrevRead = hReadPipe;
    }

    /* Wait for all processes to finish */
    for (int i = 0; i < processCount; i++) {
        WaitForSingleObject(piArr[i].hProcess, INFINITE);
        CloseHandle(piArr[i].hProcess);
        CloseHandle(piArr[i].hThread);
    }

    if (hPrevRead) CloseHandle(hPrevRead);

    return MSH_CONTINUE;
}

int execute_redirected(char *line) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    char command[MAX_CMD_LEN] = {0};
    char filename[MAX_PATH] = {0};
    int redirectType = 0; /* 1: >, 2: >>, 3: < */

    /* Parse the line for redirection */
    char *pos;
    if ((pos = strstr(line, ">>")) != NULL) {
        redirectType = 2;
        *pos = '\0';
        char *fname = pos + 2;
        while (*fname == ' ') fname++;
        char *end = fname + strlen(fname) - 1;
        while (end > fname && *end == ' ') { *end = '\0'; end--; }
        strncpy(filename, fname, MAX_PATH - 1);
    } else if ((pos = strchr(line, '>')) != NULL) {
        redirectType = 1;
        *pos = '\0';
        char *fname = pos + 1;
        while (*fname == ' ') fname++;
        char *end = fname + strlen(fname) - 1;
        while (end > fname && *end == ' ') { *end = '\0'; end--; }
        strncpy(filename, fname, MAX_PATH - 1);
    } else if ((pos = strchr(line, '<')) != NULL) {
        redirectType = 3;
        *pos = '\0';
        char *fname = pos + 1;
        while (*fname == ' ') fname++;
        char *end = fname + strlen(fname) - 1;
        while (end > fname && *end == ' ') { *end = '\0'; end--; }
        strncpy(filename, fname, MAX_PATH - 1);
    }

    /* Trim the command part */
    char *cmd = line;
    while (*cmd == ' ') cmd++;
    char *end = cmd + strlen(cmd) - 1;
    while (end > cmd && *end == ' ') { *end = '\0'; end--; }

    snprintf(command, MAX_CMD_LEN, "cmd /c %s", cmd);

    if (filename[0] == '\0') {
        print_error("Missing filename for redirection");
        return MSH_CONTINUE;
    }

    /* Open file for redirection */
    if (redirectType == 1) {
        hFile = CreateFile(filename, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL, NULL);
        si.hStdOutput = hFile;
    } else if (redirectType == 2) {
        hFile = CreateFile(filename, GENERIC_WRITE, 0, &sa, OPEN_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            SetFilePointer(hFile, 0, NULL, FILE_END);
        }
        si.hStdOutput = hFile;
    } else if (redirectType == 3) {
        hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL, NULL);
        si.hStdInput = hFile;
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        char msg[512];
        sprintf(msg, "Cannot open file: %s", filename);
        print_error(msg);
        return MSH_CONTINUE;
    }

    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, command, NULL, NULL, TRUE,
                      0, NULL, NULL, &si, &pi)) {
        char msg[512];
        sprintf(msg, "Failed to run: %s", cmd);
        print_error(msg);
        CloseHandle(hFile);
        return MSH_CONTINUE;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hFile);

    return MSH_CONTINUE;
}

int msh_export(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: export VAR=value");
        return MSH_CONTINUE;
    }

    char *eq = strchr(args[1], '=');
    if (eq) {
        *eq = '\0';
        char *varName = args[1];
        char *varValue = eq + 1;

        /* If value continues in next args, concatenate */
        char fullValue[MAX_CMD_LEN] = {0};
        strncpy(fullValue, varValue, MAX_CMD_LEN - 1);
        for (int i = 2; args[i] != NULL; i++) {
            strncat(fullValue, " ", MAX_CMD_LEN - strlen(fullValue) - 1);
            strncat(fullValue, args[i], MAX_CMD_LEN - strlen(fullValue) - 1);
        }

        if (SetEnvironmentVariable(varName, fullValue)) {
            char msg[512];
            sprintf(msg, "Set %s=%s", varName, fullValue);
            print_success(msg);
        } else {
            print_error("Failed to set environment variable");
        }
    } else {
        /* Show the value of a variable */
        char value[32768];
        if (GetEnvironmentVariable(args[1], value, sizeof(value)) > 0) {
            set_color(CLR_BRIGHT_CYAN);
            printf("  %s", args[1]);
            set_color(CLR_MUTED);
            printf("=");
            reset_color();
            printf("%s\n", value);
        } else {
            char msg[256];
            sprintf(msg, "Variable not set: %s", args[1]);
            print_warning(msg);
        }
    }

    return MSH_CONTINUE;
}

int msh_unset(char **args) {
    if (args[1] == NULL) {
        print_info("Usage: unset <variable>");
        return MSH_CONTINUE;
    }
    if (SetEnvironmentVariable(args[1], NULL)) {
        char msg[256];
        sprintf(msg, "Unset variable: %s", args[1]);
        print_success(msg);
    } else {
        print_error("Failed to unset variable");
    }
    return MSH_CONTINUE;
}

int msh_env(char **args) {
    (void)args;
    
    char *envBlock = GetEnvironmentStrings();
    if (!envBlock) {
        print_error("Cannot read environment");
        return MSH_CONTINUE;
    }

    printf("\n");
    set_color(CLR_HEADER);
    printf("  Environment Variables\n");
    set_color(CLR_MUTED);
    printf("  ──────────────────────────────────────────────────\n");
    reset_color();

    char *current = envBlock;
    while (*current) {
        /* Skip variables starting with = (internal) */
        if (*current != '=') {
            char *eq = strchr(current, '=');
            if (eq) {
                set_color(CLR_BRIGHT_CYAN);
                printf("  %.*s", (int)(eq - current), current);
                set_color(CLR_MUTED);
                printf("=");
                reset_color();
                /* Truncate long values */
                char *val = eq + 1;
                if (strlen(val) > 60) {
                    printf("%.57s...\n", val);
                } else {
                    printf("%s\n", val);
                }
            }
        }
        current += strlen(current) + 1;
    }
    printf("\n");

    FreeEnvironmentStrings(envBlock);
    return MSH_CONTINUE;
}

char *expand_env_vars(const char *line) {
    char result[MAX_CMD_LEN * 2] = {0};
    int ri = 0;
    int len = (int)strlen(line);

    for (int i = 0; i < len && ri < MAX_CMD_LEN * 2 - 1; i++) {
        if (line[i] == '$' && i + 1 < len && (line[i+1] == '_' || 
            (line[i+1] >= 'A' && line[i+1] <= 'Z') ||
            (line[i+1] >= 'a' && line[i+1] <= 'z'))) {
            /* Extract variable name */
            char varName[256] = {0};
            int vi = 0;
            i++; /* skip $ */
            while (i < len && (line[i] == '_' || 
                   (line[i] >= 'A' && line[i] <= 'Z') ||
                   (line[i] >= 'a' && line[i] <= 'z') ||
                   (line[i] >= '0' && line[i] <= '9'))) {
                varName[vi++] = line[i++];
            }
            varName[vi] = '\0';
            i--; /* back one for the loop increment */

            /* Look up the variable */
            char value[32768];
            if (GetEnvironmentVariable(varName, value, sizeof(value)) > 0) {
                int vLen = (int)strlen(value);
                if (ri + vLen < MAX_CMD_LEN * 2 - 1) {
                    memcpy(result + ri, value, vLen);
                    ri += vLen;
                }
            }
            /* If not found, it just gets removed (like bash) */
        } else {
            result[ri++] = line[i];
        }
    }
    result[ri] = '\0';

    char *expanded = malloc(ri + 1);
    if (expanded) {
        strcpy(expanded, result);
    }
    return expanded;
}
