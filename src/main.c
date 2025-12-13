/*
 * main.c - Entry Point for MSH Shell
 * 
 * Đây là file chính để khởi động shell.
 * Chỉ chứa hàm main() và các bước khởi tạo.
 */

#include "core.h"
#include "process_manager.h"

/*============================================================
 * MAIN FUNCTION
 *============================================================*/

int main(int argc, char **argv) {
    /* Suppress unused parameter warnings */
    (void)argc;
    (void)argv;
    
    /* 1. Đăng ký Ctrl+C Handler */
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        fprintf(stderr, "Warning: Could not set control handler\n");
    }
    
    /* 2. Khởi tạo Process Manager */
    init_process_manager();
    
    /* 3. Hiển thị welcome message */
    printf("\n");
    printf("==========================================\n");
    printf("  MSH - Tiny Shell for Windows\n");
    printf("  Type 'help' for available commands\n");
    printf("==========================================\n");
    printf("\n");
    
    /* 4. Chạy shell loop */
    msh_loop();
    
    return EXIT_SUCCESS;
}
