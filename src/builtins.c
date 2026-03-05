#include "builtins.h"
#include "ai.h"
#include "colors.h"
#include "core.h"
#include "hacker.h"
#include "process_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *builtin_str[] = {"cd",     "pwd",  "dir",    "datetime",   "cls",
                       "help",   "exit", "path",   "addpath",    "list",
                       "kill",   "stop", "resume", "systeminfo", "grep",
                       "search", "diff", "demo",   "aimode",     "ai"};

int (*builtin_func[])(char **) = {
    &msh_cd,     &msh_pwd,  &msh_dir,    &msh_datetime,   &msh_cls,
    &msh_help,   &msh_exit, &msh_path,   &msh_addpath,    &msh_list,
    &msh_kill,   &msh_stop, &msh_resume, &msh_systeminfo, &msh_grep,
    &msh_search, &msh_diff, &msh_demo,   &msh_aimode,     &msh_ai};

int msh_num_builtins(void) { return sizeof(builtin_str) / sizeof(char *); }

static void demo_separator(void) {
  set_color(CLR_MUTED);
  printf("  --------------------------------------------------\n");
  reset_color();
}

static void demo_run_command(const char *cmd, int pauseMs) {
  Sleep(500); /* pause before typing */
  set_color(CLR_BRIGHT_GREEN);
  printf("  msh> ");
  set_color(CLR_BRIGHT_WHITE);
  hacker_type(cmd, 40);
  reset_color();
  printf("\n");

  msh_execute_line(cmd);
  if (pauseMs > 0)
    Sleep(pauseMs);
}

int msh_path(char **args) {
  (void)args;
  char *pathVal = (char *)malloc(32768);
  if (!pathVal) {
    print_error("Memory allocation failed");
    return MSH_CONTINUE;
  }
  if (GetEnvironmentVariable("PATH", pathVal, 32768) > 0) {
    /* Split PATH by semicolons and display each entry */
    set_color(CLR_HEADER);
    printf("\n  PATH entries:\n");
    set_color(CLR_MUTED);
    printf("  ──────────────────────────────────────\n");
    reset_color();

    char *entry = strtok(pathVal, ";");
    int index = 1;
    while (entry != NULL) {
      set_color(CLR_ACCENT);
      printf("  %3d ", index++);
      set_color(CLR_BRIGHT_CYAN);
      printf("%s\n", entry);
      reset_color();
      entry = strtok(NULL, ";");
    }
    printf("\n");
  } else {
    print_error("Could not read PATH");
  }
  free(pathVal);
  return MSH_CONTINUE;
}

int msh_addpath(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: addpath <dir1 dir2 dir3...dirn>");
    return MSH_CONTINUE;
  }
  int index = 1;
  while (args[index] != NULL) {
    char *oldPath = (char *)malloc(32768);
    char *newPath = (char *)malloc(32768 + MAX_PATH);
    if (!oldPath || !newPath) {
      print_error("Memory allocation failed");
      return MSH_CONTINUE;
    }
    GetEnvironmentVariable("PATH", oldPath, 32768);
    sprintf(newPath, "%s;%s", oldPath, args[index]);
    if (!SetEnvironmentVariable("PATH", newPath)) {
      char msg[512];
      sprintf(msg, "Failed to add path: %s", args[index]);
      print_error(msg);
    } else {
      char msg[512];
      sprintf(msg, "Added to PATH: %s", args[index]);
      print_success(msg);
    }
    free(oldPath);
    free(newPath);
    index++;
  }
  return MSH_CONTINUE;
}

int msh_cd(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: cd <directory>");
  } else if (SetCurrentDirectory(args[1])) {
    char msg[512];
    sprintf(msg, "Changed to: %s", args[1]);
    print_success(msg);
  } else {
    char msg[512];
    sprintf(msg, "Cannot access directory: %s", args[1]);
    print_error(msg);
  }
  return MSH_CONTINUE;
}

