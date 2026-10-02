# Memory Card Game 🎴
Hello mọi người, đây là một game vui vẻ mình code chạy được trên Windows 95 hihi.

Một trò chơi **Memory Matching** đơn giản được xây dựng hoàn toàn bằng **C++ và Win32 API**, không sử dụng game engine hay GUI framework bên ngoài.

Người chơi lật từng cặp thẻ và cố gắng tìm tất cả các cặp giống nhau. Dự án được thực hiện nhằm tìm hiểu cách xây dựng ứng dụng desktop trên Windows, xử lý sự kiện, quản lý trạng thái chương trình và vẽ giao diện trực tiếp bằng Win32 API.

---

## 🎮 Demo

Game sử dụng bàn chơi kích thước:

```text
4 x 3
```

gồm:

```text
12 cards
6 matching pairs
```

Mỗi lần bắt đầu game, vị trí các cặp thẻ sẽ được xáo trộn ngẫu nhiên.

---

## ✨ Tính năng

- Bàn chơi Memory Card kích thước **4×3**.
- Tổng cộng **12 thẻ / 6 cặp**.
- Random vị trí các thẻ mỗi lần bắt đầu game.
- Click chuột để lật thẻ.
- Kiểm tra hai thẻ có giống nhau hay không.
- Tự động úp lại hai thẻ nếu không khớp.
- Giữ lại các cặp đã ghép thành công.
- Hiệu ứng nhấp nháy khi tìm được một cặp đúng.
- Hiển thị thông báo khi hoàn thành toàn bộ trò chơi.
- Nhấn phím `N` để bắt đầu một game mới.
- Toàn bộ giao diện được vẽ trực tiếp bằng **Windows GDI**.

---

## 🕹️ Cách chơi

1. Click vào một thẻ để lật thẻ đầu tiên.
2. Click vào một thẻ khác để lật thẻ thứ hai.
3. Nếu hai thẻ giống nhau, chúng được đánh dấu là một cặp đã hoàn thành.
4. Nếu hai thẻ khác nhau, chúng sẽ tự động úp lại sau một khoảng thời gian ngắn.
5. Tiếp tục cho đến khi tìm được toàn bộ 6 cặp.

Khi hoàn thành game, chương trình sẽ hiển thị thông báo chiến thắng.

Nhấn:

```text
N
```

để bắt đầu một game mới.

---

## 🛠️ Công nghệ sử dụng

### Programming Language

```text
C++
```

### API

```text
Win32 API
Windows GDI
```

Dự án không sử dụng:

- Qt
- SDL
- SFML
- DirectX
- Game Engine

Toàn bộ cửa sổ, event loop, input và rendering được xử lý trực tiếp bằng Win32 API.

---

## 🧠 Các kiến thức được áp dụng

### Event-driven Programming

Ứng dụng hoạt động dựa trên cơ chế message của Windows.

Các sự kiện chính được xử lý bao gồm:

```cpp
WM_CREATE
WM_LBUTTONDOWN
WM_TIMER
WM_PAINT
WM_KEYDOWN
WM_DESTROY
```

Mỗi event đảm nhiệm một phần khác nhau của game như:

- xử lý click chuột;
- cập nhật trạng thái thẻ;
- vẽ giao diện;
- xử lý animation;
- restart game.

---

## 🪟 Windows Message Loop

Game sử dụng message loop chuẩn của Win32:

```text
GetMessage
    ↓
TranslateMessage
    ↓
DispatchMessage
    ↓
WndProc
```

`WndProc` đóng vai trò trung tâm để xử lý các message được Windows gửi tới ứng dụng.

---

## 🃏 Quản lý trạng thái thẻ

Mỗi thẻ được biểu diễn bởi một cấu trúc `Card`.

```cpp
typedef struct Card {
    RECT rect;
    int value;
    BOOL flipped;
    BOOL matched;
} Card;
```

Trong đó:

| Thuộc tính | Ý nghĩa |
|---|---|
| `rect` | Vị trí và kích thước của thẻ |
| `value` | Giá trị dùng để xác định cặp |
| `flipped` | Thẻ đang được lật hay không |
| `matched` | Thẻ đã tìm được cặp hay chưa |

---

## 🔀 Random các thẻ

Khi bắt đầu game, chương trình tạo 6 cặp:

```text
1 1
2 2
3 3
4 4
5 5
6 6
```

sau đó sử dụng thuật toán shuffle để thay đổi vị trí của các thẻ.

Do đó mỗi lần bắt đầu game sẽ có một layout khác nhau.

---

## 🖱️ Xử lý click

Khi người dùng click chuột, tọa độ click được lấy từ message:

```text
WM_LBUTTONDOWN
```

Chương trình xác định thẻ được click dựa trên tọa độ `(x, y)`.

```text
Mouse Click
     ↓
Get coordinate
     ↓
Find card
     ↓
Flip card
     ↓
Wait for second card
     ↓
Check match
```

---

## 🔎 Kiểm tra cặp thẻ

Sau khi người chơi mở hai thẻ, game so sánh:

```cpp
firstCard.value == secondCard.value
```

