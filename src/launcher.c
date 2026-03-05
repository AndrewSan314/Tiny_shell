#include "launcher.h"
#include "process_manager.h"
#include "colors.h"
#include <string.h>

#define MSH_CONTINUE 1
#define MSH_EXIT 0

extern HANDLE hForegroundProcess;

static int get_output_delay_ms(void) {
    char value[16];
    DWORD len = GetEnvironmentVariableA("MSH_OUTPUT_DELAY_MS", value, sizeof(value));
    int delayMs;

    if (len == 0 || len >= sizeof(value)) {
        return 5;
    }

    delayMs = atoi(value);
    if (delayMs < 0) {
        delayMs = 0;
    }
    if (delayMs > 100) {
        delayMs = 100;
    }
    return delayMs;
}

static void replay_process_output(HANDLE hReadPipe, int delayMs) {
    char buffer[256];
    DWORD bytesRead;

    if (!hReadPipe) {
        return;
    }

    while (ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        for (DWORD i = 0; i < bytesRead; i++) {
            putchar(buffer[i]);
            fflush(stdout);

            if (delayMs <= 0) {
                continue;
            }

            if (buffer[i] == '\n') {
                Sleep(delayMs * 2);
            } else if (buffer[i] != '\r') {
                Sleep(delayMs);
            }
        }
    }
}

int is_batch_file(const char *filename) {
    if(filename == NULL) {
        return 0;
    }

    const char *dot = strrchr(filename, '.');
    if(dot != NULL) {
        if(_stricmp(dot, ".bat") == 0 || _stricmp(dot, ".cmd") == 0) {
            return 1;
        }
    }
    return 0;
}

int msh_launch(char **args) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;
    HANDLE hReadPipe = NULL;
    HANDLE hWritePipe = NULL;
    char command[MAX_CMD_LEN] = {0};
    int background = 0;
    int outputDelayMs = 0;
    int i = 0;

    while(args[i] != NULL) i++;
    int arg_count = i;
    if(arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) {
        background = 1;
        args[arg_count - 1] = NULL;
    }

    if(args[0] == NULL) {
        return MSH_CONTINUE;
    }

    if(is_batch_file(args[0])) {
        strcpy(command, "cmd /c ");
        strcat(command, args[0]);
        for(i = 1; args[i] != NULL; i++) {
            strcat(command, " ");
            strcat(command, args[i]);
        }
    } else {
        for(i = 0; args[i] != NULL; i++) {
            strcat(command, args[i]);
            if(args[i + 1] != NULL) {
                strcat(command, " ");
            }
        }
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!background) {
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
            print_error("Failed to prepare output capture");
            return MSH_CONTINUE;
        }

        SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;
    }

    DWORD creationFlags = background ? 0 : CREATE_NEW_PROCESS_GROUP;

    if(!CreateProcess(NULL, command, NULL, NULL, background ? FALSE : TRUE,
                      creationFlags,
                      NULL, NULL, &si, &pi)) {
        char msg[MAX_CMD_LEN + 64];
        sprintf(msg, "Command not found: %s", args[0]);
        print_error(msg);
        if (hReadPipe) CloseHandle(hReadPipe);
        if (hWritePipe) CloseHandle(hWritePipe);
        return MSH_CONTINUE;
    }

    if(background) {
        char msg[128];
        sprintf(msg, "Started background process [PID %lu]", pi.dwProcessId);
        print_info(msg);
        add_bg_process(pi.dwProcessId, pi.hProcess, pi.hThread, command);
    } else {
        outputDelayMs = get_output_delay_ms();
        hForegroundProcess = pi.hProcess;

        if (hWritePipe) {
            CloseHandle(hWritePipe);
            hWritePipe = NULL;
        }

        replay_process_output(hReadPipe, outputDelayMs);
        WaitForSingleObject(pi.hProcess, INFINITE);
        hForegroundProcess = NULL;

        if (hReadPipe) CloseHandle(hReadPipe);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    
    return MSH_CONTINUE;
}

void print_win_error(const char *prefix) {
    DWORD errorMessageID = GetLastError();
    if(errorMessageID == 0) {
        return;
    }

    LPSTR messageBuffer = NULL;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer, 0, NULL);

    set_color(CLR_ERROR);
    fprintf(stderr, "%s: %s\n", prefix, messageBuffer);
    reset_color();

    LocalFree(messageBuffer);
}
