#include "colors.h"
#include <stdio.h>
#include <string.h>

/* Store original console attributes to restore later */
static WORD g_originalAttrs = CLR_WHITE;
static HANDLE g_hConsole = INVALID_HANDLE_VALUE;
static char g_currentTheme[32] = "matrix";

/* Global semantic color variables (runtime-switchable) */
WORD CLR_PROMPT = (FOREGROUND_GREEN | FOREGROUND_INTENSITY);
WORD CLR_PATH = (FOREGROUND_GREEN | FOREGROUND_INTENSITY);
WORD CLR_ERROR = (FOREGROUND_RED | FOREGROUND_INTENSITY);
WORD CLR_SUCCESS = (FOREGROUND_GREEN | FOREGROUND_INTENSITY);
WORD CLR_WARNING = (FOREGROUND_GREEN);
WORD CLR_INFO = (FOREGROUND_GREEN);
WORD CLR_HEADER = (FOREGROUND_GREEN | FOREGROUND_INTENSITY);
WORD CLR_MUTED = (FOREGROUND_GREEN);
WORD CLR_ACCENT = (FOREGROUND_GREEN | FOREGROUND_INTENSITY);
WORD CLR_DEFAULT = (FOREGROUND_GREEN);
WORD CLR_HIGHLIGHT = (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE |
                      FOREGROUND_INTENSITY);
WORD CLR_DIR_COLOR = (FOREGROUND_GREEN | FOREGROUND_INTENSITY);
WORD CLR_FILE_COLOR = (FOREGROUND_GREEN);
WORD CLR_EXE_COLOR = (FOREGROUND_GREEN | FOREGROUND_INTENSITY);

int g_demo_mode = 0;

void colors_init(void) {
  g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (g_hConsole != INVALID_HANDLE_VALUE) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(g_hConsole, &csbi)) {
      g_originalAttrs = csbi.wAttributes;
    }
  }

  /* Set console code page to UTF-8 for proper character rendering */
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  /* Enable Virtual Terminal Processing for modern ANSI support */
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode = 0;
  if (GetConsoleMode(hOut, &dwMode)) {
    dwMode |= 0x0004; /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
    SetConsoleMode(hOut, dwMode);
  }
}

