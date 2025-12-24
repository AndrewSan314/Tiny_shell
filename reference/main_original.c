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
 * - Person 2: Lines 56-182 (CtrlHandler, process management,
 * list/kill/stop/resume)
 * - Person 3: Lines 218-254 (cd, pwd, dir, date, time, cls, help, exit, path,
 * addpath)
 * - Person 4: Lines 257-319 (msh_launch - CreateProcess, foreground/background)
 */

#define _CRT_SECURE_NO_WARNINGS
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tlhelp32.h>


#define MAX_BG_PROCS 100
#define MAX_CMD_LEN 1024

/*============================================================
 * PERSON 2: Cấu trúc dữ liệu quản lý tiến trình
 *============================================================*/
typedef struct {
  DWORD pid;             // Process ID
  HANDLE hProcess;       // Handle của process
  HANDLE hThread;        // Handle của main thread (để stop/resume)
  char cmd[MAX_CMD_LEN]; // Tên lệnh
  int is_active;         // 1: đang chạy, 0: đã xong
  int is_suspended;      // 1: đang tạm dừng
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

int msh_datetime(char **args);
int msh_cls(char **args);
int msh_list(char **args);
int msh_kill(char **args);
int msh_stop(char **args);
int msh_resume(char **args);
int msh_path(char **args);
int msh_addpath(char **args);
int msh_systeminfo(char **args);
int msh_grep(char **args);
int msh_search(char **args);
int msh_diff(char **args);

char *builtin_str[] = {"cd",       "help", "exit",    "pwd",        "dir",
                       "datetime", "cls",  "list",    "kill",       "stop",
                       "resume",   "path", "addpath", "systeminfo", "grep",
                       "search",   "diff"};

int (*builtin_func[])(char **) = {
    &msh_cd,       &msh_help, &msh_exit,    &msh_pwd,        &msh_dir,
    &msh_datetime, &msh_cls,  &msh_list,    &msh_kill,       &msh_stop,
    &msh_resume,   &msh_path, &msh_addpath, &msh_systeminfo, &msh_grep,
    &msh_search,   &msh_diff};

int msh_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }

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
      printf("%-10lu %-10s %.20s\n", bg_procs[i].pid,
             bg_procs[i].is_suspended ? "Stopped" : "Running", bg_procs[i].cmd);
      count++;
    }
  }
  if (count == 0)
    printf("No background processes.\n");
  return 1;
}

int msh_kill(char **args) {
  if (args[1] == NULL) {
    printf("Usage: kill <pid>\n");
    return 1;
  }
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
  if (args[1] == NULL) {
    printf("Usage: stop <pid>\n");
    return 1;
  }
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
  if (args[1] == NULL) {
    printf("Usage: resume <pid>\n");
    return 1;
  }
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
  if (!pathVal) {
    printf("Memory error\n");
    return 1;
  }
  if (GetEnvironmentVariable("PATH", pathVal, 32768) > 0) {
    printf("PATH=%s\n", pathVal);
  } else {
    printf("Could not read PATH.\n");
  }
  free(pathVal);
  return 1;
}

int msh_addpath(char **args) {
  if (args[1] == NULL) {
    printf("Use: addpath <dir1 dir2 dir3...dirn>\n");
    return 1;
  }
  int index = 1;
  while (args[index] != NULL) {
    char *oldPath = (char *)malloc(32768);
    char *newPath = (char *)malloc(32768 + MAX_PATH);
    if (!oldPath || !newPath) {
      printf("Memory error\n");
      return 1;
    }
    GetEnvironmentVariable("PATH", oldPath, 32768);
    sprintf(newPath, "%s;%s", oldPath, args[index]);
    if (!SetEnvironmentVariable("PATH", newPath)) {
      printf("Failed to update path %s.\n", args[index]);
    }
    free(oldPath);
    free(newPath);
    index++;
  }
  return 1;
}

/*============================================================
 * PERSON 3: Các lệnh builtin cơ bản
 *============================================================*/
int msh_cd(char **args) {
  if (args[1] == NULL) {
    printf("Use: cd <dir>\n");
  } else if (SetCurrentDirectory(args[1])) {
    printf("Directory changed to %s\n", args[1]);
  } else {
    printf("Failed to change directory");
  }
  return 1;
}

int msh_help(char **args) {
  (void)args;
  printf("\n");
  printf("================== MSH - Tiny Shell ==================\n");
  printf("  cd <dir>                           Change directory\n");
  printf("  pwd                                Print working directory\n");
  printf("  dir [path]                         List directory contents\n");
  printf("  list                               List background processes\n");
  printf("  kill <pid>                         Terminate a process\n");
  printf("  stop <pid>                         Suspend a process\n");
  printf("  resume <pid>                       Resume a process\n");
  printf("  path                               Show PATH variable\n");
  printf("  addpath <dir1 dir2 dir3...dirn>    Add to PATH\n");
  printf("  date/time/cls                      Utilities\n");
  printf("  <cmd> &                            Run in background\n");
  printf("  Ctrl+C                             Kill foreground process\n");
  printf("  exit                               Exit shell\n");
  printf("======================================================\n");
  return 1;
}

