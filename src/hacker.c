#include "hacker.h"
#include "ai.h"
#include "colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static unsigned long g_promptCounter = 0;
#define HACKER_PROMPT_LINES 2

typedef struct {
    unsigned long counter;
    SYSTEMTIME timestamp;
    char userName[256];
    char shortDir[72];
    int ready;
} HackerPromptState;

static HackerPromptState g_promptState;

static HANDLE hacker_console_handle(void) {
    return GetStdHandle(STD_OUTPUT_HANDLE);
}

static int hacker_console_width(void) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE h = hacker_console_handle();
    if (h == INVALID_HANDLE_VALUE) return 80;
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return 80;

    {
        int width = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        if (width < 40) return 40;
        if (width > 140) return 140;
        return width;
    }
}

static void hacker_toggle_cursor(int visible) {
    HANDLE h = hacker_console_handle();
    CONSOLE_CURSOR_INFO ci;

    if (h == INVALID_HANDLE_VALUE) return;
    if (!GetConsoleCursorInfo(h, &ci)) return;

    ci.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(h, &ci);
}

static void hacker_clear_screen(void) {
    HANDLE h = hacker_console_handle();
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD home = {0, 0};
    DWORD written = 0;
    DWORD cells = 0;

    if (h == INVALID_HANDLE_VALUE || !GetConsoleScreenBufferInfo(h, &csbi)) {
        system("cls");
        return;
    }

    cells = (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y;
    FillConsoleOutputCharacterA(h, ' ', cells, home, &written);
    FillConsoleOutputAttribute(h, csbi.wAttributes, cells, home, &written);
    SetConsoleCursorPosition(h, home);
}

static void hacker_rule_line(WORD color) {
    int width = hacker_console_width();
    int span = width - 6;
    if (span < 30) span = 30;
    if (span > 84) span = 84;

    printf("  +");
    set_color(color);
    for (int i = 0; i < span; i++) putchar('-');
    reset_color();
    printf("+\n");
}

static void hacker_center_frame_text(const char *text, WORD color) {
    int width = hacker_console_width();
    int span = width - 6;
    int len = (int)strlen(text);
    int leftPad = 0;
    int rightPad = 0;
    int printableLen = len;
    char clipped[256];

    if (span < 30) span = 30;
    if (span > 84) span = 84;

    if (printableLen > span) {
        printableLen = span;
        snprintf(clipped, sizeof(clipped), "%.*s", printableLen, text);
        text = clipped;
    }

    leftPad = (span - printableLen) / 2;
    rightPad = span - printableLen - leftPad;

    printf("  |");
    for (int i = 0; i < leftPad; i++) putchar(' ');
    set_color(color);
    printf("%s", text);
    reset_color();
    for (int i = 0; i < rightPad; i++) putchar(' ');
    printf("|\n");
}

static void hacker_shorten_path(const char *src, char *dst, size_t dstSize) {
    size_t len;
    size_t tailLen;

    if (!src || !dst || dstSize == 0) return;

    len = strlen(src);
    if (len + 1 <= dstSize) {
        strcpy(dst, src);
        return;
    }

    if (dstSize <= 5) {
        snprintf(dst, dstSize, "%s", src + (len - (dstSize - 1)));
        return;
    }

    tailLen = dstSize - 4;
    strcpy(dst, "...");
    strncat(dst, src + (len - tailLen), tailLen);
    dst[dstSize - 1] = '\0';
}

static void hacker_capture_prompt_state(unsigned long counter) {
    char currentDir[MAX_PATH];
    DWORD userSize = sizeof(g_promptState.userName);

    g_promptState.counter = counter;
    GetCurrentDirectory(MAX_PATH, currentDir);
    hacker_shorten_path(currentDir, g_promptState.shortDir, sizeof(g_promptState.shortDir));

    if (!GetUserName(g_promptState.userName, &userSize)) {
        strcpy(g_promptState.userName, "USER");
    }

    GetLocalTime(&g_promptState.timestamp);
    g_promptState.ready = 1;
}

static int hacker_prompt_target_row(const CONSOLE_SCREEN_BUFFER_INFO *csbi) {
    int windowHeight;
    int targetRow;
    int maxRow;

    if (!csbi) return 8;

    windowHeight = (int)(csbi->srWindow.Bottom - csbi->srWindow.Top + 1);
    if (windowHeight < 12) return 3;

    targetRow = (windowHeight * 3) / 5;
    maxRow = windowHeight - HACKER_PROMPT_LINES - 3;

    if (maxRow < 3) maxRow = 3;
    if (targetRow < 3) targetRow = 3;
    if (targetRow > maxRow) targetRow = maxRow;

    return targetRow;
}

static void hacker_anchor_prompt_zone(void) {
    HANDLE h = hacker_console_handle();
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int windowHeight;
    int targetRow;
    int relativeRow;

    if (h == INVALID_HANDLE_VALUE) return;
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return;

    windowHeight = (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
    targetRow = hacker_prompt_target_row(&csbi);
    relativeRow = (int)(csbi.dwCursorPosition.Y - csbi.srWindow.Top);

    if (relativeRow > targetRow) {
        SMALL_RECT window = csbi.srWindow;
        SHORT maxTop = (SHORT)(csbi.dwSize.Y - windowHeight);
        SHORT newTop = (SHORT)(csbi.dwCursorPosition.Y - targetRow);

        if (newTop < 0) newTop = 0;
        if (newTop > maxTop) newTop = maxTop;

        window.Top = newTop;
        window.Bottom = (SHORT)(newTop + windowHeight - 1);
        SetConsoleWindowInfo(h, TRUE, &window);

        if (!GetConsoleScreenBufferInfo(h, &csbi)) return;
        relativeRow = (int)(csbi.dwCursorPosition.Y - csbi.srWindow.Top);
    }

    while (relativeRow < targetRow) {
        putchar('\n');
        relativeRow++;
    }
}

static void hacker_entropy_burst(int lines, int columns, int delayMs) {
    const char hexChars[] = "0123456789ABCDEF";

    if (columns < 8) columns = 8;
    if (columns > 24) columns = 24;

    for (int row = 0; row < lines; row++) {
        set_color(CLR_BRIGHT_GREEN);
        printf("  0x%04X  ", rand() % 0xFFFF);

        for (int col = 0; col < columns; col++) {
            int hot = (rand() % 19 == 0);
            if (hot) set_color(CLR_BRIGHT_WHITE);
            else set_color(CLR_BRIGHT_GREEN);

            printf("%c%c", hexChars[rand() % 16], hexChars[rand() % 16]);
            if (col % 2 == 1) putchar(' ');
        }

        reset_color();
        printf("\n");
        fflush(stdout);
        Sleep(delayMs);
    }
}

static void hacker_radar_sweep(const char *label, int width, int loops, int delayMs) {
    if (width < 16) width = 16;
    if (width > 44) width = 44;

    for (int loop = 0; loop < loops; loop++) {
        for (int pos = 0; pos < width; pos++) {
            printf("\r  ");
            set_color(CLR_BRIGHT_GREEN);
            printf("[SCAN] ");
            set_color(CLR_BRIGHT_WHITE);
            printf("%s ", label);
            set_color(CLR_BRIGHT_GREEN);
            putchar('[');

            for (int i = 0; i < width; i++) {
                int d = abs(i - pos);
                if (d == 0) {
                    set_color(CLR_BRIGHT_WHITE);
                    putchar('|');
                    set_color(CLR_BRIGHT_GREEN);
                } else if (d <= 2) {
                    putchar(':');
                } else {
                    putchar('.');
                }
            }
            putchar(']');
            reset_color();
            fflush(stdout);
            Sleep(delayMs);
        }
    }
    printf("\n");
}

static void hacker_countdown(const char *label, int start, int delayMs) {
    for (int i = start; i >= 1; i--) {
        printf("\r  ");
        set_color(CLR_BRIGHT_GREEN);
        printf("%s in ", label);
        set_color(CLR_BRIGHT_WHITE);
        printf("%d", i);
        reset_color();
        printf("...");
        fflush(stdout);
        Sleep(delayMs);
    }
    printf("\n");
}

void hacker_delay(int minMs, int maxMs) {
    int delay;
    if (maxMs < minMs) {
        int t = minMs;
        minMs = maxMs;
        maxMs = t;
    }
    delay = minMs + (rand() % (maxMs - minMs + 1));
    Sleep(delay);
}

void hacker_beep(void) {
    Beep(800, 50);
}

void hacker_type(const char *text, int delayMs) {
    for (int i = 0; text[i]; i++) {
        putchar(text[i]);
        fflush(stdout);
        if (text[i] != ' ') Sleep(delayMs);
        else Sleep(delayMs / 3);
    }
}

void hacker_type_colored(const char *text, WORD color, int delayMs) {
    set_color(color);
    hacker_type(text, delayMs);
    reset_color();
}

void hacker_hex_rain(int lines, int delayMs) {
    const char hexChars[] = "0123456789ABCDEF";
    set_color(CLR_BRIGHT_GREEN);
    for (int row = 0; row < lines; row++) {
        printf("  0x%04X  ", (rand() % 0xFFFF));
        for (int col = 0; col < 32; col++) {
            printf("%c%c ", hexChars[rand() % 16], hexChars[rand() % 16]);
            if (col % 8 == 7) printf(" ");
        }
        printf("\n");
        fflush(stdout);
        Sleep(delayMs);
    }
    reset_color();
}

void hacker_binary_stream(int chars, int delayMs) {
    set_color(CLR_BRIGHT_GREEN);
    for (int i = 0; i < chars; i++) {
        printf("%d", rand() % 2);
        if (i % 8 == 7) printf(" ");
        if (i % 64 == 63) printf("\n");
        fflush(stdout);
        Sleep(delayMs);
    }
    printf("\n");
    reset_color();
}

void hacker_progress_bar(const char *label, int durationMs) {
    int barWidth = 30;
    int steps = 24;
    int stepDelay = durationMs / steps;

    for (int i = 0; i <= steps; i++) {
        int percent = (i * 100) / steps;
        int filled = (i * barWidth) / steps;

        printf("\r  ");
        set_color(CLR_BRIGHT_GREEN);
        printf("%s ", label);
        set_color(CLR_BRIGHT_WHITE);
        printf("[");
        set_color(CLR_BRIGHT_GREEN);
        for (int j = 0; j < filled; j++) putchar('#');
        if (filled < barWidth) {
            set_color(CLR_BRIGHT_WHITE);
            putchar('>');
            set_color(CLR_BRIGHT_GREEN);
            for (int j = filled + 1; j < barWidth; j++) putchar('.');
        }
        set_color(CLR_BRIGHT_WHITE);
        printf("] ");
        set_color(CLR_BRIGHT_GREEN);
        printf("%3d%%", percent);
        reset_color();
        fflush(stdout);

        if (i < 2 || i > steps - 3) Sleep(stepDelay * 2);
        else Sleep(stepDelay / 2 + (rand() % (stepDelay + 1)));
    }
    printf("\n");
}

void hacker_scan(const char *label, int durationMs) {
    int dots = 0;
    int maxDots = 4;
    int elapsed = 0;
    int interval = 220;

    while (elapsed < durationMs) {
        printf("\r  ");
        set_color(CLR_BRIGHT_GREEN);
        printf("[*] ");
        set_color(CLR_BRIGHT_WHITE);
        printf("%s", label);
        for (int i = 0; i < maxDots; i++) {
            if (i < dots) putchar('.');
            else putchar(' ');
        }
        reset_color();
        fflush(stdout);

        dots = (dots + 1) % (maxDots + 1);
        Sleep(interval);
        elapsed += interval;
    }

    printf("\r  ");
    set_color(CLR_BRIGHT_GREEN);
    printf("[+] ");
    set_color(CLR_BRIGHT_WHITE);
    printf("%s", label);
    set_color(CLR_BRIGHT_GREEN);
    printf(" [OK]\n");
    reset_color();
}

void hacker_access_granted(void) {
    for (int flash = 0; flash < 4; flash++) {
        printf("\r  ");
        set_color(CLR_BRIGHT_GREEN | BACKGROUND_GREEN);
        printf("                 ACCESS GRANTED                 ");
        reset_color();
        fflush(stdout);
        Sleep(90);
        printf("\r  ");
        printf("                                                ");
        fflush(stdout);
        Sleep(70);
    }

    printf("\r  ");
    set_color(CLR_BRIGHT_GREEN);
    printf("  >>> ");
    set_color(CLR_BRIGHT_WHITE);
    printf("ROOT TOKEN ACCEPTED");
    set_color(CLR_BRIGHT_GREEN);
    printf(" <<<");
    reset_color();
    printf("\n");
}

void hacker_glitch_reveal(const char *text, WORD color, int iterations) {
    int len = (int)strlen(text);
    char *buf = (char *)malloc((size_t)len + 1);
    const char glitchChars[] = "@#$%&!?*^~+=<>/\\|{}[]";
    int glitchLen = (int)strlen(glitchChars);

    if (!buf) return;

    for (int iter = 0; iter < iterations; iter++) {
        int revealed = (iter * len) / iterations;

        for (int i = 0; i < len; i++) {
            if (i < revealed) buf[i] = text[i];
            else if (text[i] == ' ') buf[i] = ' ';
            else buf[i] = glitchChars[rand() % glitchLen];
        }
        buf[len] = '\0';

        printf("\r");
        set_color(color);
        printf("%s", buf);
        reset_color();
        fflush(stdout);
        Sleep(35 + (rand() % 24));
    }

    printf("\r");
    set_color(color);
    printf("%s", text);
    reset_color();
    printf("\n");
    free(buf);
}

void hacker_boot_sequence(void) {
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    char userName[256];
    char currentDir[MAX_PATH];
    char shortDir[72];
    char line[512];
    DWORD compSize = sizeof(computerName);
    DWORD userSize = sizeof(userName);
    SYSTEMTIME st;
    ULONGLONG uptimeSec;
    int days;
    int hours;
    int mins;
    MEMORYSTATUSEX memInfo;
    DWORDLONG totalRAM_MB;
    DWORDLONG usedRAM_MB;

    srand((unsigned int)(time(NULL) ^ GetTickCount()));

    GetComputerName(computerName, &compSize);
    GetUserName(userName, &userSize);
    GetCurrentDirectory(MAX_PATH, currentDir);
    hacker_shorten_path(currentDir, shortDir, sizeof(shortDir));
    GetLocalTime(&st);

    uptimeSec = GetTickCount64() / 1000;
    days = (int)(uptimeSec / 86400);
    hours = (int)((uptimeSec % 86400) / 3600);
    mins = (int)((uptimeSec % 3600) / 60);

    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatusEx(&memInfo);
    totalRAM_MB = memInfo.ullTotalPhys / (1024 * 1024);
    usedRAM_MB = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024);

    hacker_toggle_cursor(0);
    hacker_clear_screen();

    printf("\n");
    hacker_rule_line(CLR_BRIGHT_GREEN);
    hacker_center_frame_text("MSH QUANTUM BOOT CHAMBER", CLR_BRIGHT_GREEN);
    hacker_center_frame_text("High-signal shell engine online", CLR_MUTED);
    hacker_rule_line(CLR_BRIGHT_GREEN);
    printf("\n");

    hacker_glitch_reveal("  [NEURAL-LINK] Injecting shell intelligence...", CLR_BRIGHT_GREEN, 14);
    hacker_radar_sweep("Aligning threat vectors", 30, 2, 16);
    hacker_progress_bar("Compiling command graph", 620);
    hacker_progress_bar("Decrypting profile state", 560);
    hacker_progress_bar("Mounting process controls", 600);
    printf("\n");

    set_color(CLR_BRIGHT_GREEN);
    hacker_type("  [DATA] Entropy burst from memory sectors:\n", 7);
    reset_color();
    hacker_entropy_burst(5, 16, 18);
    printf("\n");

    hacker_scan("Fingerprinting operator", 700);
    hacker_scan("Building execution lattice", 620);
    hacker_scan("Establishing stealth channel", 680);
    printf("\n");

    hacker_rule_line(CLR_BRIGHT_GREEN);
    hacker_center_frame_text("LIVE SYSTEM TELEMETRY", CLR_BRIGHT_WHITE);
    hacker_rule_line(CLR_BRIGHT_GREEN);

    snprintf(line, sizeof(line), "  [HOST]   %s", computerName);
    hacker_type_colored(line, CLR_BRIGHT_GREEN, 6); printf("\n");
    snprintf(line, sizeof(line), "  [USER]   %s", userName);
    hacker_type_colored(line, CLR_BRIGHT_GREEN, 6); printf("\n");
    snprintf(line, sizeof(line), "  [TIME]   %04d-%02d-%02d %02d:%02d:%02d",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    hacker_type_colored(line, CLR_BRIGHT_GREEN, 6); printf("\n");
    snprintf(line, sizeof(line), "  [UPTIME] %dd %dh %dm", days, hours, mins);
    hacker_type_colored(line, CLR_BRIGHT_GREEN, 6); printf("\n");
    snprintf(line, sizeof(line), "  [MEM]    %llu / %llu MB (%lu%%)",
             usedRAM_MB, totalRAM_MB, memInfo.dwMemoryLoad);
    hacker_type_colored(line, CLR_BRIGHT_GREEN, 6); printf("\n");
    snprintf(line, sizeof(line), "  [DIR]    %s", shortDir);
    hacker_type_colored(line, CLR_BRIGHT_GREEN, 6); printf("\n");

    hacker_rule_line(CLR_BRIGHT_GREEN);
    printf("\n");

    hacker_glitch_reveal("   __  __  ____  _   _   ____  _   _  _____  _      _      ", CLR_BRIGHT_GREEN, 12);
    hacker_glitch_reveal("  |  \\/  |/ ___|| | | | / ___|| | | || ____|| |    | |     ", CLR_BRIGHT_GREEN, 12);
    hacker_glitch_reveal("  | |\\/| |\\___ \\| |_| | \\___ \\| |_| ||  _|  | |    | |     ", CLR_BRIGHT_GREEN, 12);
    hacker_glitch_reveal("  | |  | | ___) |  _  |  ___) |  _  || |___ | |___ | |___  ", CLR_BRIGHT_GREEN, 12);
    hacker_glitch_reveal("  |_|  |_||____/|_| |_| |____/|_| |_||_____||_____||_____| ", CLR_BRIGHT_GREEN, 12);
    printf("\n");

    hacker_access_granted();
    hacker_countdown("Launching shell", 3, 280);

    set_color(CLR_BRIGHT_GREEN);
    hacker_type("  [READY] Shell hot. Type 'demo' for the full showcase.\n", 8);
    hacker_type("  [READY] Type 'help' for all commands.\n\n", 8);
    reset_color();

    hacker_toggle_cursor(1);
}

void hacker_begin_prompt(void) {
    g_promptCounter++;
    hacker_capture_prompt_state(g_promptCounter);
    hacker_anchor_prompt_zone();
}

void hacker_rewind_prompt(void) {
    HANDLE h = hacker_console_handle();
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD pos;

    if (h == INVALID_HANDLE_VALUE) return;
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return;

    pos = csbi.dwCursorPosition;
    if (pos.Y > 0) pos.Y--;
    pos.X = 0;
    SetConsoleCursorPosition(h, pos);
}

void hacker_prompt(void) {
    if (!g_promptState.ready) {
        hacker_begin_prompt();
    }

    set_color(CLR_MUTED);
    printf("[#%04lu %02d:%02d:%02d] ",
           g_promptState.counter,
           g_promptState.timestamp.wHour,
           g_promptState.timestamp.wMinute,
           g_promptState.timestamp.wSecond);
    set_color(CLR_BRIGHT_GREEN);
    printf("%s", g_promptState.userName);
    set_color(CLR_BRIGHT_WHITE);
    printf("@");
    set_color(CLR_BRIGHT_GREEN);
    printf("msh ");
    set_color(CLR_BRIGHT_YELLOW);
    printf("[LIVE]");
    reset_color();
    printf(" ");
    set_color(CLR_BRIGHT_CYAN);
    printf("%s", g_promptState.shortDir);
    reset_color();
    printf("\n");
    set_color(CLR_BRIGHT_GREEN);
    printf(ai_is_mode_enabled() ? "[AI]>> " : ">> ");
    reset_color();
}
