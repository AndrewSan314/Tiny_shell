#include "utils.h"
#include "colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


/*============================================================
 * FILE UTILITIES
 *============================================================*/

int msh_cat(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: cat <file1> [file2] ...");
    return MSH_CONTINUE;
  }

  for (int f = 1; args[f] != NULL; f++) {
    FILE *file = fopen(args[f], "r");
    if (!file) {
      char msg[512];
      sprintf(msg, "Cannot open: %s", args[f]);
      print_error(msg);
      continue;
    }

    char line[4096];
    int lineNum = 1;

    /* Show filename header when multiple files */
    if (args[2] != NULL) {
      set_color(CLR_HEADER);
      printf("\n  === %s ===\n", args[f]);
      reset_color();
    }

    while (fgets(line, sizeof(line), file)) {
      set_color(CLR_MUTED);
      printf("%4d ", lineNum++);
      reset_color();
      printf("%s", line);
    }

    /* Ensure newline at end */
    if (line[strlen(line) - 1] != '\n')
      printf("\n");
    fclose(file);
  }
  return MSH_CONTINUE;
}

int msh_touch(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: touch <file1> [file2] ...");
    return MSH_CONTINUE;
  }

  for (int i = 1; args[i] != NULL; i++) {
    HANDLE hFile = CreateFile(args[i], GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
      /* Update the file's timestamp */
      FILETIME ft;
      SYSTEMTIME st;
      GetSystemTime(&st);
      SystemTimeToFileTime(&st, &ft);
      SetFileTime(hFile, NULL, NULL, &ft);
      CloseHandle(hFile);

      char msg[512];
      sprintf(msg, "Touched: %s", args[i]);
      print_success(msg);
    } else {
      char msg[512];
      sprintf(msg, "Cannot touch: %s", args[i]);
      print_error(msg);
    }
  }
  return MSH_CONTINUE;
}

int msh_rm(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: rm <file1> [file2] ...");
    return MSH_CONTINUE;
  }

  for (int i = 1; args[i] != NULL; i++) {
    if (DeleteFile(args[i])) {
      char msg[512];
      sprintf(msg, "Deleted: %s", args[i]);
      print_success(msg);
    } else {
      /* Try as directory */
      if (RemoveDirectory(args[i])) {
        char msg[512];
        sprintf(msg, "Removed directory: %s", args[i]);
        print_success(msg);
      } else {
        char msg[512];
        sprintf(msg, "Cannot remove: %s", args[i]);
        print_error(msg);
      }
    }
  }
  return MSH_CONTINUE;
}

int msh_cp(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    print_info("Usage: cp <source> <destination>");
    return MSH_CONTINUE;
  }

  if (CopyFile(args[1], args[2], FALSE)) {
    char msg[512];
    sprintf(msg, "Copied: %s -> %s", args[1], args[2]);
    print_success(msg);
  } else {
    char msg[512];
    sprintf(msg, "Failed to copy: %s", args[1]);
    print_error(msg);
  }
  return MSH_CONTINUE;
}

int msh_mv(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    print_info("Usage: mv <source> <destination>");
    return MSH_CONTINUE;
  }

  if (MoveFile(args[1], args[2])) {
    char msg[512];
    sprintf(msg, "Moved: %s -> %s", args[1], args[2]);
    print_success(msg);
  } else {
    char msg[512];
    sprintf(msg, "Failed to move: %s", args[1]);
    print_error(msg);
  }
  return MSH_CONTINUE;
}

int msh_head(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: head <file> [lines]");
    return MSH_CONTINUE;
  }

  int numLines = 10;
  if (args[2] != NULL) {
    numLines = atoi(args[2]);
    if (numLines <= 0)
      numLines = 10;
  }

  FILE *file = fopen(args[1], "r");
  if (!file) {
    char msg[512];
    sprintf(msg, "Cannot open: %s", args[1]);
    print_error(msg);
    return MSH_CONTINUE;
  }

  set_color(CLR_HEADER);
  printf("\n  First %d lines of %s:\n", numLines, args[1]);
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();

  char line[4096];
  int count = 0;
  while (count < numLines && fgets(line, sizeof(line), file)) {
    set_color(CLR_MUTED);
    printf("  %4d ", ++count);
    reset_color();
    printf("%s", line);
  }
  printf("\n");
  fclose(file);
  return MSH_CONTINUE;
}