int msh_help(char **args) {
  (void)args;
  hacker_scan("Accessing command registry", 300);
  printf("\n");
  set_color(CLR_HEADER);
  printf("  ╔══════════════════════════════════════════════════╗\n");
  printf("  ║          MSH - Modern Shell for Windows          ║\n");
  printf("  ╚══════════════════════════════════════════════════╝\n");
  reset_color();
  printf("\n");

  /* Navigation */
  set_color(CLR_BRIGHT_YELLOW);
  printf("  NAVIGATION\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  set_color(CLR_BRIGHT_CYAN);
  printf("  cd <dir>           ");
  reset_color();
  printf("Change directory\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  pwd                ");
  reset_color();
  printf("Print working directory\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  dir [path]         ");
  reset_color();
  printf("List directory contents\n");
  printf("\n");

  /* File Operations */
  set_color(CLR_BRIGHT_YELLOW);
  printf("  FILE OPERATIONS\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  set_color(CLR_BRIGHT_CYAN);
  printf("  grep <pat> <file>  ");
  reset_color();
  printf("Search pattern in file\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  search <filename>  ");
  reset_color();
  printf("Search file recursively\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  diff <f1> <f2>     ");
  reset_color();
  printf("Compare two files\n");
  printf("\n");

  /* System */
  set_color(CLR_BRIGHT_YELLOW);
  printf("  SYSTEM\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  set_color(CLR_BRIGHT_CYAN);
  printf("  systeminfo         ");
  reset_color();
  printf("Show system information\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  datetime           ");
  reset_color();
  printf("Show date and time\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  path               ");
  reset_color();
  printf("Show PATH entries\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  addpath <dirs>     ");
  reset_color();
  printf("Add directories to PATH\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  cls                ");
  reset_color();
  printf("Clear screen\n");
  printf("\n");

  /* Process Management */
  set_color(CLR_BRIGHT_YELLOW);
  printf("  PROCESS MANAGEMENT\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  set_color(CLR_BRIGHT_CYAN);
  printf("  <cmd> &            ");
  reset_color();
  printf("Run command in background\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  list               ");
  reset_color();
  printf("List background processes\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  kill <pid>         ");
  reset_color();
  printf("Terminate a process\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  stop <pid>         ");
  reset_color();
  printf("Suspend a process\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  resume <pid>       ");
  reset_color();
  printf("Resume a process\n");
  printf("\n");

  /* File Utilities (new) */
  set_color(CLR_BRIGHT_YELLOW);
  printf("  FILE UTILITIES\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  set_color(CLR_BRIGHT_CYAN);
  printf("  cat <file>         ");
  reset_color();
  printf("Display file with line numbers\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  head <file> [n]    ");
  reset_color();
  printf("Show first N lines\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  tail <file> [n]    ");
  reset_color();
  printf("Show last N lines\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  wc <file>          ");
  reset_color();
  printf("Count lines/words/chars\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  touch <file>       ");
  reset_color();
  printf("Create file / update timestamp\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  cp <src> <dst>     ");
  reset_color();
  printf("Copy a file\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  mv <src> <dst>     ");
  reset_color();
  printf("Move or rename a file\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  rm <file>          ");
  reset_color();
  printf("Delete a file\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  mkdir <dir>        ");
  reset_color();
  printf("Create a directory\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  tree [path] [-d n] [-s ms] ");
  reset_color();
  printf("Directory tree (use -s <ms> to control speed)\n");
  printf("\n");

  /* Calculator & Themes */
  set_color(CLR_BRIGHT_YELLOW);
  printf("  EXTRAS\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  set_color(CLR_BRIGHT_CYAN);
  printf("  calc <expr>        ");
  reset_color();
  printf("Built-in calculator\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  color <theme>      ");
  reset_color();
  printf("Switch color theme\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  source <file>      ");
  reset_color();
  printf("Run script file\n");
  printf("\n");

  /* Environment */
  set_color(CLR_BRIGHT_YELLOW);
  printf("  ENVIRONMENT\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  set_color(CLR_BRIGHT_CYAN);
  printf("  export VAR=value   ");
  reset_color();
  printf("Set environment variable\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  unset VAR          ");
  reset_color();
  printf("Remove environment variable\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  env                ");
  reset_color();
  printf("List all env variables\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  echo <text>        ");
  reset_color();
  printf("Print text ($VAR expanded)\n");
  printf("\n");

  /* History & Aliases */
  set_color(CLR_BRIGHT_YELLOW);
  printf("  HISTORY & ALIASES\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  set_color(CLR_BRIGHT_CYAN);
  printf("  history            ");
  reset_color();
  printf("Show command history\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  !!                 ");
  reset_color();
  printf("Repeat last command\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  !n                 ");
  reset_color();
  printf("Repeat command #n\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  alias name=cmd     ");
  reset_color();
  printf("Create an alias\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  unalias name       ");
  reset_color();
  printf("Remove an alias\n");
  printf("\n");

  /* I/O & Shell */
  set_color(CLR_BRIGHT_YELLOW);
  printf("  SHELL & I/O\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  set_color(CLR_BRIGHT_CYAN);
  printf("  cmd1 | cmd2        ");
  reset_color();
  printf("Pipe output between commands\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  cmd > file         ");
  reset_color();
  printf("Redirect output to file\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  cmd >> file        ");
  reset_color();
  printf("Append output to file\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  cmd < file         ");
  reset_color();
  printf("Read input from file\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  whoami             ");
  reset_color();
  printf("Show current user\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  hostname           ");
  reset_color();
  printf("Show computer name\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  uptime             ");
  reset_color();
  printf("Show system uptime\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  cls / clear        ");
  reset_color();
  printf("Clear screen\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  help               ");
  reset_color();
  printf("Show this help message\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  demo [epic]        ");
  reset_color();
  printf("Run cinematic showcase (epic = max FX)\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  aimode on|off      ");
  reset_color();
  printf("Toggle AI chat mode\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  ai <message>       ");
  reset_color();
  printf("Single AI request\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  !<command>         ");
  reset_color();
  printf("Run shell command while AI mode is ON\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  exit               ");
  reset_color();
  printf("Exit MSH shell\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  Ctrl+C             ");
  reset_color();
  printf("Kill foreground process\n");
  printf("\n");

  set_color(CLR_MUTED);
  printf("  Use Up/Down arrows to navigate history, Tab to autocomplete\n");
  reset_color();
  printf("\n");

  return MSH_CONTINUE;
}

int msh_demo(char **args) {
  int epic = 0;
  if (args[1] && _stricmp(args[1], "epic") == 0)
    epic = 1;

  system("cls");
  printf("\n");
  hacker_glitch_reveal(epic ? "  MSH EPIC DEMO MODE" : "  MSH LIVE DEMO MODE",
                       CLR_BRIGHT_GREEN, epic ? 14 : 10);
  set_color(CLR_INFO);
  printf("  Running scripted showcase for a clean 2-minute recording%s.\n",
         epic ? " (EPIC)" : "");
  reset_color();
  demo_separator();

  hacker_progress_bar("Preparing cinematic mode", epic ? 800 : 600);
  hacker_scan("Checking command modules", epic ? 900 : 600);
  if (epic) {
    set_color(CLR_BRIGHT_GREEN);
    printf("  [DATA] Live binary stream:\n");
    reset_color();
    hacker_binary_stream(192, 3);
  }
  hacker_access_granted();
  printf("\n");
  demo_separator();

  /* === Section 1: Navigation & System === */
  set_color(CLR_HEADER);
  printf("\n  [ NAVIGATION & SYSTEM ]\n");
  reset_color();
  demo_run_command("pwd", 800);
  demo_run_command("datetime", 800);
  demo_run_command("dir", 1000);
  demo_run_command("systeminfo", 1200);
  demo_separator();
  Sleep(800);

  /* === Section 2: File Operations === */
  set_color(CLR_HEADER);
  printf("\n  [ FILE OPERATIONS ]\n");
  reset_color();
  demo_run_command("tree src -d 1", 1200);
  demo_run_command("search *.c src", 1000);
  demo_run_command("grep msh src\\main.c", 1000);
  demo_run_command("wc src\\main.c", 800);
  demo_run_command("head src\\main.c 5", 800);
  demo_separator();
  Sleep(800);

  /* === Section 3: Calculator === */
  set_color(CLR_HEADER);
  printf("\n  [ BUILT-IN CALCULATOR ]\n");
  reset_color();
  demo_run_command("calc 2+3*4", 1000);
  demo_run_command("calc (10-3)/2", 1000);
  demo_run_command("calc 3.14*5*5", 1000);
  demo_separator();
  Sleep(800);

  /* === Section 4: Aliases & Environment === */
  set_color(CLR_HEADER);
  printf("\n  [ ALIASES & ENVIRONMENT ]\n");
  reset_color();
  demo_run_command("alias ll=dir", 800);
  demo_run_command("alias", 800);
  demo_run_command("export GREETING=Hello_from_MSH", 800);
  demo_run_command("echo $GREETING", 800);
  demo_run_command("unalias ll", 600);
  demo_run_command("unset GREETING", 600);
  demo_separator();
  Sleep(800);

  /* === Section 5: Pipe & Redirect === */
  set_color(CLR_HEADER);
  printf("\n  [ PIPE & REDIRECT ]\n");
  reset_color();
  demo_run_command("echo Hello World > test_demo.txt", 800);
  demo_run_command("cat test_demo.txt", 800);
  demo_run_command("echo MSH is awesome >> test_demo.txt", 800);
  demo_run_command("cat test_demo.txt", 800);
  demo_run_command("rm test_demo.txt", 600);
  demo_separator();
  Sleep(800);

  /* === Section 6: Color Themes === */
  set_color(CLR_HEADER);
  printf("\n  [ COLOR THEMES ]\n");
  reset_color();
  demo_run_command("color", 1200);
  demo_run_command("color ocean", 1500);
  demo_run_command("dir", 1200);
  demo_run_command("color sunset", 1500);
  demo_run_command("calc 42*42", 1200);
  demo_run_command("color matrix", 1200);
  demo_separator();
  Sleep(800);

  /* === Section 7: Help & History === */
  set_color(CLR_HEADER);
  printf("\n  [ COMMANDS OVERVIEW ]\n");
  reset_color();
  demo_run_command("help", 1500);
  demo_run_command("history", epic ? 1000 : 800);

  demo_separator();
  set_color(CLR_SUCCESS);
  printf("  Showcase complete. Continue freestyle and end with: exit\n");
  set_color(CLR_MUTED);
  printf("  Tip: run 'demo epic' for max visual intensity.\n");
  reset_color();
  printf("\n");

  return MSH_CONTINUE;
}

int msh_exit(char **args) {
  (void)args;
  hacker_scan("Terminating secure connection", 400);
  return MSH_EXIT;
}

int msh_pwd(char **args) {
  (void)args;
  char currentwd[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, currentwd);
  set_color(CLR_BRIGHT_CYAN);
  printf("%s\n", currentwd);
  reset_color();
  return MSH_CONTINUE;
}

int msh_dir(char **args) {
  char searchPath[MAX_PATH];
  WIN32_FIND_DATA findData;

  if (args[1])
    sprintf(searchPath, "%s\\*", args[1]);
  else
    strcpy(searchPath, "*");

  HANDLE hFind = FindFirstFile(searchPath, &findData);
  if (hFind == INVALID_HANDLE_VALUE) {
    print_error("Cannot list directory");
    return MSH_CONTINUE;
  }

  hacker_scan("Analyzing filesystem structures", 300);
  printf("\n");
  set_color(CLR_HEADER);
  printf("  Directory listing:\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();

  int fileCount = 0, dirCount = 0;

  do {
    if (strcmp(findData.cFileName, ".") == 0 ||
        strcmp(findData.cFileName, "..") == 0)
      continue;

    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      set_color(CLR_DIR_COLOR);
      printf("  [DIR]  %s\n", findData.cFileName);
      dirCount++;
    } else {
      /* Get file size */
      ULONGLONG fileSize =
          ((ULONGLONG)findData.nFileSizeHigh << 32) | findData.nFileSizeLow;

      /* Color executables differently */
      const char *dot = strrchr(findData.cFileName, '.');
      if (dot && (_stricmp(dot, ".exe") == 0 || _stricmp(dot, ".bat") == 0 ||
                  _stricmp(dot, ".cmd") == 0)) {
        set_color(CLR_EXE_COLOR);
      } else {
        set_color(CLR_FILE_COLOR);
      }

      if (fileSize < 1024) {
        printf("  %5lluB  %s\n", fileSize, findData.cFileName);
      } else if (fileSize < 1024 * 1024) {
        printf("  %5lluK  %s\n", fileSize / 1024, findData.cFileName);
      } else {
        printf("  %5lluM  %s\n", fileSize / (1024 * 1024), findData.cFileName);
      }
      fileCount++;
    }
    reset_color();
  } while (FindNextFile(hFind, &findData));

  FindClose(hFind);

  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  set_color(CLR_INFO);
  printf("  %d file(s), %d dir(s)\n\n", fileCount, dirCount);
  reset_color();

  return MSH_CONTINUE;
}

int msh_datetime(char **args) {
  (void)args;
  SYSTEMTIME st;
  GetLocalTime(&st);

  set_color(CLR_ACCENT);
  printf("  Date  ");
  reset_color();
  printf(" %04d-%02d-%02d\n", st.wYear, st.wMonth, st.wDay);
  set_color(CLR_ACCENT);
  printf("  Time  ");
  reset_color();
  printf(" %02d:%02d:%02d\n", st.wHour, st.wMinute, st.wSecond);

  return MSH_CONTINUE;
}

int msh_cls(char **args) {
  (void)args;
  system("cls");
  return MSH_CONTINUE;
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
  int barWidth = 30;
  int filled = (percent * barWidth) / 100;

  hacker_progress_bar("Probing memory sectors", 400);
  hacker_progress_bar("Interrogating system CPU", 300);
  printf("\n");
  set_color(CLR_HEADER);
  printf("  ╔══════════════════════════════════════════════════╗\n");
  printf("  ║             SYSTEM INFORMATION                   ║\n");
  printf("  ╚══════════════════════════════════════════════════╝\n");
  reset_color();
  printf("\n");

  set_color(CLR_ACCENT);
  printf("  Computer  ");
  reset_color();
  printf(" %s\n", computerName);

  set_color(CLR_ACCENT);
  printf("  User      ");
  reset_color();
  printf(" %s\n", userName);

  set_color(CLR_ACCENT);
  printf("  Uptime    ");
  reset_color();
  printf(" %d days, %d hours, %d minutes\n", days, hours, mins);

  printf("\n");
  set_color(CLR_ACCENT);
  printf("  RAM Used  ");
  reset_color();
  printf(" %llu / %llu MB\n", usedRAM_MB, totalRAM_MB);

  set_color(CLR_ACCENT);
  printf("  RAM Free  ");
  reset_color();
  printf(" %llu MB\n", freeRAM_MB);

  set_color(CLR_ACCENT);
  printf("  Memory    ");
  reset_color();
  printf(" [");
  set_color(percent > 80 ? CLR_ERROR
                         : (percent > 50 ? CLR_WARNING : CLR_SUCCESS));
  for (int i = 0; i < barWidth; i++)
    printf(i < filled ? "#" : " ");
  reset_color();
  printf("] %d%%\n\n", percent);

  return MSH_CONTINUE;
}

int msh_grep(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    print_info("Usage: grep <pattern> <filename>");
    return MSH_CONTINUE;
  }

  char *pattern = args[1];
  char *filename = args[2];

  FILE *file = fopen(filename, "r");
  if (!file) {
    char msg[512];
    sprintf(msg, "Cannot open file: %s", filename);
    print_error(msg);
    return MSH_CONTINUE;
  }
  char line[1024];
  int lineNum = 0;
  int matchCount = 0;

  printf("\n");
  set_color(CLR_HEADER);
  printf("  Searching for ");
  set_color(CLR_BRIGHT_YELLOW);
  printf("'%s'", pattern);
  set_color(CLR_HEADER);
  printf(" in ");
  set_color(CLR_BRIGHT_CYAN);
  printf("%s", filename);
  reset_color();
  printf("\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();

  while (fgets(line, sizeof(line), file)) {
    lineNum++;
    char *found = strstr(line, pattern);
    if (found) {
      matchCount++;
      line[strcspn(line, "\n")] = 0;

      /* Print line number in accent color */
      set_color(CLR_ACCENT);
      printf("  %4d: ", lineNum);

      /* Print line with match highlighted */
      char *pos = line;
      int patLen = (int)strlen(pattern);
      while (*pos) {
        char *match = strstr(pos, pattern);
        if (match) {
          /* Print text before match */
          reset_color();
          while (pos < match) {
            putchar(*pos++);
          }
          /* Print match in highlight */
          set_color(CLR_HIGHLIGHT);
          for (int k = 0; k < patLen; k++) {
            putchar(*pos++);
          }
          reset_color();
        } else {
          printf("%s", pos);
          break;
        }
      }
      printf("\n");
      reset_color();
    }
  }

  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  if (matchCount == 0) {
    print_info("No matches found.");
  } else {
    char msg[128];
    sprintf(msg, "Found %d match(es)", matchCount);
    print_success(msg);
  }
  printf("\n");
  fclose(file);
  return MSH_CONTINUE;
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
      if (match_pattern(findData.cFileName, pattern)) {
        set_color(CLR_ACCENT);
        printf("  [%d] ", ++search_count);
        set_color(CLR_BRIGHT_CYAN);
        printf("%s\n", fullPath);
        reset_color();
      }
    }
  } while (FindNextFile(hFind, &findData));
  FindClose(hFind);
}

int msh_search(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: search <filename or pattern>");
    printf("  Examples: search readme.txt | search *.c | search test*\n");
    return MSH_CONTINUE;
  }
  char startPath[MAX_PATH];
  if (args[2] != NULL)
    strcpy(startPath, args[2]);
  else
    GetCurrentDirectory(MAX_PATH, startPath);
  search_count = 0;

  printf("\n");
  set_color(CLR_HEADER);
  printf("  Searching for ");
  set_color(CLR_BRIGHT_YELLOW);
  printf("'%s'", args[1]);
  set_color(CLR_HEADER);
  printf(" in ");
  set_color(CLR_BRIGHT_CYAN);
  printf("%s", startPath);
  reset_color();
  printf("\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();

  search_recursive(startPath, args[1]);

  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  if (search_count) {
    char msg[128];
    sprintf(msg, "Found %d file(s)", search_count);
    print_success(msg);
  } else {
    print_info("No files found.");
  }
  printf("\n");
  return MSH_CONTINUE;
}

int msh_diff(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    print_info("Usage: diff <file1> <file2>");
    return MSH_CONTINUE;
  }

  FILE *file1 = fopen(args[1], "r");
  FILE *file2 = fopen(args[2], "r");

  if (!file1) {
    char msg[512];
    sprintf(msg, "Cannot open: %s", args[1]);
    print_error(msg);
    if (file2)
      fclose(file2);
    return MSH_CONTINUE;
  }
  if (!file2) {
    char msg[512];
    sprintf(msg, "Cannot open: %s", args[2]);
    print_error(msg);
    fclose(file1);
    return MSH_CONTINUE;
  }

  char line1[1024], line2[1024];
  int lineNum = 0;
  int diffCount = 0;

  printf("\n");
  set_color(CLR_HEADER);
  printf("  Comparing ");
  set_color(CLR_BRIGHT_CYAN);
  printf("'%s'", args[1]);
  set_color(CLR_HEADER);
  printf(" and ");
  set_color(CLR_BRIGHT_CYAN);
  printf("'%s'", args[2]);
  reset_color();
  printf("\n");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();

  while (1) {
    char *r1 = fgets(line1, sizeof(line1), file1);
    char *r2 = fgets(line2, sizeof(line2), file2);
    lineNum++;

    if (!r1 && !r2)
      break;

    if (!r1) {
      line2[strcspn(line2, "\n")] = 0;
      set_color(CLR_ACCENT);
      printf("  Line %d:\n", lineNum);
      set_color(CLR_ERROR);
      printf("    < (end of file)\n");
      set_color(CLR_SUCCESS);
      printf("    > %s\n", line2);
      reset_color();
      diffCount++;
      continue;
    }

    if (!r2) {
      line1[strcspn(line1, "\n")] = 0;
      set_color(CLR_ACCENT);
      printf("  Line %d:\n", lineNum);
      set_color(CLR_ERROR);
      printf("    < %s\n", line1);
      set_color(CLR_SUCCESS);
      printf("    > (end of file)\n");
      reset_color();
      diffCount++;
      continue;
    }

    if (strcmp(line1, line2) != 0) {
      line1[strcspn(line1, "\n")] = 0;
      line2[strcspn(line2, "\n")] = 0;
      set_color(CLR_ACCENT);
      printf("  Line %d:\n", lineNum);
      set_color(CLR_ERROR);
      printf("    < %s\n", line1);
      set_color(CLR_SUCCESS);
      printf("    > %s\n", line2);
      reset_color();
      diffCount++;
    }
  }

  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();
  if (diffCount == 0) {
    print_success("Files are identical");
  } else {
    char msg[128];
    sprintf(msg, "Found %d difference(s)", diffCount);
    print_warning(msg);
  }
  printf("\n");

  fclose(file1);
  fclose(file2);
  return MSH_CONTINUE;
}
