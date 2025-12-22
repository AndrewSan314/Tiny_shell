/*
 * process_manager.c - Process Management Module Implementation
 * Person 2: Quản lý tiến trình background và Ctrl+C
 * 
 * ⚠️ FILE NÀY CẦN ĐƯỢC IMPLEMENT BỞI PERSON 2
 */

#include "process_manager.h"

/*============================================================
 * GLOBAL VARIABLES
 *============================================================*/

ProcessInfo bg_procs[MAX_BG_PROCS];
HANDLE hForegroundProcess = NULL;

/*============================================================
 * INITIALIZATION
 *============================================================*/

void init_process_manager(void) {
    /*
     * TODO: Khởi tạo danh sách process
     * - Set tất cả bg_procs[i].is_active = 0
     * - Set hForegroundProcess = NULL
     */
     for(int i = 0; i < MAX_BG_PROCS; i++){ 
        bg_procs[i].is_active = 0; 
     }
     hForegroundProcess = NULL;
}

/*============================================================
 * PROCESS MANAGEMENT FUNCTIONS
 *============================================================*/

void add_bg_process(DWORD pid, HANDLE hProc, HANDLE hThread, const char *cmd) {
    /*
     * TODO: Thêm một process vào danh sách background
     * 
     * Steps:
     * 1. Tìm slot trống trong bg_procs[] (is_active == 0)
     * 2. Lưu pid, hProcess, hThread, cmd vào slot đó
     * 3. Set is_active = 1, is_suspended = 0
     * 4. Nếu không có slot trống, in "Process list full!"
     * 
     * Hint: Dùng strncpy() để copy cmd an toàn
     */
     for(int i = 0; i < MAX_BG_PROCS; i++){ 
        if(bg_procs[i].is_active == 0){ 
            bg_procs[i].pid = pid;
            bg_procs[i].hProcess = hProc;
            bg_procs[i].hThread = hThread;
            strncpy(bg_procs[i].cmd, cmd, MAX_CMD_LEN);
            bg_procs[i].is_active = 1;
            bg_procs[i].is_suspended = 0;
            return;
        }
     }
     printf("Process list full!\n");
}

void cleanup_zombies(void) {
    /*
     * TODO: Dọn dẹp các process đã kết thúc
     * 
     * Steps:
     * 1. Duyệt qua tất cả bg_procs[]
     * 2. Với mỗi slot active, dùng GetExitCodeProcess() để kiểm tra
     * 3. Nếu exitCode != STILL_ACTIVE:
     *    - CloseHandle(hProcess) và CloseHandle(hThread)
     *    - Set is_active = 0
     */
     DWORD exitCode;
     for(int i = 0; i < MAX_BG_PROCS; i++){ 
        if(bg_procs[i].is_active == 1){ 
            if(GetExitCodeProcess(bg_procs[i].hProcess, &exitCode) && exitCode != STILL_ACTIVE){ 
                CloseHandle(bg_procs[i].hProcess); 
                CloseHandle(bg_procs[i].hThread); 
                bg_procs[i].is_active = 0;
            }
        }
     }
}

/*============================================================
 * CTRL+C HANDLER
 *============================================================*/

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    /*
     * TODO: Xử lý tín hiệu Ctrl+C
     * 
     * Steps:
     * 1. Kiểm tra fdwCtrlType == CTRL_C_EVENT
     * 2. Nếu hForegroundProcess != NULL:
     *    - Gọi TerminateProcess(hForegroundProcess, 1)
     *    - Set hForegroundProcess = NULL
     *    - In thông báo đã terminate
     * 3. Nếu hForegroundProcess == NULL:
     *    - In thông báo không có process foreground
     * 4. Return TRUE để chặn việc thoát shell
     */
     if(fdwCtrlType != CTRL_C_EVENT) {
         return FALSE; // Không xử lý các event khác
     }
     
     if(hForegroundProcess != NULL){ 
        TerminateProcess(hForegroundProcess, 1);
        hForegroundProcess = NULL;
        printf("Process terminated.\n");
        return TRUE;
     }
     printf("No foreground process.\n");
    
    return FALSE; /* TODO: Sửa return value */
}

