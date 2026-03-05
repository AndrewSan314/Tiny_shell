#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "../include/common.h"

extern ProcessInfo bg_procs[MAX_BG_PROCS];
extern HANDLE hForegroundProcess;

void init_process_manager(void);
void add_bg_process(DWORD pid, HANDLE hProc, HANDLE hThread, const char *cmd);
void cleanup_zombies(void);
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

int msh_list(char **args);
int msh_kill(char **args);
int msh_stop(char **args);
int msh_resume(char **args);

#endif
