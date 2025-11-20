#include <windows.h>

#define COLS 4
#define ROWS 3
#define CARD_COUNT (COLS * ROWS)
#define CARD_W 120
#define CARD_H 120
#define GAP 0
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
static int g_blinkStep = 0;
static BOOL g_blinking = FALSE;

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
            int y = BORDER + r * (CARD_H + GAP) + 28;
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
        int width = BORDER * 2 + COLS * CARD_W + (COLS - 1) * GAP;
        int height = BORDER * 2 + ROWS * CARD_H + (ROWS - 1) * GAP + 40;
        RECT rc = { 0, 0, width, height };
        AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);
        SetWindowPos(g_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void DrawCard(HDC hdc, const Card* card) {
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
        BOOL isBlinkTarget = (g_blinking &&
            (&g_cards[g_firstIndex] == card || &g_cards[g_secondIndex] == card));

        if (isBlinkTarget) {
            if (g_blinkStep % 2 == 0) {
                HBRUSH b = CreateSolidBrush(RGB(51, 255, 51));
                FillRect(hdc, &inner, b);
                DeleteObject(b);
                return;
            }
        }

        HBRUSH b = CreateSolidBrush(RGB(255, 204, 204));//RGB(240, 240, 255));
        FillRect(hdc, &inner, b);
        DeleteObject(b);

        int cx = (inner.left + inner.right) / 2;
        int cy = (inner.top + inner.bottom) / 2;//inner.top + 40;
        int radius = 36;
        int R = 180 + (card->value * 17) % 75;
        int G = 90 + (card->value * 31) % 120;
        int B = 120 + (card->value * 23) % 100;

        HBRUSH cb = CreateSolidBrush(RGB(R, G, B));
        HGDIOBJ oldBrush = SelectObject(hdc, cb);

        HPEN hNullPen = (HPEN)GetStockObject(NULL_PEN);
        HGDIOBJ oldPen = SelectObject(hdc, hNullPen);

        Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);

        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(cb);

        char buf[16];
        wsprintfA(buf, "%d", card->value);

        HGDIOBJ oldFont = SelectObject(hdc, g_hFont);
        SetBkMode(hdc, TRANSPARENT);
        RECT textRc = inner;
        DWORD dwFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
        DrawTextA(hdc, buf, -1, &textRc, dwFormat);
        SelectObject(hdc, oldFont);
    }
    else {
        HBRUSH b = CreateSolidBrush(RGB(80, 120, 200));
        FillRect(hdc, &inner, b);
        DeleteObject(b);

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
            g_blinking = TRUE;
            g_blinkStep = 0;

            SetTimer(g_hWnd, 2, 150, NULL);

            g_locked = TRUE;
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
        else if (wParam == 2) {
            g_blinkStep++;

            if (g_blinkStep >= 6) { 
                KillTimer(hWnd, 2);

                g_cards[g_firstIndex].matched = TRUE;
                g_cards[g_secondIndex].matched = TRUE;

                g_flippedCount += 2;
                g_firstIndex = g_secondIndex = -1;

                g_blinking = FALSE;
                g_locked = FALSE;

                InvalidateRect(hWnd, NULL, TRUE);
            }
            else {
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;


    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        HBRUSH bg = CreateSolidBrush(RGB(40, 44, 52));
        FillRect(hdc, &rcClient, bg);
        DeleteObject(bg);

        RECT titleRect = { 10, 4, 500, 30 };
        HGDIOBJ oldFont = SelectObject(hdc, g_hFont);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        DWORD dwFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
        DrawTextA(hdc, "Memory Card Game - 4x3", -1, &titleRect, dwFormat);
        SelectObject(hdc, oldFont);

        for (int i = 0; i < CARD_COUNT; ++i) {
            DrawCard(hdc, &g_cards[i]);
        }

        RECT statusRect = { 30 + 36, rcClient.bottom - 50, rcClient.right - 50, rcClient.bottom };
        char status[128];
        if (g_flippedCount == CARD_COUNT)
            wsprintfA(status, "CHUC MUNG !!! BAN DA GHEP HET BAI. N DE CHOI LAI.");
        else
            wsprintfA(status, "CLICK DE LAT BAI -- GHEP TAT CA CAC CAP -- N DE CHOI LAI");
        // cre: noob game for Vu Do Phuong Dong, Nguyen Xuan Duc and Tran Duy Anh <333
        char cre[100];
        wsprintfA(cre, "Cre: Dong, Duc, DAnh :)");
        SetTextColor(hdc, RGB(225, 255, 51));

        RECT creRect = { rcClient.right - 160, rcClient.bottom - 18, rcClient.right - 10, rcClient.bottom };
        DrawTextA(hdc, cre, -1, &creRect, DT_VCENTER | DT_SINGLELINE);

        SetTextColor(hdc, RGB(220, 220, 220));
        DrawTextA(hdc, status, -1, &statusRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

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

    g_hWnd = CreateWindowA(CLASS_NAME, "Memory - 4x3 (Win95)",
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
