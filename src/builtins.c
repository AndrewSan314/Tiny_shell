/*
 * builtins.c - Built-in Commands Module Implementation
 * Person 3: Các lệnh nội bộ của shell
 * 
 * ⚠️ FILE NÀY CẦN ĐƯỢC IMPLEMENT BỞI PERSON 3
 */

#include "builtins.h"
#include "process_manager.h"

/*============================================================
 * BUILTIN REGISTRY - ĐÃ HOÀN THÀNH, KHÔNG CẦN SỬA
 *============================================================*/

char *builtin_str[] = {
    "cd", "pwd", "dir", "date", "time", "cls", 
    "help", "exit", "path", "addpath",
    "list", "kill", "stop", "resume"
};

int (*builtin_func[])(char **) = {
    &msh_cd, &msh_pwd, &msh_dir, &msh_date, &msh_time, &msh_cls,
    &msh_help, &msh_exit, &msh_path, &msh_addpath,
    &msh_list, &msh_kill, &msh_stop, &msh_resume
};

int msh_num_builtins(void) {
    return sizeof(builtin_str) / sizeof(char *);
}

/*============================================================
 * NAVIGATION COMMANDS - PERSON 3 IMPLEMENT
 *============================================================*/

int msh_cd(char **args) {
    /*
     * TODO: Đổi thư mục hiện tại
     * 
     * Usage: cd <directory>
     * 
     * Hint: Dùng SetCurrentDirectory(args[1])
     * Kiểm tra args[1] != NULL trước khi gọi
     */
    
    (void)args;
    return MSH_CONTINUE;
}

int msh_pwd(char **args) {
    /*
     * TODO: In thư mục hiện tại
     * 
     * Hint: Dùng GetCurrentDirectory(MAX_PATH, buffer)
     */
    
    (void)args;
    return MSH_CONTINUE;
}

int msh_dir(char **args) {
    /*
     * TODO: Liệt kê files trong thư mục
     * 
     * Usage: dir [path]
     * 
     * Hint đơn giản: Dùng system("dir") hoặc system("dir path")
     * Hint nâng cao: Dùng FindFirstFile/FindNextFile API
     */
    
    (void)args;
    return MSH_CONTINUE;
}

/*============================================================
 * UTILITY COMMANDS - PERSON 3 IMPLEMENT
 *============================================================*/

int msh_date(char **args) {
    /*
     * TODO: Hiển thị ngày hiện tại
     * 
     * Hint đơn giản: system("date /t")
     * Hint nâng cao: Dùng GetLocalTime() API
     */
    
    (void)args;
    return MSH_CONTINUE;
}

int msh_time(char **args) {
    /*
     * TODO: Hiển thị giờ hiện tại
     */
    
    (void)args;
    return MSH_CONTINUE;
}

int msh_cls(char **args) {
    /*
     * TODO: Xóa màn hình
     * 
     * Hint: system("cls")
     */
    
    (void)args;
    return MSH_CONTINUE;
}

int msh_help(char **args) {
    /*
     * TODO: Hiển thị help message
     * 
     * In ra danh sách các lệnh và mô tả ngắn gọn
     */
    
    (void)args;
    printf("MSH Shell - Type commands and press Enter\n");
    printf("TODO: Add more help content...\n");
    return MSH_CONTINUE;
}

int msh_exit(char **args) {
    /*
     * TODO: Thoát shell
     * 
     * Return MSH_EXIT (= 0) để thoát vòng lặp
     */
    
    (void)args;
    return MSH_EXIT;
}

/*============================================================
 * ENVIRONMENT COMMANDS - PERSON 3 IMPLEMENT
 *============================================================*/

int msh_path(char **args) {
    /*
     * TODO: Hiển thị biến môi trường PATH
     * 
     * Hint: Dùng GetEnvironmentVariable("PATH", buffer, size)
     * Lưu ý: PATH có thể rất dài (32KB), cần malloc
     */
    
    (void)args;
    return MSH_CONTINUE;
}

int msh_addpath(char **args) {
    /*
     * TODO: Thêm thư mục vào PATH
     * 
     * Usage: addpath <directory>
     * 
     * Steps:
     * 1. Đọc PATH hiện tại bằng GetEnvironmentVariable()
     * 2. Nối thêm ";directory" vào cuối
     * 3. Set lại bằng SetEnvironmentVariable()
     */
    
    (void)args;
    return MSH_CONTINUE;
}
