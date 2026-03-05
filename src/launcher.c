#include "launcher.h"
#include "process_manager.h"
#include "colors.h"
#include <string.h>

#define MSH_CONTINUE 1
#define MSH_EXIT 0

extern HANDLE hForegroundProcess;

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
    char command[MAX_CMD_LEN] = {0};
    int background = 0;
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

    DWORD creationFlags = background ? 0 : CREATE_NEW_PROCESS_GROUP;

    if(!CreateProcess(NULL, command, NULL, NULL, FALSE,
                      creationFlags,
                      NULL, NULL, &si, &pi)) {
        char msg[MAX_CMD_LEN + 64];
        sprintf(msg, "Command not found: %s", args[0]);
        print_error(msg);
        return MSH_CONTINUE;
    }

    if(background) {
        char msg[128];
        sprintf(msg, "Started background process [PID %lu]", pi.dwProcessId);
        print_info(msg);
        add_bg_process(pi.dwProcessId, pi.hProcess, pi.hThread, command);
    } else {
        hForegroundProcess = pi.hProcess;
        while(hForegroundProcess != NULL) {
            WaitForSingleObject(pi.hProcess, 100);
            DWORD exitCode;
            if(GetExitCodeProcess(pi.hProcess, &exitCode) && exitCode != STILL_ACTIVE) {
                break;
            }
        }
        hForegroundProcess = NULL;
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
