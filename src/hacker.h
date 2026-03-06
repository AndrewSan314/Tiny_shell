#ifndef HACKER_H
#define HACKER_H

#include "../include/common.h"

/*============================================================
 * HOLLYWOOD HACKER VISUAL EFFECTS ENGINE
 * Pure Win32 API - No external dependencies
 *============================================================*/

/* Typing animation: prints text char-by-char with delay */
void hacker_type(const char *text, int delayMs);

/* Typing animation with color */
void hacker_type_colored(const char *text, WORD color, int delayMs);

/* Fake hex dump rain (random hex scrolling) */
void hacker_hex_rain(int lines, int delayMs);

/* Fake binary stream */
void hacker_binary_stream(int chars, int delayMs);

/* Progress bar with label: [=========>          ] 45% */
void hacker_progress_bar(const char *label, int durationMs);

/* Fake "scanning" animation with dots */
void hacker_scan(const char *label, int durationMs);

/* ACCESS GRANTED / DENIED animation */
void hacker_access_granted(void);

/* Glitch text effect (scramble then reveal) */
void hacker_glitch_reveal(const char *text, WORD color, int iterations);

/* The full Hollywood startup sequence */
void hacker_boot_sequence(void);

/* Capture state and anchor the next prompt in a readable zone */
void hacker_begin_prompt(void);

/* Hacker-style prompt with timestamp */
void hacker_prompt(void);

/* Move the cursor back to the beginning of the active prompt block */
void hacker_rewind_prompt(void);

/* Random delay for "realism" */
void hacker_delay(int minMs, int maxMs);

/* Beep sound effect */
void hacker_beep(void);

#endif /* HACKER_H */
