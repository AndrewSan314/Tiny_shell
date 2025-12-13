/*
 * launcher.h - External Process Launcher Module
 * Person 4: Chạy chương trình ngoài (foreground/background)
 */

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "../include/common.h"

/*============================================================
 * FUNCTIONS
 *============================================================*/

/**
 * msh_launch - Chạy một chương trình bên ngoài
 * @param args: Mảng các argument (args[0] là tên chương trình)
 * @return: MSH_CONTINUE để tiếp tục shell loop
 * 
 * Hỗ trợ:
 * - Foreground mode: đợi đến khi chương trình kết thúc
 * - Background mode: thêm '&' ở cuối để chạy ngầm
 * - Batch files: tự động nhận diện .bat và chạy qua cmd /c
 */
int msh_launch(char **args);

/**
 * is_batch_file - Kiểm tra xem file có phải batch file không
 * @param filename: Tên file cần kiểm tra
 * @return: 1 nếu là .bat/.cmd, 0 nếu không
 */
int is_batch_file(const char *filename);

#endif /* LAUNCHER_H */