void colors_set_theme(const char *name) {
  if (_stricmp(name, "matrix") == 0 || _stricmp(name, "green") == 0) {
    CLR_PROMPT = CLR_BRIGHT_GREEN;
    CLR_PATH = CLR_BRIGHT_GREEN;
    CLR_ERROR = CLR_BRIGHT_RED;
    CLR_SUCCESS = CLR_BRIGHT_GREEN;
    CLR_WARNING = CLR_GREEN;
    CLR_INFO = CLR_GREEN;
    CLR_HEADER = CLR_BRIGHT_GREEN;
    CLR_MUTED = CLR_GREEN;
    CLR_ACCENT = CLR_BRIGHT_GREEN;
    CLR_DEFAULT = CLR_GREEN;
    CLR_HIGHLIGHT = CLR_BRIGHT_WHITE;
    CLR_DIR_COLOR = CLR_BRIGHT_GREEN;
    CLR_FILE_COLOR = CLR_GREEN;
    CLR_EXE_COLOR = CLR_BRIGHT_GREEN;
    strncpy(g_currentTheme, "matrix", sizeof(g_currentTheme));
  } else if (_stricmp(name, "ocean") == 0 || _stricmp(name, "blue") == 0) {
    CLR_PROMPT = CLR_BRIGHT_CYAN;
    CLR_PATH = CLR_BRIGHT_CYAN;
    CLR_ERROR = CLR_BRIGHT_RED;
    CLR_SUCCESS = CLR_BRIGHT_CYAN;
    CLR_WARNING = CLR_BRIGHT_YELLOW;
    CLR_INFO = CLR_CYAN;
    CLR_HEADER = CLR_BRIGHT_CYAN;
    CLR_MUTED = CLR_CYAN;
    CLR_ACCENT = CLR_BRIGHT_BLUE;
    CLR_DEFAULT = CLR_CYAN;
    CLR_HIGHLIGHT = CLR_BRIGHT_WHITE;
    CLR_DIR_COLOR = CLR_BRIGHT_CYAN;
    CLR_FILE_COLOR = CLR_CYAN;
    CLR_EXE_COLOR = CLR_BRIGHT_BLUE;
    strncpy(g_currentTheme, "ocean", sizeof(g_currentTheme));
  } else if (_stricmp(name, "sunset") == 0 || _stricmp(name, "red") == 0) {
    CLR_PROMPT = CLR_BRIGHT_YELLOW;
    CLR_PATH = CLR_BRIGHT_YELLOW;
    CLR_ERROR = CLR_BRIGHT_RED;
    CLR_SUCCESS = CLR_BRIGHT_YELLOW;
    CLR_WARNING = CLR_YELLOW;
    CLR_INFO = CLR_YELLOW;
    CLR_HEADER = CLR_BRIGHT_RED;
    CLR_MUTED = CLR_YELLOW;
    CLR_ACCENT = CLR_BRIGHT_MAGENTA;
    CLR_DEFAULT = CLR_YELLOW;
    CLR_HIGHLIGHT = CLR_BRIGHT_WHITE;
    CLR_DIR_COLOR = CLR_BRIGHT_YELLOW;
    CLR_FILE_COLOR = CLR_YELLOW;
    CLR_EXE_COLOR = CLR_BRIGHT_RED;
    strncpy(g_currentTheme, "sunset", sizeof(g_currentTheme));
  } else if (_stricmp(name, "retro") == 0 || _stricmp(name, "white") == 0) {
    CLR_PROMPT = CLR_BRIGHT_WHITE;
    CLR_PATH = CLR_BRIGHT_WHITE;
    CLR_ERROR = CLR_BRIGHT_RED;
    CLR_SUCCESS = CLR_BRIGHT_GREEN;
    CLR_WARNING = CLR_BRIGHT_YELLOW;
    CLR_INFO = CLR_WHITE;
    CLR_HEADER = CLR_BRIGHT_WHITE;
    CLR_MUTED = CLR_WHITE;
    CLR_ACCENT = CLR_BRIGHT_CYAN;
    CLR_DEFAULT = CLR_WHITE;
    CLR_HIGHLIGHT = CLR_BRIGHT_YELLOW;
    CLR_DIR_COLOR = CLR_BRIGHT_CYAN;
    CLR_FILE_COLOR = CLR_WHITE;
    CLR_EXE_COLOR = CLR_BRIGHT_YELLOW;
    strncpy(g_currentTheme, "retro", sizeof(g_currentTheme));
  }
}

const char *colors_get_theme(void) { return g_currentTheme; }

void set_color(WORD color) {
  if (g_hConsole != INVALID_HANDLE_VALUE) {
    if (g_demo_mode) {
      fflush(stdout);
      Sleep(20);
    }
    SetConsoleTextAttribute(g_hConsole, color);
  }
}

void reset_color(void) {
  if (g_hConsole != INVALID_HANDLE_VALUE) {
    SetConsoleTextAttribute(g_hConsole, g_originalAttrs);
  }
}

void print_colored(const char *text, WORD color) {
  set_color(color);
  printf("%s", text);
  reset_color();
}

void print_colored_ln(const char *text, WORD color) {
  set_color(color);
  printf("%s\n", text);
  reset_color();
}

void print_error(const char *msg) {
  set_color(CLR_ERROR);
  printf("[ERROR] %s\n", msg);
  reset_color();
}

void print_success(const char *msg) {
  set_color(CLR_SUCCESS);
  printf("[OK] %s\n", msg);
  reset_color();
}