int msh_exit(char **args) {
  (void)args;
  return 0;
}

int msh_pwd(char **args) {
  (void)args;
  char currentwd[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, currentwd);
  printf("%s\n", currentwd);
  return 1;
}

int msh_dir(char **args) {
  char cmd[1024] = "dir ";
  if (args[1])
    strcat(cmd, args[1]);
  system(cmd);
  return 1;
}

int msh_datetime(char **args) {
  (void)args;
  system("echo %date% %time%");
  return 1;
}

int msh_cls(char **args) {
  (void)args;
  system("cls");
  return 1;
}

int msh_systeminfo(char **args) {
  (void)args;
  char computerName[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD compSize = sizeof(computerName);
  GetComputerName(computerName, &compSize);
  char userName[256];
  DWORD userSize = sizeof(userName);
  GetUserName(userName, &userSize);
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(memInfo);
  GlobalMemoryStatusEx(&memInfo);
  DWORDLONG totalRAM_MB = memInfo.ullTotalPhys / (1024 * 1024);
  DWORDLONG usedRAM_MB =
      (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024);
  DWORDLONG freeRAM_MB = memInfo.ullAvailPhys / (1024 * 1024);
  ULONGLONG uptimeSec = GetTickCount64() / 1000;
  int days = (int)(uptimeSec / 86400);
  int hours = (int)((uptimeSec % 86400) / 3600);
  int mins = (int)((uptimeSec % 3600) / 60);
  int percent = (int)memInfo.dwMemoryLoad;
  int barWidth = 20;
  int filled = (percent * barWidth) / 100;
  printf("\n");
  printf("+==================================================+\n");
  printf("|           SYSTEM INFORMATION                     |\n");
  printf("+==================================================+\n");
  printf("|  Computer : %-35s  \n", computerName);
  printf("|  User     : %-35s  \n", userName);
  printf("+--------------------------------------------------+\n");
  printf("|  RAM Used : %llu MB / %llu MB\n", usedRAM_MB, totalRAM_MB);
  printf("|  RAM Free : %llu MB\n", freeRAM_MB);
  printf("|  Memory   : [");
  for (int i = 0; i < barWidth; i++)
    printf(i < filled ? "#" : "-");
  printf("] %d%%\n", percent);
  printf("+--------------------------------------------------+\n");
  printf("|  Uptime   : %d days, %d hours, %d minutes\n", days, hours, mins);
  printf("+==================================================+\n\n");
  return 1;
}

int msh_grep(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    printf("Usage: grep <pattern> <filename>\n");
    printf("Example: grep error log.txt\n");
    return 1;
  }

  char *pattern = args[1];
  char *filename = args[2];

  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("grep: cannot open '%s'\n", filename);
    return 1;
  }
  char line[1024];
  int lineNum = 0;
  int matchCount = 0;

  printf("\nSearching for '%s' in %s:\n", pattern, filename);
  printf("================================================\n");

  while (fgets(line, sizeof(line), file)) {
    lineNum++;
    char *found = strstr(line, pattern);
    if (found) {
      matchCount++;
      line[strcspn(line, "\n")] = 0;
      printf("Line %d: %s\n", lineNum, line);
    }
  }
  printf("================================================\n");
  if (matchCount == 0) {
    printf("No matches found.\n");
  } else {
    printf("Found %d match(es).\n", matchCount);
  }
  printf("\n");
  fclose(file);
  return 1;
}

