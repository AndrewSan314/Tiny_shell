/*
 * builtins.c - Built-in Commands Module Implementation
 * Person 3: Các lệnh nội bộ của shell
 */

#include "builtins.h"
#include "process_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*============================================================
 * BUILTIN REGISTRY
 *============================================================*/

char *builtin_str[] = {"cd",     "pwd",  "dir",    "datetime",   "cls",
                       "help",   "exit", "path",   "addpath",    "list",
                       "kill",   "stop", "resume", "systeminfo", "grep",
                       "search", "diff"};

int (*builtin_func[])(char **) = {
    &msh_cd,     &msh_pwd,  &msh_dir,    &msh_datetime,   &msh_cls,
    &msh_help,   &msh_exit, &msh_path,   &msh_addpath,    &msh_list,
    &msh_kill,   &msh_stop, &msh_resume, &msh_systeminfo, &msh_grep,
    &msh_search, &msh_diff};

int msh_num_builtins(void) { return sizeof(builtin_str) / sizeof(char *); }

/*============================================================
 * PERSON 3: Lệnh môi trường (path, addpath)
 *============================================================*/
int msh_path(char **args) {
  (void)args;
  char *pathVal = (char *)malloc(32768);
  if (!pathVal) {
    printf("Memory error\n");
    return MSH_CONTINUE;
  }
  if (GetEnvironmentVariable("PATH", pathVal, 32768) > 0) {
    printf("PATH=%s\n", pathVal);
  } else {
    printf("Could not read PATH.\n");
  }
  free(pathVal);
  return MSH_CONTINUE;
}

int msh_addpath(char **args) {
  if (args[1] == NULL) {
    printf("Use: addpath <dir1 dir2 dir3...dirn>\n");
    return MSH_CONTINUE;
  }
  int index = 1;
  while (args[index] != NULL) {
    char *oldPath = (char *)malloc(32768);
    char *newPath = (char *)malloc(32768 + MAX_PATH);
    if (!oldPath || !newPath) {
      printf("Memory error\n");
      return MSH_CONTINUE;
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
  return MSH_CONTINUE;
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
  return MSH_CONTINUE;
}

int msh_help(char **args) {
  (void)args;
  printf("\n");
  printf("================== MSH - Tiny Shell ==================\n");
  printf("  addpath <dir1 dir2 dir3...dirn>    Add to PATH\n");
  printf("  cd <dir>                           Change directory\n");
  printf("  cls                                Clear screen\n");
  printf("  datetime                           Show date and time\n");
  printf(
      "  diff <file1> <file2>               Show difference between files\n");
  printf("  dir [path]                         List directory contents\n");
  printf("  exit                               Exit shell\n");
  printf("  grep <pattern> <file>              Search pattern in file\n");
  printf("  help                               Show this help message\n");
  printf("  kill <pid>                         Terminate a process\n");
  printf("  list                               List background processes\n");
  printf("  path                               Show PATH variable\n");
  printf("  pwd                                Print working directory\n");
  printf("  resume <pid>                       Resume a process\n");
  printf("  search <filename>                  Search file\n");
  printf("  stop <pid>                         Suspend a process\n");
  printf("  systeminfo                         Show system information\n");
  printf("  <cmd> &                            Run in background\n");
  printf("  Ctrl+C                             Kill foreground process\n");
  printf("======================================================\n");
  return MSH_CONTINUE;
}

int msh_exit(char **args) {
  (void)args;
  return MSH_EXIT;
}

int msh_pwd(char **args) {
  (void)args;
  char currentwd[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, currentwd);
  printf("%s\n", currentwd);
  return MSH_CONTINUE;
}

int msh_dir(char **args) {
  char cmd[1024] = "dir ";
  if (args[1])
    strcat(cmd, args[1]);
  system(cmd);
  return MSH_CONTINUE;
}

int msh_datetime(char **args) {
  (void)args;
  system("echo %date% %time%");
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
  return MSH_CONTINUE;
}

int msh_grep(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    printf("Usage: grep <pattern> <filename>\n");
    printf("Example: grep error log.txt\n");
    return MSH_CONTINUE;
  }

  char *pattern = args[1];
  char *filename = args[2];

  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("grep: cannot open '%s'\n", filename);
    return MSH_CONTINUE;
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
    return MSH_CONTINUE;
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
  return MSH_CONTINUE;
}

int msh_diff(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    printf("Usage: diff <file1> <file2>\n");
    return MSH_CONTINUE;
  }

  FILE *file1 = fopen(args[1], "r");
  FILE *file2 = fopen(args[2], "r");

  if (!file1) {
    printf("diff: cannot open '%s'\n", args[1]);
    if (file2)
      fclose(file2);
    return MSH_CONTINUE;
  }
  if (!file2) {
    printf("diff: cannot open '%s'\n", args[2]);
    fclose(file1);
    return MSH_CONTINUE;
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
  return MSH_CONTINUE;
}