int msh_tail(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: tail <file> [lines]");
    return MSH_CONTINUE;
  }

  int numLines = 10;
  if (args[2] != NULL) {
    numLines = atoi(args[2]);
    if (numLines <= 0)
      numLines = 10;
  }

  FILE *file = fopen(args[1], "r");
  if (!file) {
    char msg[512];
    sprintf(msg, "Cannot open: %s", args[1]);
    print_error(msg);
    return MSH_CONTINUE;
  }

  /* Count total lines */
  char line[4096];
  int totalLines = 0;
  while (fgets(line, sizeof(line), file)) {
    totalLines++;
  }

  /* Rewind and skip to the right position */
  rewind(file);
  int startLine = totalLines - numLines;
  if (startLine < 0)
    startLine = 0;

  set_color(CLR_HEADER);
  printf("\n  Last %d lines of %s:\n", numLines, args[1]);
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();

  int currentLine = 0;
  while (fgets(line, sizeof(line), file)) {
    if (currentLine >= startLine) {
      set_color(CLR_MUTED);
      printf("  %4d ", currentLine + 1);
      reset_color();
      printf("%s", line);
    }
    currentLine++;
  }
  printf("\n");
  fclose(file);
  return MSH_CONTINUE;
}

int msh_wc(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: wc <file1> [file2] ...");
    return MSH_CONTINUE;
  }

  printf("\n");
  set_color(CLR_HEADER);
  printf("  %-8s %-8s %-10s %s\n", "LINES", "WORDS", "CHARS", "FILE");
  set_color(CLR_MUTED);
  printf("  ──────────────────────────────────────────────────\n");
  reset_color();

  int totalLines = 0, totalWords = 0, totalChars = 0;

  for (int f = 1; args[f] != NULL; f++) {
    FILE *file = fopen(args[f], "r");
    if (!file) {
      char msg[512];
      sprintf(msg, "Cannot open: %s", args[f]);
      print_error(msg);
      continue;
    }

    int lines = 0, words = 0, chars = 0;
    int inWord = 0;
    int c;

    while ((c = fgetc(file)) != EOF) {
      chars++;
      if (c == '\n')
        lines++;
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        inWord = 0;
      } else if (!inWord) {
        inWord = 1;
        words++;
      }
    }

    set_color(CLR_ACCENT);
    printf("  %-8d %-8d %-10d ", lines, words, chars);
    set_color(CLR_BRIGHT_CYAN);
    printf("%s\n", args[f]);
    reset_color();

    totalLines += lines;
    totalWords += words;
    totalChars += chars;
    fclose(file);
  }

  /* Show total if multiple files */
  if (args[2] != NULL) {
    set_color(CLR_MUTED);
    printf("  ──────────────────────────────────────────────────\n");
    reset_color();
    set_color(CLR_BRIGHT_WHITE);
    printf("  %-8d %-8d %-10d total\n", totalLines, totalWords, totalChars);
    reset_color();
  }
  printf("\n");
  return MSH_CONTINUE;
}

int msh_mkdir(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: mkdir <directory>");
    return MSH_CONTINUE;
  }

  for (int i = 1; args[i] != NULL; i++) {
    if (CreateDirectory(args[i], NULL)) {
      char msg[512];
      sprintf(msg, "Created directory: %s", args[i]);
      print_success(msg);
    } else {
      DWORD err = GetLastError();
      if (err == ERROR_ALREADY_EXISTS) {
        char msg[512];
        sprintf(msg, "Directory already exists: %s", args[i]);
        print_warning(msg);
      } else {
        char msg[512];
        sprintf(msg, "Failed to create: %s", args[i]);
        print_error(msg);
      }
    }
  }
  return MSH_CONTINUE;
}

