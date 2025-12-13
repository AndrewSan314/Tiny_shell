/*
 * launcher.c - External Process Launcher Module Implementation
 * Person 4: Chạy chương trình ngoài (foreground/background)
 * 
 * ⚠️ FILE NÀY CẦN ĐƯỢC IMPLEMENT BỞI PERSON 4
 */

#include "launcher.h"
#include "process_manager.h"

/*============================================================
 * HELPER FUNCTIONS
 *============================================================*/

int is_batch_file(const char *filename) {
    /*
     * TODO: Kiểm tra xem file có phải là batch file (.bat, .cmd) không
     * 
     * Return: 1 nếu là batch file, 0 nếu không
     * 
     * Hint: Kiểm tra extension bằng _stricmp()
     */
    
    (void)filename;
    return 0;
}

/*============================================================
 * MAIN LAUNCH FUNCTION
 *============================================================*/

int msh_launch(char **args) {
    /*
     * TODO: Chạy một chương trình bên ngoài
     * 
     * Các bước chính:
     * 
     * 1. KIỂM TRA BACKGROUND MODE
     *    - Đếm số argument
     *    - Nếu argument cuối là "&", đây là background mode
     *    - Xóa "&" khỏi danh sách argument
     * 
     * 2. XÂY DỰNG COMMAND STRING
     *    - Nếu là batch file (.bat), thêm "cmd /c " vào đầu
     *    - Nối tất cả args thành một chuỗi
     * 
     * 3. TẠO PROCESS
     *    - Khởi tạo STARTUPINFO và PROCESS_INFORMATION
     *    - Gọi CreateProcess() với flag CREATE_NEW_PROCESS_GROUP
     *      (flag này giúp Ctrl+C không gửi trực tiếp đến child)
     * 
     * 4. XỬ LÝ THEO MODE
     *    
     *    Nếu BACKGROUND:
     *    - In "[Started process PID]"
     *    - Gọi add_bg_process() để lưu vào danh sách
     *    
     *    Nếu FOREGROUND:
     *    - Set hForegroundProcess = pi.hProcess
     *    - Đợi process kết thúc bằng polling loop:
     *      while (hForegroundProcess != NULL) {
     *          WaitForSingleObject(pi.hProcess, 100);
     *          if (process ended) break;
     *      }
     *    - Set hForegroundProcess = NULL
     *    - CloseHandle() cho hProcess và hThread
     */
    
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char command[MAX_CMD_LEN] = {0};
    int background = 0;
    
    /* TODO: Implement các bước ở trên */
    
    /* Placeholder - xóa khi implement xong */
    (void)si;
    (void)pi;
    (void)command;
    (void)background;
    (void)args;
    
    printf("TODO: msh_launch not implemented yet\n");
    
    return MSH_CONTINUE;
}
