#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h> // Để lấy snapshot thread cho lệnh stop/resume

#define MAX_BG_PROCS 100
#define MAX_CMD_LEN 1024

// Cấu trúc để quản lý tiến trình chạy ngầm
typedef struct {
    DWORD pid;          // Process ID
    HANDLE hProcess;    // Handle của process
    HANDLE hThread;     // Handle của main thread (để stop/resume)
    char cmd[MAX_CMD_LEN]; // Tên lệnh
    int is_active;      // 1: đang chạy, 0: đã xong/trống
    int is_suspended;   // 1: đang tạm dừng, 0: đang chạy
} ProcessInfo;

ProcessInfo bg_procs[MAX_BG_PROCS]; // Danh sách tiến trình background
HANDLE hForegroundProcess = NULL;   // Handle của tiến trình đang chạy foreground (để Ctrl+C kill)

/* Khai báo hàm */
int msh_cd(char **args);
int msh_help(char **args);
int msh_exit(char **args);
int msh_pwd(char **args);
int msh_dir(char **args);
int msh_date(char **args);
int msh_time(char **args);
int msh_cls(char **args);
int msh_list(char **args);      // MỚI
int msh_kill(char **args);      // MỚI
int msh_stop(char **args);      // MỚI
int msh_resume(char **args);    // MỚI
int msh_path(char **args);      // MỚI
int msh_addpath(char **args);   // MỚI

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

/* Xử lý tín hiệu Ctrl+C */
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT) {
        // Thêm dòng này để debug
        printf("\n[DEBUG] Da nhan tin hieu Ctrl+C!\n");

        if (hForegroundProcess != NULL) {
            TerminateProcess(hForegroundProcess, 1);
            hForegroundProcess = NULL;
            printf("Terminated foreground process.\n");
        } else {
            printf("Khong co tien trinh foreground de tat. Go 'exit' de thoat.\n");
            // In lại dấu nhắc để người dùng đỡ hoang mang
            printf("msh> ");
        }
        return TRUE;
    }
    return FALSE;
}
/* Các hàm tiện ích quản lý tiến trình */
void add_bg_process(DWORD pid, HANDLE hProc, HANDLE hThread, char *cmd) {
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (!bg_procs[i].is_active) {
            bg_procs[i].pid = pid;
            bg_procs[i].hProcess = hProc;
            bg_procs[i].hThread = hThread;
            strncpy(bg_procs[i].cmd, cmd, MAX_CMD_LEN);
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
                    // Tiến trình đã kết thúc
                    CloseHandle(bg_procs[i].hProcess);
                    CloseHandle(bg_procs[i].hThread);
                    bg_procs[i].is_active = 0;
                }
            }
        }
    }
}

/* TRIỂN KHAI CÁC HÀM BUILTIN MỚI */

// List: In danh sách tiến trình
int msh_list(char **args) {
    cleanup_zombies(); // Dọn dẹp trước khi in
    printf("%-10s %-10s %-20s %s\n", "PID", "Status", "Command", "");
    printf("------------------------------------------------\n");
    int count = 0;
    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active) {
            printf("%-10lu %-10s %-20s\n",
                bg_procs[i].pid,
                bg_procs[i].is_suspended ? "Stopped" : "Running",
                bg_procs[i].cmd);
            count++;
        }
    }
    if (count == 0) printf("No background processes.\n");
    return 1;
}

// Kill: Diệt tiến trình
int msh_kill(char **args) {
    if (args[1] == NULL) { printf("Usage: kill <pid>\n"); return 1; }
    DWORD targetPid = (DWORD)atoi(args[1]);

    for (int i = 0; i < MAX_BG_PROCS; i++) {
        if (bg_procs[i].is_active && bg_procs[i].pid == targetPid) {
            TerminateProcess(bg_procs[i].hProcess, 0);
            printf("Process %lu killed.\n", targetPid);
            // Dọn dẹp sẽ được xử lý ở lần gọi list hoặc loop sau
            return 1;
        }
    }
    printf("Process %lu not found.\n", targetPid);
    return 1;
}

// Stop: Tạm dừng tiến trình (Suspend Thread)
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

// Resume: Tiếp tục tiến trình
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

// Path: Xem biến môi trường PATH
int msh_path(char **args) {
    char *pathVal = (char *)malloc(32768); // Path có thể rất dài
    if (GetEnvironmentVariable("PATH", pathVal, 32768) > 0) {
        printf("PATH=%s\n", pathVal);
    } else {
        printf("Could not read PATH.\n");
    }
    free(pathVal);
    return 1;
}

// Addpath: Thêm đường dẫn vào PATH (cho session hiện tại)
int msh_addpath(char **args) {
    if (args[1] == NULL) { printf("Usage: addpath <dir>\n"); return 1; }

    char *oldPath = (char *)malloc(32768);
    char *newPath = (char *)malloc(32768 + MAX_PATH);

    GetEnvironmentVariable("PATH", oldPath, 32768);
    sprintf(newPath, "%s;%s", oldPath, args[1]); // Nối thêm vào cuối

    if (SetEnvironmentVariable("PATH", newPath)) {
        printf("Path updated.\n");
    } else {
        printf("Failed to update path.\n");
    }

    free(oldPath);
    free(newPath);
    return 1;
}

