/*
 * main_original.c - FILE GỐC ĐẦY ĐỦ (CHỈ ĐỌC ĐỂ THAM KHẢO)
 * 
 * ⚠️ KHÔNG SỬA FILE NÀY - Dùng để tham khảo cách implement
 * 
 * Mỗi người đọc phần code liên quan đến mình, sau đó tự viết lại
 * vào file skeleton tương ứng trong thư mục src/
 * 
 * PHÂN CÔNG:
 * - Person 1: Lines 321-373 (msh_read_line, msh_split_line, msh_loop)
 * - Person 2: Lines 56-182 (CtrlHandler, process management, list/kill/stop/resume)
 * - Person 3: Lines 218-254 (cd, pwd, dir, date, time, cls, help, exit, path, addpath)
 * - Person 4: Lines 257-319 (msh_launch - CreateProcess, foreground/background)
 */

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>

#define MAX_BG_PROCS 100
#define MAX_CMD_LEN 1024

/*============================================================
 * PERSON 2: Cấu trúc dữ liệu quản lý tiến trình
 *============================================================*/
typedef struct {
    DWORD pid;          // Process ID
    HANDLE hProcess;    // Handle của process
    HANDLE hThread;     // Handle của main thread (để stop/resume)
    char cmd[MAX_CMD_LEN]; // Tên lệnh
    int is_active;      // 1: đang chạy, 0: đã xong
    int is_suspended;   // 1: đang tạm dừng
} ProcessInfo;

ProcessInfo bg_procs[MAX_BG_PROCS];
HANDLE hForegroundProcess = NULL;

/*============================================================
 * PERSON 3: Khai báo các lệnh builtin
 *============================================================*/
int msh_cd(char **args);
int msh_help(char **args);
int msh_exit(char **args);
int msh_pwd(char **args);
int msh_dir(char **args);
int msh_date(char **args);
int msh_time(char **args);
int msh_cls(char **args);
int msh_list(char **args);
int msh_kill(char **args);
int msh_stop(char **args);
int msh_resume(char **args);
int msh_path(char **args);
int msh_addpath(char **args);

char *builtin_str[] = {
  "cd", "help", "exit", "pwd", "dir", "date", "time", "cls",
  "list", "kill", "stop", "resume", "path", "addpath"
};

int (*builtin_func[])(char **) = {
  &msh_cd, &msh_help, &msh_exit, &msh_pwd, &msh_dir, &msh_date, &msh_time, &msh_cls,
  &msh_list, &msh_kill, &msh_stop, &msh_resume, &msh_path, &msh_addpath
};

int msh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*============================================================
 * PERSON 2: Xử lý tín hiệu Ctrl+C
 *============================================================*/
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT) {
        printf("\n");
        if (hForegroundProcess != NULL) {
            TerminateProcess(hForegroundProcess, 1);
            hForegroundProcess = NULL;
            printf("Terminated foreground process.\n");
        } else {
            printf("No foreground process. Type 'exit' to quit.\n");
            printf("msh> ");
        }
        return TRUE;
    }
    return FALSE;
}

/*============================================================
 * PERSON 2: Hàm quản lý tiến trình background
 *============================================================*/
void add_bg_process(DWORD pid, HANDLE hProc, HANDLE hThread, char *cmd) {
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (!bg_procs[i].is_active) {
            bg_procs[i].pid = pid;
            bg_procs[i].hProcess = hProc;
            bg_procs[i].hThread = hThread;
            strncpy(bg_procs[i].cmd, cmd, MAX_CMD_LEN - 1);
            bg_procs[i].cmd[MAX_CMD_LEN - 1] = '\0';
            bg_procs[i].is_active = 1;
            bg_procs[i].is_suspended = 0;
            return;
        }
    }
    printf("Process list full!\n");
}

void cleanup_zombies() {
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active) {
            DWORD exitCode;
            if (GetExitCodeProcess(bg_procs[i].hProcess, &exitCode)) {
                if (exitCode != STILL_ACTIVE) {
                    CloseHandle(bg_procs[i].hProcess);
                    CloseHandle(bg_procs[i].hThread);
                    bg_procs[i].is_active = 0;
                }
            }
        }
    }
}

/*============================================================
 * PERSON 2: Các lệnh quản lý process (list, kill, stop, resume)
 *============================================================*/
