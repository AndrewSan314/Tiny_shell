/*
 * common.h - Shared definitions for MSH Shell
 * Dùng chung cho tất cả các module
 */

#ifndef COMMON_H
#define COMMON_H

/* Tắt cảnh báo unsafe functions trên Windows */
#define _CRT_SECURE_NO_WARNINGS

/* Windows API */
#include <windows.h>
#include <tlhelp32.h>

/* Standard C Library */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*============================================================
 * CONSTANTS - Các hằng số dùng chung
 *============================================================*/

#define MAX_BG_PROCS    100     /* Số lượng process background tối đa */
#define MAX_CMD_LEN     1024    /* Độ dài tối đa của một command */
#define MSH_RL_BUFSIZE  1024    /* Buffer size cho readline */
#define MSH_TOK_BUFSIZE 64      /* Số lượng token tối đa */
#define MSH_TOK_DELIM   " \t\r\n\a"  /* Ký tự phân cách token */
#define POLL_INTERVAL   100     /* Thời gian polling (ms) */

/*============================================================
 * STRUCTURES - Các cấu trúc dữ liệu
 *============================================================*/

/**
 * ProcessInfo - Thông tin về một tiến trình background
 * Được quản lý bởi process_manager module
 */
typedef struct {
    DWORD pid;              /* Process ID */
    HANDLE hProcess;        /* Handle của process */
    HANDLE hThread;         /* Handle của main thread (để stop/resume) */
    char cmd[MAX_CMD_LEN];  /* Tên lệnh đã chạy */
    int is_active;          /* 1: đang chạy, 0: đã xong/trống */
    int is_suspended;       /* 1: đang tạm dừng, 0: đang chạy */
} ProcessInfo;

/*============================================================
 * RETURN CODES - Mã trả về
 *============================================================*/

#define MSH_CONTINUE 1  /* Tiếp tục vòng lặp shell */
#define MSH_EXIT     0  /* Thoát shell */

#endif /* COMMON_H */