/*============================================================
 * SYSTEM UTILITIES
 *============================================================*/

int msh_whoami(char **args) {
  (void)args;
  char userName[256];
  DWORD userSize = sizeof(userName);
  GetUserName(userName, &userSize);
  set_color(CLR_BRIGHT_CYAN);
  printf("%s\n", userName);
  reset_color();
  return MSH_CONTINUE;
}

int msh_hostname(char **args) {
  (void)args;
  char computerName[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD compSize = sizeof(computerName);
  GetComputerName(computerName, &compSize);
  set_color(CLR_BRIGHT_CYAN);
  printf("%s\n", computerName);
  reset_color();
  return MSH_CONTINUE;
}

int msh_uptime(char **args) {
  (void)args;
  ULONGLONG uptimeSec = GetTickCount64() / 1000;
  int days = (int)(uptimeSec / 86400);
  int hours = (int)((uptimeSec % 86400) / 3600);
  int mins = (int)((uptimeSec % 3600) / 60);
  int secs = (int)(uptimeSec % 60);

  set_color(CLR_ACCENT);
  printf("  Uptime: ");
  reset_color();
  printf("%d days, %d hours, %d minutes, %d seconds\n", days, hours, mins,
         secs);
  return MSH_CONTINUE;
}

int msh_echo(char **args) {
  for (int i = 1; args[i] != NULL; i++) {
    if (i > 1)
      printf(" ");
    printf("%s", args[i]);
  }
  printf("\n");
  return MSH_CONTINUE;
}

int msh_clear(char **args) {
  (void)args;
  system("cls");
  return MSH_CONTINUE;
}

/*============================================================
 * TREE COMMAND - Directory tree visualization
 *============================================================*/

static void tree_print(const char *basePath, const char *prefix, int maxDepth,
                       int curDepth, int *dirCount, int *fileCount,
                       int lineDelayMs) {
  if (maxDepth > 0 && curDepth >= maxDepth)
    return;

  WIN32_FIND_DATA findData;
  char searchPath[MAX_PATH];
  sprintf(searchPath, "%s\\*", basePath);

  HANDLE hFind = FindFirstFile(searchPath, &findData);
  if (hFind == INVALID_HANDLE_VALUE)
    return;

  /* First pass: count entries to know which is last */
  typedef struct {
    char name[MAX_PATH];
    int isDir;
  } Entry;
  Entry entries[256];
  int count = 0;

  do {
    if (strcmp(findData.cFileName, ".") == 0 ||
        strcmp(findData.cFileName, "..") == 0)
      continue;
    if (findData.cFileName[0] == '.')
      continue; /* skip hidden */
    if (count < 256) {
      strncpy(entries[count].name, findData.cFileName, MAX_PATH - 1);
      entries[count].isDir =
          (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
      count++;
    }
  } while (FindNextFile(hFind, &findData));
  FindClose(hFind);

  for (int i = 0; i < count; i++) {
    int isLast = (i == count - 1);
    const char *connector = isLast ? "└── " : "├── ";

    printf("  %s", prefix);
    set_color(CLR_MUTED);
    printf("%s", connector);

    if (entries[i].isDir) {
      set_color(CLR_DIR_COLOR);
      printf("%s\n", entries[i].name);
      reset_color();
      (*dirCount)++;
      if (lineDelayMs > 0)
        Sleep(lineDelayMs);

      /* Recurse with updated prefix */
      char newPrefix[512];
      sprintf(newPrefix, "%s%s", prefix, isLast ? "    " : "│   ");
      char childPath[MAX_PATH];
      sprintf(childPath, "%s\\%s", basePath, entries[i].name);
      tree_print(childPath, newPrefix, maxDepth, curDepth + 1, dirCount,
                 fileCount, lineDelayMs);
    } else {
      set_color(CLR_FILE_COLOR);
      printf("%s\n", entries[i].name);
      reset_color();
      (*fileCount)++;
      if (lineDelayMs > 0)
        Sleep(lineDelayMs);
    }
  }
}

int msh_tree(char **args) {
  char startPath[MAX_PATH];
  int maxDepth = 0; /* 0 = unlimited */
  int lineDelayMs = 35;
  int i;

  GetCurrentDirectory(MAX_PATH, startPath);

  for (i = 1; args[i] != NULL; i++) {
    if (_stricmp(args[i], "-d") == 0) {
      if (args[i + 1] == NULL) {
        print_info("Usage: tree [path] [-d depth] [-s delay_ms]");
        return MSH_CONTINUE;
      }
      maxDepth = atoi(args[++i]);
      if (maxDepth < 0)
        maxDepth = 0;
    } else if (_stricmp(args[i], "-s") == 0) {
      if (args[i + 1] == NULL) {
        print_info("Usage: tree [path] [-d depth] [-s delay_ms]");
        return MSH_CONTINUE;
      }
      lineDelayMs = atoi(args[++i]);
      if (lineDelayMs < 0)
        lineDelayMs = 0;
      if (lineDelayMs > 1000)
        lineDelayMs = 1000;
    } else {
      strncpy(startPath, args[i], MAX_PATH - 1);
      startPath[MAX_PATH - 1] = '\0';
    }
  }

  printf("\n");
  set_color(CLR_HEADER);
  printf("  %s\n", startPath);
  reset_color();

  int dirCount = 0, fileCount = 0;
  tree_print(startPath, "", maxDepth, 0, &dirCount, &fileCount, lineDelayMs);

  set_color(CLR_MUTED);
  printf("\n  ");
  set_color(CLR_INFO);
  printf("%d directories, %d files\n\n", dirCount, fileCount);
  reset_color();

  return MSH_CONTINUE;
}

/*============================================================
 * CALC COMMAND - Simple math expression evaluator
 * Supports: +, -, *, /, (), unary minus
 * Uses recursive descent parsing
 *============================================================*/

typedef struct {
  const char *input;
  int pos;
} CalcParser;

static double calc_expr(CalcParser *p);

static void calc_skip_spaces(CalcParser *p) {
  while (p->input[p->pos] == ' ')
    p->pos++;
}

static double calc_number(CalcParser *p) {
  calc_skip_spaces(p);
  double result = 0;
  int hasDecimal = 0;
  double decimalPlace = 0.1;

  /* Handle unary minus */
  int negative = 0;
  if (p->input[p->pos] == '-') {
    negative = 1;
    p->pos++;
    calc_skip_spaces(p);
  }

  /* Handle parentheses */
  if (p->input[p->pos] == '(') {
    p->pos++; /* skip ( */
    result = calc_expr(p);
    if (p->input[p->pos] == ')')
      p->pos++; /* skip ) */
    return negative ? -result : result;
  }

  /* Parse number */
  while ((p->input[p->pos] >= '0' && p->input[p->pos] <= '9') ||
         p->input[p->pos] == '.') {
    if (p->input[p->pos] == '.') {
      hasDecimal = 1;
      p->pos++;
      continue;
    }
    if (hasDecimal) {
      result += (p->input[p->pos] - '0') * decimalPlace;
      decimalPlace *= 0.1;
    } else {
      result = result * 10 + (p->input[p->pos] - '0');
    }
    p->pos++;
  }

  return negative ? -result : result;
}

static double calc_factor(CalcParser *p) { return calc_number(p); }

static double calc_term(CalcParser *p) {
  double result = calc_factor(p);
  calc_skip_spaces(p);

  while (p->input[p->pos] == '*' || p->input[p->pos] == '/' ||
         p->input[p->pos] == '%') {
    char op = p->input[p->pos++];
    double right = calc_factor(p);
    calc_skip_spaces(p);
    if (op == '*')
      result *= right;
    else if (op == '/') {
      if (right == 0) {
        result = 0;
      } /* div by zero guard */
      else
        result /= right;
    } else if (op == '%')
      result = (int)result % (int)right;
  }
  return result;
}

static double calc_expr(CalcParser *p) {
  double result = calc_term(p);
  calc_skip_spaces(p);

  while (p->input[p->pos] == '+' || p->input[p->pos] == '-') {
    char op = p->input[p->pos++];
    double right = calc_term(p);
    calc_skip_spaces(p);
    if (op == '+')
      result += right;
    else
      result -= right;
  }
  return result;
}

int msh_calc(char **args) {
  if (args[1] == NULL) {
    print_info("Usage: calc <expression>");
    printf("  Examples: calc 2+3*4  |  calc (10-3)/2  |  calc 3.14*5*5\n");
    return MSH_CONTINUE;
  }

  /* Concatenate all args into one expression string */
  char expr[1024] = {0};
  for (int i = 1; args[i] != NULL; i++) {
    if (i > 1)
      strcat(expr, " ");
    strncat(expr, args[i], sizeof(expr) - strlen(expr) - 1);
  }

  CalcParser parser = {expr, 0};
  double result = calc_expr(&parser);

  printf("\n");
  set_color(CLR_ACCENT);
  printf("  ");
  set_color(CLR_BRIGHT_CYAN);
  printf("%s", expr);
  set_color(CLR_MUTED);
  printf(" = ");
  set_color(CLR_BRIGHT_WHITE);

  /* Print as integer if it is one, otherwise as decimal */
  if (result == (int)result && result < 1000000000 && result > -1000000000)
    printf("%d\n", (int)result);
  else
    printf("%.6g\n", result);

  reset_color();
  printf("\n");
  return MSH_CONTINUE;
}

/*============================================================
 * COLOR COMMAND - Switch shell color themes at runtime
 *============================================================*/

int msh_color(char **args) {
  if (args[1] == NULL) {
    printf("\n");
    set_color(CLR_HEADER);
    printf("  Color Themes\n");
    set_color(CLR_MUTED);
    printf("  ──────────────────────────────────────────────────\n");
    reset_color();

    const char *current = colors_get_theme();

    set_color(CLR_BRIGHT_GREEN);
    printf("  matrix   ");
    reset_color();
    printf("Classic green hacker theme%s\n",
           strcmp(current, "matrix") == 0 ? "  [active]" : "");

    set_color(CLR_BRIGHT_CYAN);
    printf("  ocean    ");
    reset_color();
    printf("Cool blue/cyan theme%s\n",
           strcmp(current, "ocean") == 0 ? "  [active]" : "");

    set_color(CLR_BRIGHT_YELLOW);
    printf("  sunset   ");
    reset_color();
    printf("Warm red/yellow theme%s\n",
           strcmp(current, "sunset") == 0 ? "  [active]" : "");

    set_color(CLR_BRIGHT_WHITE);
    printf("  retro    ");
    reset_color();
    printf("Classic white terminal%s\n",
           strcmp(current, "retro") == 0 ? "  [active]" : "");

    printf("\n");
    set_color(CLR_INFO);
    printf("  Usage: color <theme>\n");
    reset_color();
    printf("\n");
    return MSH_CONTINUE;
  }

  const char *oldTheme = colors_get_theme();
  colors_set_theme(args[1]);
  const char *newTheme = colors_get_theme();

  if (strcmp(oldTheme, newTheme) != 0 || _stricmp(args[1], newTheme) == 0) {
    char msg[128];
    sprintf(msg, "Theme switched to: %s", newTheme);
    print_success(msg);
  } else {
    char msg[128];
    sprintf(msg, "Unknown theme: %s (available: matrix, ocean, sunset, retro)",
            args[1]);
    print_warning(msg);
  }

  return MSH_CONTINUE;
}
