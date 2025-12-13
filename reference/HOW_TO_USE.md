# 📚 Hướng Dẫn Tham Khảo Code Gốc

## Cách Làm Việc

1. **Mở file `reference/main_original.c`** để xem code hoàn chỉnh
2. **Tìm phần code liên quan đến mình** (xem bảng bên dưới)
3. **Đọc hiểu logic**
4. **Mở file skeleton trong `src/`**
5. **Tự viết lại** theo cách hiểu của mình

---

## Phân Chia Theo Dòng Code

| Person | File cần làm            | Dòng tham khảo trong `main_original.c`                                   |
| ------ | ----------------------- | ------------------------------------------------------------------------ |
| **1**  | `src/core.c`            | Lines 340-410 (read_line, split_line, execute, loop)                     |
| **2**  | `src/process_manager.c` | Lines 70-180 (CtrlHandler, add_bg, cleanup, list/kill/stop/resume)       |
| **3**  | `src/builtins.c`        | Lines 185-290 (cd, pwd, dir, path, addpath, help, date, time, cls, exit) |
| **4**  | `src/launcher.c`        | Lines 295-340 (msh_launch - CreateProcess, foreground/background)        |

---

## File Structure

```
lsh/
├── reference/
│   └── main_original.c     ← ĐỌC FILE NÀY ĐỂ THAM KHẢO
│
├── src/
│   ├── main.c              ← Đã hoàn thành (không cần sửa)
│   ├── core.c              ← Person 1 code ở đây
│   ├── process_manager.c   ← Person 2 code ở đây
│   ├── builtins.c          ← Person 3 code ở đây
│   └── launcher.c          ← Person 4 code ở đây
│
├── include/
│   └── common.h            ← Shared (không cần sửa)
```

---

## Build & Test

```cmd
cd "d:\C Project\lsh"
.\build.bat
.\msh.exe
```

---

## Lưu Ý Quan Trọng

⚠️ **KHÔNG SỬA file `reference/main_original.c`** - Chỉ đọc để tham khảo

⚠️ **Mỗi người chỉ sửa file của mình** trong thư mục `src/`

⚠️ **Commit thường xuyên** để tránh conflict
