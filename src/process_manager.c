#include "process_manager.h"
#include "colors.h"
#include "hacker.h"

ProcessInfo bg_procs[MAX_BG_PROCS];
HANDLE hForegroundProcess = NULL;

void init_process_manager(void) {
     for(int i = 0; i < MAX_BG_PROCS; i++){ 
        bg_procs[i].is_active = 0; 
     }
     hForegroundProcess = NULL;
}

void add_bg_process(DWORD pid, HANDLE hProc, HANDLE hThread, const char *cmd) {
     for(int i = 0; i < MAX_BG_PROCS; i++){ 
        if(bg_procs[i].is_active == 0){ 
            bg_procs[i].pid = pid;
            bg_procs[i].hProcess = hProc;
            bg_procs[i].hThread = hThread;
            strncpy(bg_procs[i].cmd, cmd, MAX_CMD_LEN);
            bg_procs[i].is_active = 1;
            bg_procs[i].is_suspended = 0;
            return;
        }
     }
     print_error("Process list is full!");
}

void cleanup_zombies(void) {
     DWORD exitCode;
     for(int i = 0; i < MAX_BG_PROCS; i++){ 
        if(bg_procs[i].is_active == 1){ 
            if(GetExitCodeProcess(bg_procs[i].hProcess, &exitCode) && exitCode != STILL_ACTIVE){ 
                CloseHandle(bg_procs[i].hProcess); 
                CloseHandle(bg_procs[i].hThread); 
                bg_procs[i].is_active = 0;
            }
        }
     }
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT) {
        printf("\n");
        if (hForegroundProcess != NULL) {
            TerminateProcess(hForegroundProcess, 1);
            hForegroundProcess = NULL;
            print_warning("Terminated foreground process");
        } else {
            print_info("No foreground process. Type 'exit' to quit.");
            hacker_begin_prompt();
            hacker_prompt();
        }
        return TRUE;
    }
    return FALSE;
}

int msh_list(char **args) {
    (void)args;
    
    cleanup_zombies();

    printf("\n");
    set_color(CLR_HEADER);
    printf("  %-10s %-12s %s\n", "PID", "STATUS", "COMMAND");
    set_color(CLR_MUTED);
    printf("  ──────────────────────────────────────────────────\n");
    reset_color();

    int count = 0;
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active) {
            set_color(CLR_ACCENT);
            printf("  %-10lu ", bg_procs[i].pid);
            
            if (bg_procs[i].is_suspended) {
                set_color(CLR_WARNING);
                printf("%-12s ", "Stopped");
            } else {
                set_color(CLR_SUCCESS);
                printf("%-12s ", "Running");
            }
            
            reset_color();
            printf("%.30s\n", bg_procs[i].cmd);
            count++;
        }
    }
    if (count == 0) {
        print_info("No background processes");
    }
    printf("\n");
    
    return MSH_CONTINUE;
}

int msh_kill(char **args) {
    if (args[1] == NULL) { 
        print_info("Usage: kill <pid>"); 
        return MSH_CONTINUE; 
    }
    DWORD targetPid = (DWORD)atoi(args[1]);
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active && bg_procs[i].pid == targetPid) {
            if (TerminateProcess(bg_procs[i].hProcess, 0)) {
                CloseHandle(bg_procs[i].hProcess);
                CloseHandle(bg_procs[i].hThread);
                bg_procs[i].is_active = 0;
                char msg[128];
                sprintf(msg, "Process %lu killed", targetPid);
                print_success(msg);
            } else {
                char msg[128];
                sprintf(msg, "Failed to kill process %lu (Error: %lu)", targetPid, GetLastError());
                print_error(msg);
            }
            return MSH_CONTINUE;
        }
    }
    char msg[128];
    sprintf(msg, "Process %lu not found", targetPid);
    print_warning(msg);
    return MSH_CONTINUE;
}

int msh_stop(char **args) {
    if (args[1] == NULL) { 
        print_info("Usage: stop <pid>"); 
        return MSH_CONTINUE; 
    }
    DWORD targetPid = (DWORD)atoi(args[1]);
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active && bg_procs[i].pid == targetPid) {
            if (SuspendThread(bg_procs[i].hThread) != (DWORD)-1) {
                bg_procs[i].is_suspended = 1;
                char msg[128];
                sprintf(msg, "Process %lu stopped", targetPid);
                print_success(msg);
            } else {
                char msg[128];
                sprintf(msg, "Failed to stop process %lu", targetPid);
                print_error(msg);
            }
            return MSH_CONTINUE;
        }
    }
    char msg[128];
    sprintf(msg, "Process %lu not found", targetPid);
    print_warning(msg);
    return MSH_CONTINUE;
}

int msh_resume(char **args) {
    if (args[1] == NULL) { 
        print_info("Usage: resume <pid>"); 
        return MSH_CONTINUE; 
    }
    DWORD targetPid = (DWORD)atoi(args[1]);
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active && bg_procs[i].pid == targetPid) {
            if (ResumeThread(bg_procs[i].hThread) != (DWORD)-1) {
                bg_procs[i].is_suspended = 0;
                char msg[128];
                sprintf(msg, "Process %lu resumed", targetPid);
                print_success(msg);
            } else {
                char msg[128];
                sprintf(msg, "Failed to resume process %lu", targetPid);
                print_error(msg);
            }
            return MSH_CONTINUE;
        }
    }
    char msg[128];
    sprintf(msg, "Process %lu not found", targetPid);
    print_warning(msg);
    return MSH_CONTINUE;
}
