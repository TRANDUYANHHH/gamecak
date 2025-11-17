// memory_win95.cpp
// Compile (MSVC VC6): cl /EHsc memory_win95.cpp user32.lib gdi32.lib
// Compile (MinGW): g++ memory_win95.cpp -o memory.exe -lgdi32 -luser32
//
// Designed to run on legacy Windows 95 / VC6 environments.
// Uses ANSI Win32 APIs only (CreateFontA, DrawTextA, CreateWindowA, etc).

#include <windows.h>

#define COLS 4
#define ROWS 4
#define CARD_COUNT (COLS * ROWS)
#define CARD_W 120
#define CARD_H 160
#define GAP 12
#define BORDER 16
#define FLIP_BACK_MS 800

typedef struct Card {
    RECT rect;
    int value;
    BOOL flipped;
    BOOL matched;
} Card;

static Card g_cards[CARD_COUNT];
static HWND g_hWnd = NULL;
static HFONT g_hFont = NULL;
static int g_flippedCount = 0;
static int g_firstIndex = -1;
static int g_secondIndex = -1;
static BOOL g_locked = FALSE;

// Helper: index from client point
int IndexFromPoint(int x, int y) {
    POINT pt;
    pt.x = x;
    pt.y = y;

    for (int i = 0; i < CARD_COUNT; ++i) {
        if (PtInRect(&g_cards[i].rect, pt))
            return i;
    }
    return -1;
}

// Simple Fisher-Yates shuffle using rand() seeded by GetTickCount
void ShuffleIntArray(int* arr, int n) {
    unsigned int seed = (unsigned int)GetTickCount();
    srand((unsigned int)(seed ^ 0xA5A5A5A5));
    for (int i = n - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        int t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
    }
}

void StartNewGame() {
    g_flippedCount = 0;
    g_firstIndex = -1;
    g_secondIndex = -1;
    g_locked = FALSE;

    int values[CARD_COUNT];
    for (int v = 0; v < CARD_COUNT / 2; ++v) {
        values[v * 2] = v + 1;
        values[v * 2 + 1] = v + 1;
    }
    ShuffleIntArray(values, CARD_COUNT);

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            int i = r * COLS + c;
            int x = BORDER + c * (CARD_W + GAP);
            int y = BORDER + r * (CARD_H + GAP) + 28; // leave room for title
            g_cards[i].rect.left = x;
            g_cards[i].rect.top = y;
            g_cards[i].rect.right = x + CARD_W;
            g_cards[i].rect.bottom = y + CARD_H;
            g_cards[i].value = values[i];
            g_cards[i].flipped = FALSE;
            g_cards[i].matched = FALSE;
        }
    }

    if (g_hWnd) {
        // Resize window client area to fit board (optional)
        int width = BORDER * 2 + COLS * CARD_W + (COLS - 1) * GAP;
        int height = BORDER * 2 + ROWS * CARD_H + (ROWS - 1) * GAP + 40;
        RECT rc = { 0, 0, width, height };
        AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);
        SetWindowPos(g_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
    }
}

