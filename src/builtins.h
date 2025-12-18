/*
 * builtins.h - Built-in Commands Module
 * Person 3: Các lệnh nội bộ của shell
 */

#ifndef BUILTINS_H
#define BUILTINS_H

#include "../include/common.h"

/*============================================================
 * BUILTIN REGISTRY - Đăng ký lệnh nội bộ
 *============================================================*/

extern char *builtin_str[];            /* Mảng tên các lệnh */
extern int (*builtin_func[])(char **); /* Mảng con trỏ hàm */

/**
 * msh_num_builtins - Trả về số lượng lệnh nội bộ
 */
int msh_num_builtins(void);

/*============================================================
 * NAVIGATION COMMANDS - Lệnh điều hướng
 *============================================================*/

int msh_cd(char **args);  /* cd <dir> - Đổi thư mục */
int msh_pwd(char **args); /* pwd - Hiển thị thư mục hiện tại */
int msh_dir(char **args); /* dir [path] - Liệt kê file */

/*============================================================
 * UTILITY COMMANDS - Lệnh tiện ích
 *============================================================*/

int msh_datetime(char **args); /* datetime - Hiển thị ngày giờ */
int msh_cls(char **args);      /* cls - Xóa màn hình */
int msh_help(char **args);     /* help - Hiển thị trợ giúp */
int msh_exit(char **args);     /* exit - Thoát shell */

/*============================================================
 * ENVIRONMENT COMMANDS - Lệnh môi trường
 *============================================================*/

int msh_path(char **args);    /* path - Hiển thị PATH */
int msh_addpath(char **args); /* addpath <dir> - Thêm vào PATH */

/*============================================================
 * ADVANCED COMMANDS - Lệnh nâng cao
 *============================================================*/

int msh_systeminfo(char **args); /* systeminfo - Thông tin hệ thống */
int msh_grep(char **args);   /* grep <pattern> <file> - Tìm kiếm trong file */
int msh_search(char **args); /* search <pattern> - Tìm file */
int msh_diff(char **args);   /* diff <file1> <file2> - So sánh 2 file */

#endif /* BUILTINS_H */