static int search_count = 0;
int match_pattern(const char *filename, const char *pattern) {
  if (strchr(pattern, '*') != NULL) {
    if (pattern[0] == '*') {
      const char *ext = pattern + 1;
      int extLen = strlen(ext);
      int nameLen = strlen(filename);
      if (nameLen >= extLen)
        return _stricmp(filename + nameLen - extLen, ext) == 0;
      return 0;
    } else {
      int prefixLen = strchr(pattern, '*') - pattern;
      return _strnicmp(filename, pattern, prefixLen) == 0;
    }
  }
  return _stricmp(filename, pattern) == 0;
}
void search_recursive(const char *basePath, const char *pattern) {
  WIN32_FIND_DATA findData;
  char searchPath[MAX_PATH], fullPath[MAX_PATH];
  sprintf(searchPath, "%s\\*", basePath);
  HANDLE hFind = FindFirstFile(searchPath, &findData);
  if (hFind == INVALID_HANDLE_VALUE)
    return;
  do {
    if (strcmp(findData.cFileName, ".") == 0 ||
        strcmp(findData.cFileName, "..") == 0)
      continue;
    sprintf(fullPath, "%s\\%s", basePath, findData.cFileName);
    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      search_recursive(fullPath, pattern);
    } else {
      if (match_pattern(findData.cFileName, pattern))
        printf("  [%d] %s\n", ++search_count, fullPath);
    }
  } while (FindNextFile(hFind, &findData));
  FindClose(hFind);
}
int msh_search(char **args) {
  if (args[1] == NULL) {
    printf("Usage: search <filename or pattern>\n");
    printf("  search readme.txt | search *.c | search test*\n");
    return 1;
  }
  char startPath[MAX_PATH];
  if (args[2] != NULL)
    strcpy(startPath, args[2]);
  else
    GetCurrentDirectory(MAX_PATH, startPath);
  search_count = 0;
  printf("\nSearching for '%s' in %s...\n", args[1], startPath);
  printf("================================================\n");
  search_recursive(startPath, args[1]);
  printf("================================================\n");
  printf(search_count ? "Found %d file(s)\n\n" : "No files found.\n\n",
         search_count);
  return 1;
}

int msh_diff(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    printf("Usage: diff <file1> <file2>\n");
    return 1;
  }

  FILE *file1 = fopen(args[1], "r");
  FILE *file2 = fopen(args[2], "r");

  if (!file1) {
    printf("diff: cannot open '%s'\n", args[1]);
    if (file2)
      fclose(file2);
    return 1;
  }
  if (!file2) {
    printf("diff: cannot open '%s'\n", args[2]);
    fclose(file1);
    return 1;
  }

  char line1[1024], line2[1024];
  int lineNum = 0;
  int diffCount = 0;

  printf("\nComparing '%s' and '%s':\n", args[1], args[2]);
  printf("================================================\n");

  while (1) {
    char *r1 = fgets(line1, sizeof(line1), file1);
    char *r2 = fgets(line2, sizeof(line2), file2);
    lineNum++;

    /* Cả 2 file đều hết */
    if (!r1 && !r2)
      break;

    /* File 1 hết trước */
    if (!r1) {
      line2[strcspn(line2, "\n")] = 0;
      printf("Line %d:\n", lineNum);
      printf("  < (end of file)\n");
      printf("  > %s\n", line2);
      diffCount++;
      continue;
    }

    /* File 2 hết trước */
    if (!r2) {
      line1[strcspn(line1, "\n")] = 0;
      printf("Line %d:\n", lineNum);
      printf("  < %s\n", line1);
      printf("  > (end of file)\n");
      diffCount++;
      continue;
    }

    /* So sánh 2 dòng */
    if (strcmp(line1, line2) != 0) {
      line1[strcspn(line1, "\n")] = 0;
      line2[strcspn(line2, "\n")] = 0;
      printf("Line %d:\n", lineNum);
      printf("  < %s\n", line1);
      printf("  > %s\n", line2);
      diffCount++;
    }
  }

  printf("================================================\n");
  if (diffCount == 0) {
    printf("Files are identical.\n");
  } else {
    printf("Found %d difference(s).\n", diffCount);
  }
  printf("\n");

  fclose(file1);
  fclose(file2);
  return 1;
}
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
  while (args[i] != NULL)
    i++;
  if (i > 0 && strcmp(args[i - 1], "&") == 0) {
    background = 1;
    args[i - 1] = NULL;
  }

  // 2. Xây dựng chuỗi lệnh (xử lý batch file)
  int is_bat = 0;
  if (strstr(args[0], ".bat") != NULL || strstr(args[0], ".BAT") != NULL) {
    strcat(command, "cmd /c ");
    is_bat = 1;
  }

  i = 0;
  while (args[i] != NULL) {
    if (i > 0 || is_bat)
      strcat(command, " ");
    strcat(command, args[i]);
    i++;
  }

  // 3. Khởi tạo structures
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  // 4. Tạo process với CREATE_NEW_PROCESS_GROUP
  if (!CreateProcess(NULL, command, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP,
                     NULL, NULL, &si, &pi)) {
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
      if (waitResult == WAIT_OBJECT_0)
        break;
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
  if (args[0] == NULL)
    return 1;
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

  if (!buffer) {
    fprintf(stderr, "Memory error\n");
    exit(EXIT_FAILURE);
  }

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
      if (!buffer) {
        fprintf(stderr, "Memory error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **msh_split_line(char *line) {
  int bufsize = MSH_TOK_BUFSIZE;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "Memory error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, MSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += MSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "Memory error\n");
        exit(EXIT_FAILURE);
      }
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
