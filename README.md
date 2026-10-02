# Memory Card Game

Hello, đây là một game nho nhỏ mình code, vui vẻ thôi. Mục đích là chạy được trên Windows 95.

Một trò chơi lật thẻ ghi nhớ đơn giản được viết bằng **C++ và Win32 API**.

Project được mình thực hiện để luyện tập lập trình C++, làm quen với ứng dụng desktop trên Windows và hiểu rõ hơn cách xây dựng một chương trình có giao diện, tương tác chuột và quản lý trạng thái game.

## Demo

Game có bàn chơi **4x3**, gồm **12 thẻ và 6 cặp**.

Nhiệm vụ của người chơi là lật các thẻ và tìm tất cả các cặp giống nhau.

Nếu hai thẻ không khớp, chúng sẽ tự động úp lại sau một khoảng thời gian ngắn.

## Tính năng

- Bàn chơi 4x3 với 6 cặp thẻ.
- Xáo trộn vị trí thẻ khi bắt đầu game.
- Click chuột để lật thẻ.
- Tự động kiểm tra các cặp giống nhau.
- Hiệu ứng đơn giản khi tìm đúng cặp.
- Thông báo khi hoàn thành trò chơi.
- Có thể bắt đầu game mới bằng phím `N`.

## Công nghệ sử dụng

- C++
- Win32 API
- Windows GDI

Project được viết trực tiếp bằng Win32 API và không sử dụng game engine hoặc GUI framework bên ngoài.

## Chạy project

### Yêu cầu

- Windows
- Trình biên dịch C++

Ví dụ với MinGW:

```bash
git clone https://github.com/TRANDUYANHHH/gamecak.git
cd gamecak

g++ game.cpp -o MemoryGame.exe -mwindows
```

Sau đó chạy:

```bash
MemoryGame.exe
```

Bạn cũng có thể mở source code bằng Visual Studio và build trực tiếp trên Windows.

## Cấu trúc project

```text
gamecak/
└── game.cpp
```

Hiện tại toàn bộ source code của game được đặt trong `game.cpp`.

## Những gì mình học được

Thông qua project này, mình có cơ hội thực hành:

- Lập trình C++.
- Xây dựng ứng dụng desktop trên Windows.
- Xử lý chuột và bàn phím.
- Làm việc với giao diện bằng Win32 API.
- Quản lý trạng thái của một chương trình.
- Tổ chức logic cho một game nhỏ.

Đây là một project khá đơn giản, nhưng giúp mình hiểu rõ hơn cách một ứng dụng GUI hoạt động phía sau thay vì chỉ sử dụng các framework có sẵn.

## Hướng phát triển

Trong tương lai mình muốn bổ sung thêm:

- Bộ đếm thời gian.
- Bộ đếm số lượt.
- Nhiều mức độ khó.
- Hình ảnh thay cho các ký hiệu đơn giản.
- Animation đẹp hơn.
- High Score.
- Âm thanh.
- Cải thiện giao diện.

## Thành viên

Project được thực hiện bởi:

- Vu Do Phuong Dong
- Nguyen Xuan Duc
- Tran Duy Anh

## GitHub

GitHub của mình:

[TRANDUYANHHH](https://github.com/TRANDUYANHHH)

Repository:

[gamecak](https://github.com/TRANDUYANHHH/gamecak)
