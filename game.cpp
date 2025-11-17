// Memory Card Game 4x4 - Win32 API (Single-file C++)
// - Simple 2D memory matching game (4x4 grid)
// - Uses Win32 GDI for drawing and basic input handling
// - Compile with MSVC: cl /EHsc memory_game.cpp user32.lib gdi32.lib
//   Or with MinGW: g++ memory_game.cpp -municode -lgdi32 -luser32 -o memory_game.exe
//
// Controls: Click a card to flip. Match pairs to remove them. When two flipped cards do not match,
// they will flip back after a short delay.

#include <windows.h>
#include <windowsx.h>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include <sstream>
#include <chrono>

// Game configuration
const int COLS = 4;
const int ROWS = 4;
const int CARD_COUNT = COLS * ROWS;
const int CARD_W = 120;
const int CARD_H = 160;
const int GAP = 12;
const int BORDER = 16;
const int FLIP_BACK_MS = 800; // delay before flipping back on mismatch

struct Card {
    RECT rect;
    int value;      // pair id (1..8)
    bool flipped;
    bool matched;
};

// Global game state
static std::vector<Card> g_cards;
static HWND g_hWnd = nullptr;
static HFONT g_hFont = nullptr;
static int g_flippedCount = 0;
static int g_firstIndex = -1;
static int g_secondIndex = -1;
static bool g_locked = false; // when waiting for flip-back

// Helpers
int IndexFromPoint(int x, int y) {
    for (int i = 0; i < (int)g_cards.size(); ++i) {
        if (PtInRect(&g_cards[i].rect, { x, y })) return i;
    }
    return -1;
}

