/*
 * core.h - Core Shell Module
 * Person 1: Vòng lặp chính và parser
 */

#ifndef CORE_H
#define CORE_H

#include "../include/common.h"

/*============================================================
 * FUNCTIONS
 *============================================================*/

/**
 * msh_loop - Vòng lặp chính của shell
 * Đọc input -> Parse -> Execute -> Repeat
 */
void msh_loop(void);

/**
 * msh_read_line - Đọc một dòng input từ người dùng
 * @return: Chuỗi đã đọc (cần free sau khi dùng)
 */
char *msh_read_line(void);

/**
 * msh_split_line - Tách chuỗi thành mảng token
 * @param line: Chuỗi cần tách
 * @return: Mảng các token (cần free sau khi dùng)
 */
char **msh_split_line(char *line);

/**
 * msh_execute - Thực thi một lệnh
 * @param args: Mảng các argument
 * @return: MSH_CONTINUE hoặc MSH_EXIT
 */
int msh_execute(char **args);

#endif /* CORE_H */
