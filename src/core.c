/*
 * core.c - Core Shell Module Implementation
 * Person 1: Vòng lặp chính và parser
 * 
 * ⚠️ FILE NÀY CẦN ĐƯỢC IMPLEMENT BỞI PERSON 1
 */

#include "core.h"
#include "builtins.h"
#include "launcher.h"
#include "process_manager.h"

/*============================================================
 * INPUT FUNCTIONS
 *============================================================*/

char *msh_read_line(void) {
    /*
     * TODO: Đọc một dòng input từ người dùng
     * 
     * Steps:
     * 1. Cấp phát buffer với malloc()
     * 2. Đọc từng ký tự bằng getchar() cho đến khi gặp '\n' hoặc EOF
     * 3. Thêm '\0' vào cuối
     * 4. Return buffer (caller sẽ free)
     * 
     * Lưu ý: Xử lý buffer overflow nếu input quá dài!
     */
    
    int buff_size = MSH_RL_BUFSIZE;
    char *buffer = malloc(sizeof(char) * buff_size);
    int position = 0;
    int character;

    if (!buffer) {
        fprintf(stderr, "Memory error\n");
        exit(EXIT_FAILURE);
    }
    while(1) {
        character = getchar(); // Đọc input từ người dùng
        if(character == EOF || character == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        buffer[position] = character;
        position += 1;

        // Xử lý buffer overflow
        if(position >= buff_size - 1) {
            buff_size += MSH_RL_BUFSIZE;
            buffer = realloc(buffer, buff_size);
            if (!buffer) { fprintf(stderr, "Memory error\n"); exit(EXIT_FAILURE); }
        }
    }
}

char **msh_split_line(char *line) {
    /*
     * TODO: Tách chuỗi thành mảng các token
     * 
     * Input: "ls -l /home"
     * Output: ["ls", "-l", "/home", NULL]
     * 
     * Hint: Dùng strtok(line, MSH_TOK_DELIM)
     * 
     * Lưu ý: 
     * - Mảng phải kết thúc bằng NULL
     * - Xử lý trường hợp có nhiều token hơn buffer size
     */
    
    int buff_size = MSH_TOK_BUFSIZE;
    char **tokens = malloc(buff_size * sizeof(char *));
    char *token;
    int position = 0;

    if (!tokens) {
        fprintf(stderr, "Memory error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, MSH_TOK_DELIM);

    while(token != NULL) {
        tokens[position] = token;
        position += 1;

        if(position >= buff_size) {
            buff_size += MSH_TOK_BUFSIZE;
            tokens = realloc(tokens, buff_size * sizeof(char *));
            if (!tokens) { fprintf(stderr, "Memory error\n"); exit(EXIT_FAILURE); }
        }

        token = strtok(NULL, MSH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

/*============================================================
 * EXECUTION FUNCTIONS
 *============================================================*/

int msh_execute(char **args) {
    /*
     * TODO: Thực thi một lệnh
     * 
     * Steps:
     * 1. Nếu args[0] == NULL (lệnh rỗng), return MSH_CONTINUE
     * 2. Kiểm tra xem có phải builtin command không:
     *    - Duyệt qua builtin_str[]
     *    - So sánh bằng _stricmp() (case-insensitive)
     *    - Nếu match, gọi builtin_func[i](args)
     * 3. Nếu không phải builtin, gọi msh_launch(args)
     */
    
    if (args[0] == NULL) {
        return MSH_CONTINUE;
    }
    for(int i = 0; i < msh_num_builtins(); i++) {
        if(_stricmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return msh_launch(args);
}

/*============================================================
 * MAIN LOOP
 *============================================================*/

void msh_loop(void) {
    /*
     * TODO: Vòng lặp chính của shell
     * 
     * Repeat:
     * 1. cleanup_zombies() - dọn dẹp background processes đã xong
     * 2. In prompt "msh> "
     * 3. Đọc input bằng msh_read_line()
     * 4. Parse input bằng msh_split_line()
     * 5. Thực thi bằng msh_execute()
     * 6. Free memory
     * 7. Nếu status == MSH_EXIT (0), thoát loop
     */
    
    char *line;
    char **args;
    int status;

    do {
        /* TODO: Implement shell loop */
        cleanup_zombies();
        printf("msh> ");
        fflush(stdout);

        line = msh_read_line();
        args = msh_split_line(line);
        status = msh_execute(args);

        free(line);
        free(args);

    } while (status);
}