// Draw a single card using safe SelectObject usage
void DrawCard(HDC hdc, const Card* card) {
    // Draw frame using stock black brush (cast to HBRUSH and do not delete)
    HBRUSH hFrame = (HBRUSH)GetStockObject(BLACK_BRUSH);
    FrameRect(hdc, &card->rect, hFrame);

    RECT inner = card->rect;
    InflateRect(&inner, -6, -6);

    if (card->matched) {
        HBRUSH b = CreateSolidBrush(RGB(200, 200, 200));
        FillRect(hdc, &inner, b);
        DeleteObject(b);
        return;
    }

    if (card->flipped) {
        // light background
        HBRUSH b = CreateSolidBrush(RGB(240, 240, 255));
        FillRect(hdc, &inner, b);
        DeleteObject(b);

        // colored circle representing value
        int cx = (inner.left + inner.right) / 2;
        int cy = inner.top + 40;
        int radius = 36;
        int R = 180 + (card->value * 17) % 75;
        int G = 90 + (card->value * 31) % 120;
        int B = 120 + (card->value * 23) % 100;

        HBRUSH cb = CreateSolidBrush(RGB(R, G, B));
        HGDIOBJ oldBrush = SelectObject(hdc, cb);

        // use null pen for filled ellipse border experience
        HPEN hNullPen = (HPEN)GetStockObject(NULL_PEN);
        HGDIOBJ oldPen = SelectObject(hdc, hNullPen);

        Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);

        // restore
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        // do not DeleteObject(NULL_PEN or stock brush)
        DeleteObject(cb);

        // draw number text
        char buf[16];
        wsprintfA(buf, "%d", card->value);

        // select font and draw text centered top
        HGDIOBJ oldFont = SelectObject(hdc, g_hFont);
        SetBkMode(hdc, TRANSPARENT);
        RECT textRc = inner;
        // text top aligned
        DrawTextA(hdc, buf, -1, &textRc, DT_CENTER | DT_TOP);
        SelectObject(hdc, oldFont);
    }
    else {
        // back of card design
        HBRUSH b = CreateSolidBrush(RGB(80, 120, 200));
        FillRect(hdc, &inner, b);
        DeleteObject(b);

        // simple horizontal stripes
        int step = 12;
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(60, 90, 170));
        HGDIOBJ oldPen2 = SelectObject(hdc, hPen);
        for (int y = inner.top + 8; y < inner.bottom; y += step) {
            MoveToEx(hdc, inner.left + 8, y, NULL);
            LineTo(hdc, inner.right - 8, y);
        }
        SelectObject(hdc, oldPen2);
        DeleteObject(hPen);
    }
}

void CheckForMatch() {
    if (g_firstIndex >= 0 && g_secondIndex >= 0) {
        Card* a = &g_cards[g_firstIndex];
        Card* b = &g_cards[g_secondIndex];
        if (a->value == b->value) {
            a->matched = TRUE;
            b->matched = TRUE;
            g_flippedCount += 2;
            g_firstIndex = g_secondIndex = -1;
            g_locked = FALSE;
        }
        else {
            g_locked = TRUE;
            SetTimer(g_hWnd, 1, FLIP_BACK_MS, NULL);
        }
        InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // Use ANSI CreateFontA and a font that exists on Win95
        g_hFont = CreateFontA(
            20, 0, 0, 0, FW_BOLD,
            FALSE, FALSE, FALSE,
            ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            "MS Sans Serif"
        );

        // start a new game
        StartNewGame();
        break;
    }

    case WM_LBUTTONDOWN: {
        if (g_locked) break;
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        int idx = IndexFromPoint(x, y);
        if (idx >= 0 && !g_cards[idx].flipped && !g_cards[idx].matched) {
            g_cards[idx].flipped = TRUE;
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
            KillTimer(hWnd, 1);
            if (g_firstIndex >= 0 && g_secondIndex >= 0) {
                g_cards[g_firstIndex].flipped = FALSE;
                g_cards[g_secondIndex].flipped = FALSE;
            }
            g_firstIndex = g_secondIndex = -1;
            g_locked = FALSE;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // background
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        HBRUSH bg = CreateSolidBrush(RGB(40, 44, 52));
        FillRect(hdc, &rcClient, bg);
        DeleteObject(bg);

        // title (use DrawTextA)
        RECT titleRect = { 10, 4, 500, 30 };
        HGDIOBJ oldFont = SelectObject(hdc, g_hFont);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        DrawTextA(hdc, "Memory - 4x4 (Win95)", -1, &titleRect, DT_LEFT | DT_TOP);
        SelectObject(hdc, oldFont);

        // draw cards
        for (int i = 0; i < CARD_COUNT; ++i) {
            DrawCard(hdc, &g_cards[i]);
        }

        // status
        RECT statusRect = { 10, rcClient.bottom - 28, rcClient.right - 10, rcClient.bottom };
        char status[128];
        if (g_flippedCount == CARD_COUNT)
            wsprintfA(status, "Chuc mung! Ban da ghep het bai. N de choi lai.");
        else
            wsprintfA(status, "Click de lat bai. Gheptat ca cap. N de choi lai.");

        SetTextColor(hdc, RGB(220, 220, 220));
        DrawTextA(hdc, status, -1, &statusRect, DT_LEFT | DT_TOP);

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "MemoryGameWin95";

    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "RegisterClass failed", "Error", MB_ICONERROR);
        return 0;
    }

    g_hWnd = CreateWindowA(CLASS_NAME, "Memory - 4x4 (Win95)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 520,
        NULL, NULL, hInstance, NULL);

    if (!g_hWnd) return 0;

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}