void StartNewGame() {
    g_cards.clear();
    g_flippedCount = 0;
    g_firstIndex = g_secondIndex = -1;
    g_locked = false;

    // create pair values 1..(CARD_COUNT/2)
    std::vector<int> values;
    for (int v = 1; v <= CARD_COUNT / 2; ++v) {
        values.push_back(v);
        values.push_back(v);
    }
    // shuffle
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(values.begin(), values.end(), rng);

    // compute window content size based on constants
    int width = BORDER * 2 + COLS * CARD_W + (COLS - 1) * GAP;
    int height = BORDER * 2 + ROWS * CARD_H + (ROWS - 1) * GAP + 40; // extra for title

    // prepare card rects
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            int i = r * COLS + c;
            int x = BORDER + c * (CARD_W + GAP);
            int y = BORDER + r * (CARD_H + GAP) + 28; // leave space for title
            RECT rc = { x, y, x + CARD_W, y + CARD_H };
            Card card;
            card.rect = rc;
            card.value = values[i];
            card.flipped = false;
            card.matched = false;
            g_cards.push_back(card);
        }
    }

    // Resize window to fit (optional): set window client size
    if (g_hWnd) {
        RECT client = { 0,0,width,height };
        AdjustWindowRectEx(&client, WS_OVERLAPPEDWINDOW, FALSE, 0);
        SetWindowPos(g_hWnd, NULL, 0, 0, client.right - client.left, client.bottom - client.top, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void DrawCard(HDC hdc, const Card& card) {
    // draw card background
    HBRUSH hBack = CreateSolidBrush(RGB(220, 220, 220));
    FrameRect(hdc, &card.rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    RECT inner = card.rect;
    InflateRect(&inner, -6, -6);

    if (card.matched) {
        // faded matched
        HBRUSH b = CreateSolidBrush(RGB(200, 200, 200));
        FillRect(hdc, &inner, b);
        DeleteObject(b);
    }
    else if (card.flipped) {
        // show face: simple colored rectangle + number
        HBRUSH b = CreateSolidBrush(RGB(240, 240, 255));
        FillRect(hdc, &inner, b);
        DeleteObject(b);

        // draw a colored circle and number to represent the 'face'
        int cx = (inner.left + inner.right) / 2;
        int cy = inner.top + 40;
        int radius = 36;
        HBRUSH colorBrush = CreateSolidBrush(RGB(180 + (card.value * 17) % 75, 90 + (card.value * 31) % 120, 120 + (card.value * 23) % 100));
        HBRUSH old = (HBRUSH)SelectObject(hdc, colorBrush);
        Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
        SelectObject(hdc, old);
        DeleteObject(colorBrush);

        // draw number
        SetBkMode(hdc, TRANSPARENT);
        HFONT oldf = (HFONT)SelectObject(hdc, g_hFont);
        std::wostringstream ss;
        ss << card.value;
        DrawTextW(hdc, ss.str().c_str(), -1, &inner, DT_CENTER | DT_TOP);
        SelectObject(hdc, oldf);
    }
    else {
        // back of card
        HBRUSH b = CreateSolidBrush(RGB(80, 120, 200));
        FillRect(hdc, &inner, b);
        DeleteObject(b);

        // pattern
        int step = 12;
        for (int y = inner.top + 8; y < inner.bottom; y += step) {
            MoveToEx(hdc, inner.left + 8, y, NULL);
            LineTo(hdc, inner.right - 8, y);
        }
    }
}

void CheckForMatch() {
    if (g_firstIndex >= 0 && g_secondIndex >= 0) {
        Card& a = g_cards[g_firstIndex];
        Card& b = g_cards[g_secondIndex];
        if (a.value == b.value) {
            a.matched = true;
            b.matched = true;
            g_flippedCount += 2;
            // reset
            g_firstIndex = g_secondIndex = -1;
            g_locked = false;
        }
        else {
            // start timer to flip back after delay
            g_locked = true;
            SetTimer(g_hWnd, 1, FLIP_BACK_MS, NULL);
        }
        InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

// Win32 procedures
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        // create a font for numbers
        g_hFont = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        break;

    case WM_LBUTTONDOWN: {
        if (g_locked) break; // ignore clicks while locked
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        int idx = IndexFromPoint(x, y);
        if (idx >= 0 && !g_cards[idx].flipped && !g_cards[idx].matched) {
            g_cards[idx].flipped = true;
            if (g_firstIndex == -1) g_firstIndex = idx;
            else if (g_secondIndex == -1) g_secondIndex = idx;

            InvalidateRect(hWnd, NULL, TRUE);

            if (g_firstIndex != -1 && g_secondIndex != -1) {
                CheckForMatch();
            }
        }
        break;
    }

    case WM_TIMER:
        if (wParam == 1) {
            // flip back non-matching cards
            KillTimer(hWnd, 1);
            if (g_firstIndex >= 0 && g_secondIndex >= 0) {
                g_cards[g_firstIndex].flipped = false;
                g_cards[g_secondIndex].flipped = false;
            }
            g_firstIndex = g_secondIndex = -1;
            g_locked = false;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        // background
        HBRUSH bg = CreateSolidBrush(RGB(40, 44, 52));
        FillRect(hdc, &rcClient, bg);
        DeleteObject(bg);

        // title
        RECT titleRect = { 10, 4, 500, 30 };
        HFONT oldf = (HFONT)SelectObject(hdc, g_hFont);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        DrawTextW(hdc, L"Memory - 4x4 (Win32 GDI)", -1, &titleRect, DT_LEFT | DT_TOP);
        SelectObject(hdc, oldf);

        // draw cards
        for (const Card& card : g_cards) {
            DrawCard(hdc, card);
        }

        // status
        RECT statusRect = { 10, rcClient.bottom - 28, rcClient.right - 10, rcClient.bottom };
        std::wstring status;
        if (g_flippedCount == CARD_COUNT)
            status = L"Chúc mừng! Bạn đã ghép hết bài. Nhấn N để chơi lại.";
        else
            status = L"Click để lật bài. Ghép tất cả cặp. Nhấn N để chơi lại.";

        SetTextColor(hdc, RGB(220, 220, 220));
        DrawTextW(hdc, status.c_str(), -1, &statusRect, DT_LEFT | DT_TOP);

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_KEYDOWN:
        if (wParam == 'N') {
            StartNewGame();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_DESTROY:
        if (g_hFont) DeleteObject(g_hFont);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    // register class
    const wchar_t CLASS_NAME[] = L"MemoryGameClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // initial window size set inside StartNewGame
    g_hWnd = CreateWindowEx(0, CLASS_NAME, L"Memory - 4x4", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 520, NULL, NULL, hInstance, NULL);

    if (!g_hWnd) return 0;

    StartNewGame();

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    // message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