int msh_list(char **args) {
    (void)args;
    cleanup_zombies();
    printf("%-10s %-10s %-20s\n", "PID", "Status", "Command");
    printf("------------------------------------------\n");
    int count = 0;
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active) {
            printf("%-10lu %-10s %.20s\n",
                bg_procs[i].pid,
                bg_procs[i].is_suspended ? "Stopped" : "Running",
                bg_procs[i].cmd);
            count++;
        }
    }
    if (count == 0) printf("No background processes.\n");
    return 1;
}

int msh_kill(char **args) {
    if (args[1] == NULL) { printf("Usage: kill <pid>\n"); return 1; }
    DWORD targetPid = (DWORD)atoi(args[1]);
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active && bg_procs[i].pid == targetPid) {
            TerminateProcess(bg_procs[i].hProcess, 0);
            printf("Process %lu killed.\n", targetPid);
            return 1;
        }
    }
    printf("Process %lu not found.\n", targetPid);
    return 1;
}

int msh_stop(char **args) {
    if (args[1] == NULL) { printf("Usage: stop <pid>\n"); return 1; }
    DWORD targetPid = (DWORD)atoi(args[1]);
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active && bg_procs[i].pid == targetPid) {
            if (SuspendThread(bg_procs[i].hThread) != (DWORD)-1) {
                bg_procs[i].is_suspended = 1;
                printf("Process %lu stopped.\n", targetPid);
            } else {
                printf("Failed to stop process %lu.\n", targetPid);
            }
            return 1;
        }
    }
    printf("Process %lu not found.\n", targetPid);
    return 1;
}

int msh_resume(char **args) {
    if (args[1] == NULL) { printf("Usage: resume <pid>\n"); return 1; }
    DWORD targetPid = (DWORD)atoi(args[1]);
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active && bg_procs[i].pid == targetPid) {
            if (ResumeThread(bg_procs[i].hThread) != (DWORD)-1) {
                bg_procs[i].is_suspended = 0;
                printf("Process %lu resumed.\n", targetPid);
            } else {
                printf("Failed to resume process %lu.\n", targetPid);
            }
            return 1;
        }
    }
    printf("Process %lu not found.\n", targetPid);
    return 1;
}

/*============================================================
 * PERSON 3: Lệnh môi trường (path, addpath)
 *============================================================*/
int msh_path(char **args) {
    (void)args;
    char *pathVal = (char *)malloc(32768);
    if (!pathVal) { printf("Memory error\n"); return 1; }
    if (GetEnvironmentVariable("PATH", pathVal, 32768) > 0) {
        printf("PATH=%s\n", pathVal);
    } else {
        printf("Could not read PATH.\n");
    }
    free(pathVal);
    return 1;
}

int msh_addpath(char **args) {
    if (args[1] == NULL) { printf("Usage: addpath <dir>\n"); return 1; }
    char *oldPath = (char *)malloc(32768);
    char *newPath = (char *)malloc(32768 + MAX_PATH);
    if (!oldPath || !newPath) { printf("Memory error\n"); return 1; }
    GetEnvironmentVariable("PATH", oldPath, 32768);
    sprintf(newPath, "%s;%s", oldPath, args[1]);
    if (SetEnvironmentVariable("PATH", newPath)) {
        printf("Path updated.\n");
    } else {
        printf("Failed to update path.\n");
    }
    free(oldPath);
    free(newPath);
    return 1;
}

/*============================================================
 * PERSON 3: Các lệnh builtin cơ bản
 *============================================================*/
int msh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "msh: expected argument to \"cd\"\n");
    } else if (!SetCurrentDirectory(args[1])) {
        fprintf(stderr, "msh: cannot change directory\n");
    }
    return 1;
}

int msh_help(char **args) {
    (void)args;
    printf("\n");
    printf("================== MSH - Tiny Shell ==================\n");
    printf("  cd <dir>         Change directory\n");
    printf("  pwd              Print working directory\n");
    printf("  dir [path]       List directory contents\n");
    printf("  list             List background processes\n");
    printf("  kill <pid>       Terminate a process\n");
    printf("  stop <pid>       Suspend a process\n");
    printf("  resume <pid>     Resume a process\n");
    printf("  path             Show PATH variable\n");
    printf("  addpath <dir>    Add to PATH\n");
    printf("  date/time/cls    Utilities\n");
    printf("  <cmd> &          Run in background\n");
    printf("  Ctrl+C           Kill foreground process\n");
    printf("  exit             Exit shell\n");
    printf("======================================================\n");
    return 1;
}