void print_warning(const char *msg) {
  set_color(CLR_WARNING);
  printf("[WARN] %s\n", msg);
  reset_color();
}

void print_info(const char *msg) {
  set_color(CLR_INFO);
  printf("[INFO] %s\n", msg);
  reset_color();
}

void print_prompt(void) {
  char currentDir[MAX_PATH];
  char computerName[MAX_COMPUTERNAME_LENGTH + 1];
  char userName[256];
  DWORD compSize = sizeof(computerName);
  DWORD userSize = sizeof(userName);

  GetCurrentDirectory(MAX_PATH, currentDir);
  GetComputerName(computerName, &compSize);
  GetUserName(userName, &userSize);

  /* Format: user@machine path $ */
  set_color(CLR_BRIGHT_GREEN);
  printf("%s", userName);
  set_color(CLR_MUTED);
  printf("@");
  set_color(CLR_BRIGHT_YELLOW);
  printf("%s", computerName);
  reset_color();
  printf(" ");
  set_color(CLR_BRIGHT_CYAN);
  printf("%s", currentDir);
  reset_color();
  printf("\n");
  set_color(CLR_BRIGHT_MAGENTA);
  printf("$ ");
  reset_color();
}

void print_banner(void) {
  char computerName[MAX_COMPUTERNAME_LENGTH + 1];
  char userName[256];
  DWORD compSize = sizeof(computerName);
  DWORD userSize = sizeof(userName);
  GetComputerName(computerName, &compSize);
  GetUserName(userName, &userSize);

  char currentDir[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, currentDir);

  SYSTEMTIME st;
  GetLocalTime(&st);

  ULONGLONG uptimeSec = GetTickCount64() / 1000;
  int days = (int)(uptimeSec / 86400);
  int hours = (int)((uptimeSec % 86400) / 3600);
  int mins = (int)((uptimeSec % 3600) / 60);

  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(memInfo);
  GlobalMemoryStatusEx(&memInfo);
  DWORDLONG totalRAM_MB = memInfo.ullTotalPhys / (1024 * 1024);
  DWORDLONG usedRAM_MB =
      (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024);

  printf("\n");
  set_color(CLR_BRIGHT_CYAN);
  printf("  ___  ___ ___  _  _\n");
  printf(" |  \\/  |/ __|| || |\n");
  printf(" | |\\/| |\\__ \\| __ |\n");
  printf(" |_|  |_||___/|_||_|\n");
  reset_color();

  printf("\n");
  set_color(CLR_HEADER);
  printf(" MSH - Modern Shell for Windows");
  reset_color();
  printf("  v2.0\n");

  set_color(CLR_MUTED);
  printf(" ──────────────────────────────────────────\n");
  reset_color();

  /* User & Machine */
  set_color(CLR_ACCENT);
  printf("  User     ");
  reset_color();
  printf(" %s@%s\n", userName, computerName);

  /* Date & Time */
  set_color(CLR_ACCENT);
  printf("  Date     ");
  reset_color();
  printf(" %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay,
         st.wHour, st.wMinute, st.wSecond);

  /* Uptime */
  set_color(CLR_ACCENT);
  printf("  Uptime   ");
  reset_color();
  printf(" %dd %dh %dm\n", days, hours, mins);

  /* Memory */
  set_color(CLR_ACCENT);
  printf("  Memory   ");
  reset_color();
  printf(" %llu / %llu MB (%lu%%)\n", usedRAM_MB, totalRAM_MB,
         memInfo.dwMemoryLoad);

  /* Current Dir */
  set_color(CLR_ACCENT);
  printf("  Dir      ");
  reset_color();
  printf(" %s\n", currentDir);

  set_color(CLR_MUTED);
  printf(" ──────────────────────────────────────────\n");
  reset_color();

  set_color(CLR_INFO);
  printf("  Type 'help' for commands, 'exit' to quit\n");
  reset_color();
  printf("\n");
}
