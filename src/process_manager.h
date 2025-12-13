/*
 * process_manager.h - Process Management Module
 * Person 2: Quản lý tiến trình background và Ctrl+C
 */

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "../include/common.h"

/*============================================================
 * GLOBAL VARIABLES - Biến toàn cục
 *============================================================*/

extern ProcessInfo bg_procs[MAX_BG_PROCS];  /* Danh sách process background */
extern HANDLE hForegroundProcess;            /* Handle của foreground process */

/*============================================================
 * FUNCTIONS - Các hàm public
 *============================================================*/

/**
 * init_process_manager - Khởi tạo module quản lý tiến trình
 * Gọi một lần trong main() trước khi chạy shell loop
 */
void init_process_manager(void);

/**
 * add_bg_process - Thêm một process vào danh sách background
 * @param pid: Process ID
 * @param hProc: Handle của process
 * @param hThread: Handle của main thread
 * @param cmd: Tên lệnh đã chạy
 */
void add_bg_process(DWORD pid, HANDLE hProc, HANDLE hThread, const char *cmd);

/**
 * cleanup_zombies - Dọn dẹp các process đã kết thúc
 * Gọi trước mỗi lần hiển thị prompt
 */
void cleanup_zombies(void);

/**
 * CtrlHandler - Xử lý tín hiệu Ctrl+C
 * Được đăng ký với SetConsoleCtrlHandler()
 */
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

/*============================================================
 * BUILTIN COMMANDS - Các lệnh nội bộ về process
 *============================================================*/

int msh_list(char **args);    /* list - Liệt kê background processes */
int msh_kill(char **args);    /* kill <pid> - Kết thúc process */
int msh_stop(char **args);    /* stop <pid> - Tạm dừng process */
int msh_resume(char **args);  /* resume <pid> - Tiếp tục process */

#endif /* PROCESS_MANAGER_H */
