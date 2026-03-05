#ifndef COLORS_H
#define COLORS_H

#include "../include/common.h"

/*============================================================
 * COLOR CONSTANTS - Win32 Console Colors
 * Uses SetConsoleTextAttribute() with FOREGROUND_* flags
 *============================================================*/

/* Base colors */
#define CLR_BLACK       0
#define CLR_BLUE        (FOREGROUND_BLUE)
#define CLR_GREEN       (FOREGROUND_GREEN)
#define CLR_CYAN        (FOREGROUND_GREEN | FOREGROUND_BLUE)
#define CLR_RED         (FOREGROUND_RED)
#define CLR_MAGENTA     (FOREGROUND_RED | FOREGROUND_BLUE)
#define CLR_YELLOW      (FOREGROUND_RED | FOREGROUND_GREEN)
#define CLR_WHITE       (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

/* Bright/Intense colors */
#define CLR_BRIGHT_BLUE     (CLR_BLUE | FOREGROUND_INTENSITY)
#define CLR_BRIGHT_GREEN    (CLR_GREEN | FOREGROUND_INTENSITY)
#define CLR_BRIGHT_CYAN     (CLR_CYAN | FOREGROUND_INTENSITY)
#define CLR_BRIGHT_RED      (CLR_RED | FOREGROUND_INTENSITY)
#define CLR_BRIGHT_MAGENTA  (CLR_MAGENTA | FOREGROUND_INTENSITY)
#define CLR_BRIGHT_YELLOW   (CLR_YELLOW | FOREGROUND_INTENSITY)
#define CLR_BRIGHT_WHITE    (CLR_WHITE | FOREGROUND_INTENSITY)

/* Semantic colors - used throughout the shell */
/* Semantic colors - used throughout the shell (Matrix Theme) */
#define CLR_PROMPT      CLR_BRIGHT_GREEN
#define CLR_PATH        CLR_BRIGHT_GREEN
#define CLR_ERROR       CLR_BRIGHT_RED
#define CLR_SUCCESS     CLR_BRIGHT_GREEN
#define CLR_WARNING     CLR_GREEN
#define CLR_INFO        CLR_GREEN
#define CLR_HEADER      CLR_BRIGHT_GREEN
#define CLR_MUTED       CLR_GREEN
#define CLR_ACCENT      CLR_BRIGHT_GREEN
#define CLR_DEFAULT     CLR_GREEN
#define CLR_HIGHLIGHT   CLR_BRIGHT_WHITE
#define CLR_DIR_COLOR   CLR_BRIGHT_GREEN
#define CLR_FILE_COLOR  CLR_GREEN
#define CLR_EXE_COLOR   CLR_BRIGHT_GREEN

/*============================================================
 * FUNCTION DECLARATIONS
 *============================================================*/

/* Initialize console color support */
void colors_init(void);

/* Set console text color */
void set_color(WORD color);

/* Reset to default color */
void reset_color(void);

/* Print colored text (does NOT add newline) */
void print_colored(const char *text, WORD color);

/* Print colored text with newline */
void print_colored_ln(const char *text, WORD color);

/* Semantic print helpers */
void print_error(const char *msg);
void print_success(const char *msg);
void print_warning(const char *msg);
void print_info(const char *msg);

/* Print the styled prompt */
void print_prompt(void);

/* Print the startup banner */
void print_banner(void);

#endif /* COLORS_H */
