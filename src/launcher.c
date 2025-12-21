/*
 * launcher.c - External Process Launcher Module Implementation
 * Person 4: Chạy chương trình ngoài (foreground/background)
 * 
 * ⚠️ FILE NÀY CẦN ĐƯỢC IMPLEMENT BỞI PERSON 4
 */

#include "launcher.h"
#include "process_manager.h"
#include <string.h>
/*============================================================
 * HELPER FUNCTIONS
 *============================================================*/

#define MSH_CONTINUE 1
#define MSH_EXIT 0

extern void add_bg_process(DWORD pid);
extern HANDLE hForegroundProcess;

int is_batch_file(const char *filename) {
    /*
     * TODO: Kiểm tra xem file có phải là batch file (.bat, .cmd) không
     * 
     * Return: 1 nếu là batch file, 0 nếu không
     * 
     * Hint: Kiểm tra extension bằng _stricmp()
     */
    
    if(filename == NULL) {
        return 0;
    }

    const char *dot = strrchr(filename, '.');
    if(dot != NULL) {
        if(_stricmp(dot, ".bat") == 0 || _stricmp(dot, ".cmd") == 0) {
            return 1;
        }
    }
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
    int i = 0;

    while(args[i] != NULL) i++;
    int arg_count = i;
    if(arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) {
        background = 1;
        args[arg_count - 1] = NULL;
    }

    if(args[0] == NULL) {
        return MSH_CONTINUE;
    }

    if(is_batch_file(args[0])) {
        strcpy(command, "cmd /c ");
        strcat(command, args[0]);
        for(i = 1; args[i] != NULL; i++) {
            strcat(command, " ");
            strcat(command, args[i]);
        }
    } else {
        for(i = 0; args[i] != NULL; i++) {
            strcat(command, args[i]);
            if(args[i + 1] != NULL) {
                strcat(command, " ");
            }
        }
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if(!CreateProcess(NULL, command, NULL, NULL, FALSE,
                      CREATE_NEW_PROCESS_GROUP,
                      NULL, NULL, &si, &pi)) {
        printf("Failed to create process\n");
        return MSH_CONTINUE;
    }

    if(background) {
        printf("[Started process %lu]\n", pi.dwProcessId);
        add_bg_process(pi.dwProcessId);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        hForegroundProcess = pi.hProcess;
        while(hForegroundProcess != NULL) {
            WaitForSingleObject(pi.hProcess, 100);
            DWORD exitCode;
            if(GetExitCodeProcess(pi.hProcess, &exitCode) && exitCode != STILL_ACTIVE) {
                break;
            }
        }
        hForegroundProcess = NULL;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    
    return MSH_CONTINUE;
}

void print_win_error(const char *prefix) {
    DWORD errorMessageID = GetLastError();
    if(errorMessageID == 0) {
        return;
    }

    LPSTR messageBuffer = NULL;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer, 0, NULL);

    fprintf(stderr, "%s: %s\n", prefix, messageBuffer);

    LocalFree(messageBuffer);
}