/*============================================================
 * BUILTIN COMMANDS - Process Control
 *============================================================*/

int msh_list(char **args) {
    (void)args;
    
    /*
     * TODO: Liệt kê tất cả background processes
     * 
     * Output format:
     * PID        Status     Command
     * ------------------------------------------
     * 1234       Running    notepad.exe
     * 5678       Stopped    calc.exe
     * 
     * Hint: 
     * - Gọi cleanup_zombies() trước
     * - Dùng printf("%-10lu %-10s %.20s\n", ...) để format
     * - is_suspended ? "Stopped" : "Running"
     */
     cleanup_zombies();
     int cnt = 0;
     printf("%-10s %-10s %-20s\n", "PID", "Status", "Command");
     for(int i = 0; i < MAX_BG_PROCS; i++){ 
        if(bg_procs[i].is_active){ 
            printf("%-10lu %-10s %.20s\n", bg_procs[i].pid, bg_procs[i].is_suspended ? "Stopped" : "Running", bg_procs[i].cmd);
            cnt++;
        }
     }
     if(cnt == 0){ 
        printf("No background processes found.\n");
     }
    
    return MSH_CONTINUE;
}

int msh_kill(char **args) {
    /*
     * TODO: Kết thúc một process theo PID
     * 
     * Usage: kill <pid>
     * 
     * Steps:
     * 1. Kiểm tra args[1] != NULL, nếu NULL thì in usage
     * 2. Convert args[1] sang DWORD bằng atoi()
     * 3. Tìm process trong bg_procs[] theo pid
     * 4. Gọi TerminateProcess()
     * 5. In thông báo success hoặc not found
     */
    (void)args;
    if(args[1] == NULL){
        printf("usage: kill <pid>\n");
        return 1; 
    }
    DWORD pid = (DWORD)atoi(args[1]); 
    for(int i = 0; i < MAX_BG_PROCS; i++){ 
        if(bg_procs[i].is_active && bg_procs[i].pid == pid){ 
            TerminateProcess(bg_procs[i].hProcess, 1);
            bg_procs[i].is_active = 0;
            printf("Process %lu terminated.\n", pid);
            return 0;
        }
    }
    printf("Process %lu not found.\n", pid);
    return 1;
}

int msh_stop(char **args) {
    /*
     * TODO: Tạm dừng (suspend) một process
     * 
     * Usage: stop <pid>
     * 
     * Hint: Dùng SuspendThread(hThread)
     * Nhớ set is_suspended = 1 sau khi suspend thành công
     */
    
    (void)args;
    if(args[1] == NULL){
        printf("Usage: stop <pid>\n");
        return 1; 
    }
    DWORD pid = (DWORD)atoi(args[1]); 
    for(int i = 0; i < MAX_BG_PROCS; i++){ 
        if(bg_procs[i].is_active && bg_procs[i].pid == pid){ 
            SuspendThread(bg_procs[i].hThread);
            bg_procs[i].is_suspended = 1;   
            printf("Process %lu stopped.\n", pid);
            return 0;
        }
    }
    printf("Process %lu not found.\n", pid);
    return 1;
}

int msh_resume(char **args) {
    /*
     * TODO: Tiếp tục (resume) một process đã bị suspend
     * 
     * Usage: resume <pid>
     * 
     * Hint: Dùng ResumeThread(hThread)
     * Nhớ set is_suspended = 0 sau khi resume thành công
     */
    
    (void)args;
    if(args[1] == NULL){
        printf("Usage: resume <pid>\n");
        return 1; 
    }
    DWORD pid = (DWORD)atoi(args[1]); 
    for(int i = 0; i < MAX_BG_PROCS; i++){ 
        if(bg_procs[i].is_active && bg_procs[i].pid == pid){ 
            ResumeThread(bg_procs[i].hThread);
            bg_procs[i].is_suspended = 0;   
            printf("Process %lu resumed.\n", pid);
            return 0;
        }
    }
    printf("Process %lu not found.\n", pid);
    return 1;
}