/* Các hàm builtin cũ (giữ nguyên hoặc chỉnh sửa nhẹ) */
int msh_cd(char **args) {
  if (args[1] == NULL) fprintf(stderr, "msh: expected argument to \"cd\"\n");
  else if (!SetCurrentDirectory(args[1])) fprintf(stderr, "msh: cannot change dir\n");
  return 1;
}

int msh_help(char **args) {
  printf("MSH - Tiny Shell Features:\n");
  printf("  list             : List background processes\n");
  printf("  kill <pid>       : Kill a process\n");
  printf("  stop <pid>       : Suspend a process\n");
  printf("  resume <pid>     : Resume a process\n");
  printf("  path             : Show PATH variable\n");
  printf("  addpath <dir>    : Append to PATH\n");
  printf("  cmd &            : Run in background\n");
  printf("  program.bat      : Run batch file\n");
  printf("  Ctrl+C           : Kill foreground process\n");
  printf("  (Plus: cd, pwd, dir, date, time, cls, exit)\n");
  return 1;
}
int msh_exit(char **args) { return 0; }
int msh_pwd(char **args) {
    char cwd[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd);
    printf("%s\n", cwd);
    return 1;
}
int msh_dir(char **args) {
    // (Giữ nguyên logic dir từ code trước của bạn hoặc dùng system("dir") cho nhanh)
    char cmd[1024] = "dir ";
    if (args[1]) strcat(cmd, args[1]);
    system(cmd);
    return 1;
}
int msh_date(char **args) { system("date /t"); return 1; }
int msh_time(char **args) { system("time /t"); return 1; }
int msh_cls(char **args) { system("cls"); return 1; }

/* HÀM LAUNCH - QUAN TRỌNG NHẤT: Xử lý Background & Batch file */
int msh_launch(char **args) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char command[1024] = {0};
    int background = 0;
    int i = 0;

    // 1. Kiểm tra Background mode (& ở cuối)
    while (args[i] != NULL) i++;
    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        background = 1;
        args[i-1] = NULL; // Xóa dấu & khỏi tham số
    }

    // 2. Xây dựng chuỗi lệnh
    // Đặc biệt: Nếu chạy file .bat, cần gọi cmd /c
    int is_bat = 0;
    if (strstr(args[0], ".bat") != NULL || strstr(args[0], ".BAT") != NULL) {
        strcat(command, "cmd /c ");
        is_bat = 1;
    }

    i = 0;
    while (args[i] != NULL) {
        if (i > 0 || is_bat) strcat(command, " "); // Thêm dấu cách
        strcat(command, args[i]);
        i++;
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "msh: command not found: %s\n", args[0]);
        return 1;
    }

    if (background) {
        // BACKGROUND MODE: Không đợi, thêm vào danh sách
        printf("[Started process %lu]\n", pi.dwProcessId);
        add_bg_process(pi.dwProcessId, pi.hProcess, pi.hThread, command);
    } else {
        // FOREGROUND MODE: Đợi
        hForegroundProcess = pi.hProcess; // Lưu handle để Ctrl+C bắt được
        WaitForSingleObject(pi.hProcess, INFINITE);
        hForegroundProcess = NULL;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 1;
}

int msh_execute(char **args) {
  if (args[0] == NULL) return 1;
  for (int i = 0; i < msh_num_builtins(); i++) {
    if (_stricmp(args[0], builtin_str[i]) == 0) return (*builtin_func[i])(args);
  }
  return msh_launch(args);
}

// (Giữ nguyên msh_read_line và msh_split_line như cũ)
char *msh_read_line(void) {
  char *line = NULL; size_t bufsize = 0;
  // Code read_line cũ của bạn ở đây.
  // Để ngắn gọn tôi viết lại bản đơn giản dùng gets/fgets hoặc giữ code cũ của bạn
  // Ở đây tôi giả định bạn giữ hàm cũ.
  #define MSH_RL_BUFSIZE 1024
  int position = 0;
  char *buffer = malloc(sizeof(char) * MSH_RL_BUFSIZE);
  int c;
  if (!buffer) exit(EXIT_FAILURE);
  while (1) {
    c = getchar();
    if (c == EOF || c == '\n') { buffer[position] = '\0'; return buffer; }
    buffer[position] = c; position++;
  }
}

char **msh_split_line(char *line) {
  // Code split_line cũ của bạn
  #define MSH_TOK_BUFSIZE 64
  #define MSH_TOK_DELIM " \t\r\n\a"
  int bufsize = MSH_TOK_BUFSIZE;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;
  token = strtok(line, MSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token; position++;
    token = strtok(NULL, MSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void msh_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    cleanup_zombies(); // Dọn dẹp tiến trình background đã xong trước khi in prompt

    char cwd[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd);
    printf("msh>");

    line = msh_read_line();
    args = msh_split_line(line);
    status = msh_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  // Đăng ký Ctrl+C Handler
  if (SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
      // Thành công
  } else {
      fprintf(stderr, "Error setting control handler");
  }

  // Khởi tạo danh sách process
  for(int i=0; i<MAX_BG_PROCS; i++) bg_procs[i].is_active = 0;

  msh_loop();
  return EXIT_SUCCESS;
}
