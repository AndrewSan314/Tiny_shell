#include "colors.h"
#include <stdio.h>
#include <string.h>

/* Store original console attributes to restore later */
static WORD g_originalAttrs = CLR_WHITE;
static HANDLE g_hConsole = INVALID_HANDLE_VALUE;

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

void set_color(WORD color) {
    if (g_hConsole != INVALID_HANDLE_VALUE) {
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
    DWORDLONG usedRAM_MB = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024);

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
    printf(" %04d-%02d-%02d %02d:%02d:%02d\n",
           st.wYear, st.wMonth, st.wDay,
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
    printf(" %llu / %llu MB (%lu%%)\n", usedRAM_MB, totalRAM_MB, memInfo.dwMemoryLoad);

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