int msh_exit(char **args) { (void)args; return 0; }

int msh_pwd(char **args) {
    (void)args;
    char cwd[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd);
    printf("%s\n", cwd);
    return 1;
}

int msh_dir(char **args) {
    char cmd[1024] = "dir ";
    if (args[1]) strcat(cmd, args[1]);
    system(cmd);
    return 1;
}

int msh_date(char **args) { (void)args; system("date /t"); return 1; }
int msh_time(char **args) { (void)args; system("time /t"); return 1; }
int msh_cls(char **args) { (void)args; system("cls"); return 1; }

/*============================================================
 * PERSON 4: Hàm chạy chương trình ngoài (QUAN TRỌNG NHẤT)
 *============================================================*/
int msh_launch(char **args) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char command[1024] = {0};
    int background = 0;
    int i = 0;

    // 1. Kiểm tra background mode (& ở cuối)
    while (args[i] != NULL) i++;
    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        background = 1;
        args[i-1] = NULL;
    }

    // 2. Xây dựng chuỗi lệnh (xử lý batch file)
    int is_bat = 0;
    if (strstr(args[0], ".bat") != NULL || strstr(args[0], ".BAT") != NULL) {
        strcat(command, "cmd /c ");
        is_bat = 1;
    }

    i = 0;
    while (args[i] != NULL) {
        if (i > 0 || is_bat) strcat(command, " ");
        strcat(command, args[i]);
        i++;
    }

    // 3. Khởi tạo structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // 4. Tạo process với CREATE_NEW_PROCESS_GROUP
    if (!CreateProcess(NULL, command, NULL, NULL, FALSE, 
                       CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "msh: command not found: %s\n", args[0]);
        return 1;
    }

    // 5. Xử lý theo mode
    if (background) {
        printf("[Started process %lu]\n", pi.dwProcessId);
        add_bg_process(pi.dwProcessId, pi.hProcess, pi.hThread, command);
    } else {
        hForegroundProcess = pi.hProcess;
        // Polling loop để handle Ctrl+C
        while (hForegroundProcess != NULL) {
            DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);
            if (waitResult == WAIT_OBJECT_0) break;
        }
        hForegroundProcess = NULL;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 1;
}

/*============================================================
 * PERSON 1: Dispatch lệnh
 *============================================================*/
int msh_execute(char **args) {
    if (args[0] == NULL) return 1;
    for (int i = 0; i < msh_num_builtins(); i++) {
        if (_stricmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return msh_launch(args);
}

/*============================================================
 * PERSON 1: Đọc và parse input
 *============================================================*/
#define MSH_RL_BUFSIZE 1024
#define MSH_TOK_BUFSIZE 64
#define MSH_TOK_DELIM " \t\r\n\a"

char *msh_read_line(void) {
    int bufsize = MSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) { fprintf(stderr, "Memory error\n"); exit(EXIT_FAILURE); }

    while (1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        buffer[position] = c;
        position++;
        
        // Xử lý buffer overflow
        if (position >= bufsize - 1) {
            bufsize += MSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) { fprintf(stderr, "Memory error\n"); exit(EXIT_FAILURE); }
        }
    }
}

char **msh_split_line(char *line) {
    int bufsize = MSH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) { fprintf(stderr, "Memory error\n"); exit(EXIT_FAILURE); }

    token = strtok(line, MSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += MSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) { fprintf(stderr, "Memory error\n"); exit(EXIT_FAILURE); }
        }

        token = strtok(NULL, MSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/*============================================================
 * PERSON 1: Vòng lặp chính của shell
 *============================================================*/
void msh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        cleanup_zombies();
        printf("msh> ");
        fflush(stdout);

        line = msh_read_line();
        args = msh_split_line(line);
        status = msh_execute(args);

        free(line);
        free(args);
    } while (status);
}

/*============================================================
 * MAIN - Entry point
 *============================================================*/
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    // Đăng ký Ctrl+C Handler
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    // Khởi tạo danh sách process
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        bg_procs[i].is_active = 0;
    }

    // Welcome message
    printf("\n");
    printf("==========================================\n");
    printf("  MSH - Tiny Shell for Windows\n");
    printf("  Type 'help' for available commands\n");
    printf("==========================================\n\n");

    // Chạy shell loop
    msh_loop();

    return EXIT_SUCCESS;
}
