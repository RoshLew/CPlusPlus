// Snake.cpp - Win32 graphical snake game
// Compile (MinGW):  g++ Snake.cpp -o Snake.exe -mwindows -municode -O2 -std=c++17
// Compile (MSVC):   cl /EHsc Snake.cpp /link user32.lib gdi32.lib /SUBSYSTEM:WINDOWS
//
// Controls: arrow keys or WASD to steer, Space to pause, R to restart, Esc to quit.

#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <random>

using namespace std;

// ---------- Layout ----------
const int CELL        = 22;           // pixel size of one grid cell
const int COLS        = 24;
const int ROWS        = 24;
const int GRID_W      = CELL * COLS;  // 528
const int GRID_H      = CELL * ROWS;  // 528
const int WINDOW_W    = GRID_W + 16;  // a little padding
const int WINDOW_H    = GRID_H + 70;  // room for the score line
const int SCORE_H     = 28;           // height of the score band

// ---------- Game state ----------
struct Point { int x, y; };

enum Dir { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

vector<Point> g_snake;       // g_snake[0] is the head
Dir           g_dir   = DIR_RIGHT;
Dir           g_next  = DIR_RIGHT;  // queued direction; applied at next tick
Point         g_food  = { -1, -1 };
int           g_score = 0;
bool          g_paused   = false;
bool          g_gameOver = false;
mt19937       g_rng;

HINSTANCE g_hInst = nullptr;

// ---------- Helpers ----------
bool inSnake(int x, int y) {
    for (const auto& p : g_snake)
        if (p.x == x && p.y == y) return true;
    return false;
}

Point randomEmptyCell() {
    Point p;
    do {
        p.x = uniform_int_distribution<int>(0, COLS - 1)(g_rng);
        p.y = uniform_int_distribution<int>(0, ROWS - 1)(g_rng);
    } while (inSnake(p.x, p.y));
    return p;
}

void placeFood() { g_food = randomEmptyCell(); }

void resetGame() {
    g_snake.clear();
    g_snake.push_back({ COLS / 2,     ROWS / 2     });
    g_snake.push_back({ COLS / 2 - 1, ROWS / 2     });
    g_snake.push_back({ COLS / 2 - 2, ROWS / 2     });
    g_dir   = DIR_RIGHT;
    g_next  = DIR_RIGHT;
    g_score = 0;
    g_paused   = false;
    g_gameOver = false;
    placeFood();
}

bool step() {
    g_dir = g_next;
    Point head = g_snake.front();
    switch (g_dir) {
        case DIR_UP:    head.y--; break;
        case DIR_DOWN:  head.y++; break;
        case DIR_LEFT:  head.x--; break;
        case DIR_RIGHT: head.x++; break;
    }

    // Wall collision
    if (head.x < 0 || head.x >= COLS || head.y < 0 || head.y >= ROWS) {
        g_gameOver = true; return false;
    }
    // Self collision (skip the tail — it's about to move)
    for (size_t i = 0; i + 1 < g_snake.size(); ++i)
        if (g_snake[i].x == head.x && g_snake[i].y == head.y) {
            g_gameOver = true; return false;
        }

    g_snake.insert(g_snake.begin(), head);
    if (head.x == g_food.x && head.y == g_food.y) {
        ++g_score;
        if ((int)g_snake.size() >= COLS * ROWS) return true;   // board full — win
        placeFood();
    } else {
        g_snake.pop_back();
    }
    return true;
}

// ---------- Drawing ----------
void FillCell(HDC dc, int x, int y, COLORREF c) {
    RECT r = { x * CELL, y * CELL, (x + 1) * CELL, (y + 1) * CELL };
    HBRUSH b = CreateSolidBrush(c);
    FillRect(dc, &r, b);
    DeleteObject(b);
}

void PaintGame(HWND, HDC screen) {
    HWND hwnd = WindowFromDC(screen);
    RECT rc; GetClientRect(hwnd, &rc);

    HDC     mem    = CreateCompatibleDC(screen);
    HBITMAP bmp    = CreateCompatibleBitmap(screen, rc.right, rc.bottom);
    HBITMAP oldBmp = (HBITMAP)SelectObject(mem, bmp);

    // Background
    HBRUSH bg = CreateSolidBrush(RGB(20, 24, 28));
    FillRect(mem, &rc, bg);
    DeleteObject(bg);

    // Checkerboard grid
    for (int y = 0; y < ROWS; ++y)
        for (int x = 0; x < COLS; ++x)
            FillCell(mem, x, y, ((x ^ y) & 1) ? RGB(28, 34, 40) : RGB(24, 30, 36));

    // Food
    FillCell(mem, g_food.x, g_food.y, RGB(230, 70, 70));

    // Snake (head brighter, body gradient)
    for (size_t i = 0; i < g_snake.size(); ++i) {
        int shade = 180 - (int)i * 4;     // body slightly darker toward tail
        if (shade < 60) shade = 60;
        COLORREF c = (i == 0)
            ? RGB(120, 220, 120)
            : RGB(70, shade, 90);
        FillCell(mem, g_snake[i].x, g_snake[i].y, c);
    }

    // Score band
    SetBkMode(mem, TRANSPARENT);
    SetTextColor(mem, RGB(230, 230, 230));
    HFONT font = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT oldF = (HFONT)SelectObject(mem, font);
    wchar_t buf[64];
    if      (g_gameOver) _snwprintf_s(buf, _TRUNCATE, L" Game over — score %d. Press R to restart. ", g_score);
    else if (g_paused)   _snwprintf_s(buf, _TRUNCATE, L" Paused — press Space to resume. ");
    else                 _snwprintf_s(buf, _TRUNCATE, L" Score: %d   Length: %d ", g_score, (int)g_snake.size());
    RECT sr = {0, GRID_H, rc.right, GRID_H + SCORE_H};
    DrawTextW(mem, buf, -1, &sr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(mem, oldF);
    DeleteObject(font);

    BitBlt(screen, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
}

// ---------- Window procedure ----------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 110, NULL);       // ~9 moves/sec
        resetGame();
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        PaintGame(hwnd, dc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_TIMER:
        if (!g_paused && !g_gameOver) {
            step();
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;

    case WM_KEYDOWN: {
        Dir want = g_dir;
        switch (wp) {
            case VK_UP:    case 'W': want = DIR_UP;    break;
            case VK_DOWN:  case 'S': want = DIR_DOWN;  break;
            case VK_LEFT:  case 'A': want = DIR_LEFT;  break;
            case VK_RIGHT: case 'D': want = DIR_RIGHT; break;
            case VK_SPACE:
                if (!g_gameOver) g_paused = !g_paused;
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            case 'R': case 'r':
                resetGame();
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            case VK_ESCAPE:
                DestroyWindow(hwnd);
                return 0;
        }
        // Disallow 180° reversals — a queued reversal would kill you instantly.
        auto opposite = [](Dir d) {
            return d == DIR_UP ? DIR_DOWN :
                   d == DIR_DOWN ? DIR_UP :
                   d == DIR_LEFT ? DIR_RIGHT : DIR_LEFT;
        };
        if (want != opposite(g_dir)) g_next = want;
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

// ---------- Entry point ----------
int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, LPWSTR, int) {
    g_rng.seed(static_cast<unsigned int>(time(0)));
    g_hInst = hi;

    const wchar_t* CLASS_NAME = L"SnakeWnd";
    WNDCLASSW wc = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    CreateWindowExW(
        0, CLASS_NAME, L"Snake",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_W, WINDOW_H,
        NULL, NULL, hi, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
