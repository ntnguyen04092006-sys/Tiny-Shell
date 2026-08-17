# NTN OS Shell Management (TinyShell)

**Tác giả:** Nguyễn Tạo Nguyên (MSSV: 202416307)

NTN OS Shell Management (hay TinyShell) là một chương trình giả lập giao diện dòng lệnh (Shell) cơ bản được viết bằng ngôn ngữ C dành cho hệ điều hành Windows. Chương trình cung cấp các tính năng quản lý tiến trình cốt lõi, bao gồm thực thi lệnh, quản lý tiến trình chạy ngầm/chạy nổi (background/foreground), và tương tác với các biến môi trường.

## Các tính năng chính

### 1. Vòng lặp chính và cơ chế nhận lệnh
*   Hoạt động dựa trên một vòng lặp vô hạn (Infinite Loop) thực hiện chu trình: **Nhận lệnh -> Phân tích -> Thực thi**.
*   Sử dụng `fgets` để đọc lệnh và xử lý chuỗi an toàn.
*   Hỗ trợ trực tiếp các lệnh built-in ngay trong tiến trình của Shell. Nếu là lệnh gọi ngoài, Shell sẽ cấp phát một tiến trình mới.

### 2. Quản lý và khởi tạo tiến trình
*   Khởi tạo tiến trình con thông qua hàm `CreateProcess` của Windows API, quản lý qua Startup Info và Process Information (giữ lại Handles và PIDs).
*   Gán trạng thái (Status) mặc định cho mỗi tiến trình khi sinh ra để theo dõi.

### 3. Chế độ thực thi (Background / Foreground)
*   **Background Mode:** Hỗ trợ chạy ngầm bằng cách thêm ký tự `&` vào cuối câu lệnh (VD: `notepad &`).
*   **Foreground Mode:** Mặc định các lệnh sẽ chạy ở Foreground. Shell sử dụng `WaitForSingleObject()` để tạm dừng vô hạn, chờ tiến trình con kết thúc mới trả lại quyền điều khiển (Prompt).

### 4. Quản lý tiến trình nâng cao
*   `kill <PID>`: Dừng hẳn một tiến trình đang chạy dựa vào Process ID (sử dụng `TerminateProcess` và `OpenProcess`).
*   `stop <PID>`: Tạm ngưng (Suspend) luồng chính của một tiến trình đang chạy (sử dụng `SuspendThread`).
*   `resume <PID>`: Tiếp tục (Resume) một tiến trình đang bị tạm ngưng (sử dụng `ResumeThread`).
*   `list`: Hiển thị danh sách các tiến trình do Shell quản lý dưới dạng bảng định dạng chuẩn gồm: STT, Command Name, PID và Status (Running/Suspended/Stopped).

### 5. Quản lý biến môi trường (PATH)
Người dùng không cần nhập toàn bộ đường dẫn dài để khởi chạy ứng dụng nếu đã nạp thư mục đó vào PATH của Shell:
*   `path`: In ra danh sách các đường dẫn hiện có trong hệ thống.
*   `addpath <đường_dẫn>`: Thêm một đường dẫn thư mục mới vào mảng lưu trữ PATH. Shell sẽ tự động dò tìm file thực thi trong các thư mục này khi nhận lệnh.

### 6. Xử lý tín hiệu ngắt (CTRL+C)
*   Sử dụng cơ chế Callback qua `SetConsoleCtrlHandler` để bắt tín hiệu từ bàn phím.
*   Khi nhấn `CTRL+C`, nếu có một tiến trình Foreground đang chạy, Shell sẽ chỉ ngắt tiến trình con đó (bằng `TerminateProcess`) và **giữ cho Shell gốc tiếp tục hoạt động**, tránh việc toàn bộ hệ thống bị thoát đột ngột.

### 7. Hỗ trợ thực thi file Batch (*.bat)
*   Nhận diện tự động các file có đuôi `.bat`.
*   Tự động gọi `cmd.exe /c` để tiến hành thực thi chuỗi lệnh bên trong file batch của Windows một cách mượt mà.

### 8. Các lệnh Built-in khác
*   `help`: Hiển thị danh sách các lệnh hỗ trợ.
*   `date`: Xem ngày hiện tại của hệ thống.
*   `time`: Xem giờ hiện tại của hệ thống.
*   `exit`: Thoát khỏi NTN OS Shell.

## Hướng dẫn biên dịch và sử dụng

**Yêu cầu:** Môi trường Windows, trình biên dịch C (GCC qua MinGW hoặc MSVC).

1. Biên dịch mã nguồn với GCC:
   ```bash
   gcc -o myshell shell.c
   ```
2. Khởi chạy Shell:
   ```bash
   ./myshell
   ```
3. Tại dấu nhắc lệnh `NTNShell> `, bạn có thể nhập các lệnh hoặc tên chương trình (ví dụ: `notepad`, `calc`, `help`, `list`, `date`, `addpath C:\Tools`).