### Nếu giống nhau

Hai thẻ được đánh dấu:

```text
matched = true
```

và được giữ ở trạng thái hoàn thành.

### Nếu khác nhau

Game sử dụng Timer để chờ khoảng:

```text
800 ms
```

sau đó tự động úp hai thẻ xuống.

---

## ⏱️ Timer

Win32 Timer được sử dụng để xử lý các hiệu ứng mà không làm treo giao diện.

Game sử dụng timer cho hai mục đích chính.

### Flip back

Nếu hai thẻ không giống nhau:

```text
Card A
Card B
   ↓
Wait ~800 ms
   ↓
Flip both cards back
```

### Match animation

Nếu hai thẻ giống nhau, game tạo hiệu ứng nhấp nháy ngắn trước khi đánh dấu chúng là một cặp hoàn thành.

---

## 🎨 Rendering

Giao diện được vẽ trực tiếp bằng Windows GDI.

Một số API được sử dụng:

```text
FillRect
FrameRect
Ellipse
DrawText
CreateSolidBrush
CreatePen
SelectObject
```

Game không sử dụng hình ảnh bên ngoài mà các thành phần giao diện được vẽ trực tiếp bằng các primitive của GDI.

---

## 🏗️ Luồng hoạt động

```text
                Start Game
                    │
                    ▼
              Create 6 pairs
                    │
                    ▼
                Shuffle
                    │
                    ▼
              Draw 4x3 board
                    │
                    ▼
              Wait for click
                    │
                    ▼
               Flip card
                    │
          ┌─────────┴─────────┐
          │                   │
      First card          Second card
                              │
                              ▼
                         Compare values
                         /            \
                        /              \
                    Match           Not Match
                      │                 │
                      ▼                 ▼
                  Animation         Wait 800ms
                      │                 │
                      ▼                 ▼
                  Mark pair          Flip back
                      │
                      └───────┬─────────┘
                              │
                              ▼
                     All pairs matched?
                         /        \
                       No          Yes
                       │            │
                       ▼            ▼
                   Continue       Victory
```

---

## 📁 Cấu trúc project

Project hiện tại có cấu trúc rất đơn giản:

```text
gamecak/
│
└── game.cpp
```

Toàn bộ logic game, rendering và xử lý Windows event được triển khai trong `game.cpp`.

---

## 🚀 Build & Run

### Yêu cầu

- Windows
- C++ Compiler hỗ trợ Win32 API

Có thể sử dụng:

- Visual Studio / MSVC
- MinGW g++

---

### Build bằng MinGW

Clone repository:

```bash
git clone https://github.com/TRANDUYANHHH/gamecak.git
cd gamecak
```

Compile:

```bash
g++ game.cpp -o MemoryGame.exe -mwindows
```

Sau đó chạy:

```bash
MemoryGame.exe
```

---

### Build bằng Visual Studio

1. Tạo một **Windows Desktop Application** hoặc Empty C++ Project.
2. Thêm `game.cpp` vào project.
3. Build project.
4. Chạy file `.exe`.

Do chương trình sử dụng:

```cpp
WinMain(...)
```

nên application được xây dựng dưới dạng Windows GUI application thay vì console application.

---

## 📚 Những gì mình học được

Thông qua project này, mình đã thực hành:

- C/C++ programming.
- Win32 API.
- Windows message loop.
- Event-driven programming.
- Mouse và keyboard input.
- Windows GDI.
- Timer và animation.
- Quản lý trạng thái ứng dụng.
- Randomization và shuffle.
- Thiết kế logic game.
- Quản lý GDI resources.
- Debug ứng dụng native Windows.

Điểm mình thấy thú vị nhất của project là có thể xây dựng một ứng dụng có giao diện và tương tác hoàn chỉnh chỉ bằng **C++ và Windows API**, mà không cần sử dụng một framework hoặc game engine bên ngoài.

---

## 🔮 Hướng phát triển

Một số tính năng có thể được bổ sung trong tương lai:

- [ ] Bộ đếm số lượt chơi.
- [ ] Bộ đếm thời gian hoàn thành.
- [ ] Hệ thống điểm.
- [ ] Nhiều mức độ khó: 4×3, 4×4, 6×6.
- [ ] Thay số bằng hình ảnh/icon.
- [ ] Animation lật thẻ mượt hơn.
- [ ] Menu bắt đầu game.
- [ ] High Score.
- [ ] Âm thanh.
- [ ] Tách source code thành nhiều module/class.
- [ ] Refactor sang kiến trúc hướng đối tượng.

---

## 👨‍💻 Tác giả

Project được thực hiện bởi:

- Vu Do Phuong Dong
- Nguyen Xuan Duc
- Tran Duy Anh

GitHub:

[TRANDUYANHHH](https://github.com/TRANDUYANHHH)

Repository:

[gamecak](https://github.com/TRANDUYANHHH/gamecak)

---

## 📌 Mục đích

Project được xây dựng với mục đích học tập, thực hành **C++**, tìm hiểu **Windows Programming** và cách xây dựng một ứng dụng GUI từ mức thấp bằng **Win32 API**.
