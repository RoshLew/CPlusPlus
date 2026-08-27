// TicTacToe2.cpp - Win32 graphical version (no external dependencies)
// Compile (MinGW):   g++ TicTacToe2.cpp -o TicTacToe2.exe -mwindows -O2 -std=c++17 -municode
// Compile (MSVC):   cl /EHsc TicTacToe2.cpp /link user32.lib gdi32.lib /SUBSYSTEM:WINDOWS
//
// Controls: click a cell to play, "R" or the New Game button to reset, Esc to quit.

#include <windows.h>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <utility>

using namespace std;

// ---------- Layout ----------
const int BOARD_SIZE  = 3;
const int CELL_SIZE   = 180;
const int GRID_PIXELS = CELL_SIZE * BOARD_SIZE;     // 540
const int WINDOW_W    = 580;
const int WINDOW_H    = 760;
const int STATUS_Y    = GRID_PIXELS + 10;           // top of status band
const int BUTTON_Y    = GRID_PIXELS + 55;           // top of "New Game" button
const int SCORE_Y     = BUTTON_Y + 50;              // top of score panel

// ---------- Game state ----------
char    board[BOARD_SIZE][BOARD_SIZE];              // ' ', 'X', 'O'
const char HUMAN_MARK = 'O';
const char AI_MARK    = 'X';
bool    g_humanTurn = true;                         // human goes first
bool    g_gameOver  = false;
wstring g_status    = L"Your move (O). Click an empty cell.";

// Persistent score (survives resets; cleared on app exit).
int g_scoreHuman = 0;
int g_scoreAI    = 0;
int g_scoreDraw  = 0;

// Winning line, encoded as 4 ints: {a_r, a_c, b_r, b_c} of the two endpoints.
// {-1,-1,-1,-1} means "no winning line to draw".
int g_winLine[4] = { -1, -1, -1, -1 };

mt19937 rng;
HINSTANCE g_hInst = nullptr;
HWND      g_btn   = nullptr;

// ---------- Game logic (same AI as the console version) ----------
void initBoard() {
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            board[i][j] = ' ';
}

bool checkWin(char p) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (board[i][0] == p && board[i][1] == p && board[i][2] == p) return true;
        if (board[0][i] == p && board[1][i] == p && board[2][i] == p) return true;
    }
    return (board[0][0] == p && board[1][1] == p && board[2][2] == p)
        || (board[0][2] == p && board[1][1] == p && board[2][0] == p);
}

// Like checkWin, but also writes the winning-line endpoints into `out` so the
// paint routine can draw a highlight stroke. Returns true if `p` has a line.
bool findWinLine(char p, int out[4]) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (board[i][0] == p && board[i][1] == p && board[i][2] == p) {
            out[0]=i; out[1]=0; out[2]=i; out[3]=2; return true;
        }
        if (board[0][i] == p && board[1][i] == p && board[2][i] == p) {
            out[0]=0; out[1]=i; out[2]=2; out[3]=i; return true;
        }
    }
    if (board[0][0] == p && board[1][1] == p && board[2][2] == p) {
        out[0]=0; out[1]=0; out[2]=2; out[3]=2; return true;
    }
    if (board[0][2] == p && board[1][1] == p && board[2][0] == p) {
        out[0]=0; out[1]=2; out[2]=2; out[3]=0; return true;
    }
    return false;
}

bool allFilled() {
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            if (board[i][j] == ' ') return false;
    return true;
}

// Try every empty cell with `probe`; if any produces a win, commit `place` there.
bool findLineMove(char probe, char place) {
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            if (board[i][j] == ' ') {
                board[i][j] = probe;
                bool w = checkWin(probe);
                board[i][j] = ' ';
                if (w) { board[i][j] = place; return true; }
            }
    return false;
}

void aiMove() {
    if (findLineMove(AI_MARK,    AI_MARK))    return;   // take the win
    if (findLineMove(HUMAN_MARK, AI_MARK))    return;   // block the human
    if (board[1][1] == ' ') { board[1][1] = AI_MARK; return; }   // center
    vector<pair<int,int>> corners = {{0,0}, {0,2}, {2,0}, {2,2}};
    shuffle(corners.begin(), corners.end(), rng);
    for (auto [r, c] : corners)
        if (board[r][c] == ' ') { board[r][c] = AI_MARK; return; }
    for (int i = 0; i < BOARD_SIZE; ++i)                    // any edge
        for (int j = 0; j < BOARD_SIZE; ++j)
            if (board[i][j] == ' ') { board[i][j] = AI_MARK; return; }
}

void resetGame(HWND hwnd) {
    KillTimer(hwnd, 1);                                // cancel any pending AI move
    initBoard();
    g_humanTurn = true;
    g_gameOver  = false;
    g_status    = L"Your move (O). Click an empty cell.";
    g_winLine[0] = g_winLine[1] = g_winLine[2] = g_winLine[3] = -1;
}

// ---------- Drawing ----------
void DrawX(HDC dc, int x, int y, int size) {
    HPEN pen = CreatePen(PS_SOLID, 8, RGB(220, 60, 60));
    HPEN old = (HPEN)SelectObject(dc, pen);
    const int pad = 30;
    MoveToEx(dc, x + pad,        y + pad,        NULL);
    LineTo  (dc, x + size - pad, y + size - pad);
    MoveToEx(dc, x + size - pad, y + pad,        NULL);
    LineTo  (dc, x + pad,        y + size - pad);
    SelectObject(dc, old);
    DeleteObject(pen);
}

void DrawO(HDC dc, int x, int y, int size) {
    HPEN pen = CreatePen(PS_SOLID, 8, RGB(60, 110, 220));
    HPEN old = (HPEN)SelectObject(dc, pen);
    HBRUSH oldB = (HBRUSH)SelectObject(dc, GetStockObject(NULL_BRUSH));
    const int pad = 30;
    Ellipse(dc, x + pad, y + pad, x + size - pad, y + size - pad);
    SelectObject(dc, oldB);
    SelectObject(dc, old);
    DeleteObject(pen);
}

// Thick stroke through the center of the two endpoint cells.
void DrawWinLine(HDC dc, const int p[4]) {
    int ax = p[1] * CELL_SIZE + CELL_SIZE / 2;
    int ay = p[0] * CELL_SIZE + CELL_SIZE / 2;
    int bx = p[3] * CELL_SIZE + CELL_SIZE / 2;
    int by = p[2] * CELL_SIZE + CELL_SIZE / 2;
    HPEN pen = CreatePen(PS_SOLID, 10, RGB(50, 170, 80));
    HPEN old = (HPEN)SelectObject(dc, pen);
    MoveToEx(dc, ax, ay, NULL);
    LineTo  (dc, bx, by);
    SelectObject(dc, old);
    DeleteObject(pen);
}

void DrawScorePanel(HDC dc, int width) {
    HFONT font = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT oldF = (HFONT)SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(40, 40, 40));

    wchar_t buf[96];
    _snwprintf_s(buf, _TRUNCATE, L" You (O): %d     Computer (X): %d     Draws: %d ",
                 g_scoreHuman, g_scoreAI, g_scoreDraw);
    RECT r = {0, SCORE_Y, width, SCORE_Y + 32};
    DrawTextW(dc, buf, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(dc, oldF);
    DeleteObject(font);
}

// Double-buffered paint: draw everything to a memory DC then blit once.
void PaintGame(HWND hwnd, HDC screen) {
    RECT rc;
    GetClientRect(hwnd, &rc);

    HDC     mem    = CreateCompatibleDC(screen);
    HBITMAP bmp    = CreateCompatibleBitmap(screen, rc.right, rc.bottom);
    HBITMAP oldBmp = (HBITMAP)SelectObject(mem, bmp);

    HBRUSH bg = CreateSolidBrush(RGB(245, 245, 245));
    FillRect(mem, &rc, bg);
    DeleteObject(bg);

    // Grid lines
    HPEN gridPen = CreatePen(PS_SOLID, 4, RGB(40, 40, 40));
    HPEN oldPen  = (HPEN)SelectObject(mem, gridPen);
    for (int i = 1; i < BOARD_SIZE; ++i) {
        MoveToEx(mem, i * CELL_SIZE, 0,           NULL);
        LineTo  (mem, i * CELL_SIZE, GRID_PIXELS);
        MoveToEx(mem, 0,           i * CELL_SIZE, NULL);
        LineTo  (mem, GRID_PIXELS, i * CELL_SIZE);
    }
    SelectObject(mem, oldPen);
    DeleteObject(gridPen);

    // X's and O's
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            if      (board[i][j] == 'X') DrawX(mem, j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE);
            else if (board[i][j] == 'O') DrawO(mem, j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE);

    // Winning line (drawn on top of marks so it's clearly visible).
    if (g_gameOver && g_winLine[0] != -1) DrawWinLine(mem, g_winLine);

    // Status text
    SetBkMode(mem, TRANSPARENT);
    HFONT font = CreateFontW(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT oldF = (HFONT)SelectObject(mem, font);
    RECT sr = {0, STATUS_Y, GRID_PIXELS, BUTTON_Y};
    DrawTextW(mem, g_status.c_str(), -1, &sr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(mem, oldF);
    DeleteObject(font);

    // Score panel
    DrawScorePanel(mem, rc.right);

    BitBlt(screen, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
}

// ---------- Window procedure ----------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        g_btn = CreateWindowExW(
            0, L"BUTTON", L"New Game (R)",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            GRID_PIXELS / 2 - 150, BUTTON_Y, 140, 30,
            hwnd, (HMENU)1, g_hInst, NULL);
        CreateWindowExW(
            0, L"BUTTON", L"Reset Score",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            GRID_PIXELS / 2 + 10, BUTTON_Y, 140, 30,
            hwnd, (HMENU)2, g_hInst, NULL);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        PaintGame(hwnd, dc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        if (g_gameOver || !g_humanTurn) return 0;
        int mx = LOWORD(lp);
        int my = HIWORD(lp);
        if (mx >= GRID_PIXELS || my >= GRID_PIXELS) return 0;
        int col = mx / CELL_SIZE;
        int row = my / CELL_SIZE;
        if (board[row][col] != ' ') return 0;

        board[row][col] = HUMAN_MARK;
        if (checkWin(HUMAN_MARK)) {
            g_status   = L"You win! Press R or click New Game.";
            g_gameOver = true;
            findWinLine(HUMAN_MARK, g_winLine);
            g_scoreHuman++;
        } else if (allFilled()) {
            g_status   = L"Draw! Press R or click New Game.";
            g_gameOver = true;
            g_scoreDraw++;
        } else {
            g_humanTurn = false;
            g_status    = L"Computer is thinking...";
        }
        InvalidateRect(hwnd, NULL, TRUE);
        if (!g_gameOver) SetTimer(hwnd, 1, 350, NULL);   // brief delay so the move shows
        return 0;
    }

    case WM_TIMER:
        KillTimer(hwnd, 1);
        aiMove();
        if (checkWin(AI_MARK)) {
            g_status   = L"Computer wins. Press R or click New Game.";
            g_gameOver = true;
            findWinLine(AI_MARK, g_winLine);
            g_scoreAI++;
        } else if (allFilled()) {
            g_status   = L"Draw! Press R or click New Game.";
            g_gameOver = true;
            g_scoreDraw++;
        } else {
            g_humanTurn = true;
            g_status    = L"Your move (O). Click an empty cell.";
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_COMMAND:
        if (HIWORD(wp) == BN_CLICKED) {
            int id = LOWORD(wp);
            if (id == 1) {                       // New Game
                resetGame(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (id == 2) {                // Reset Score
                g_scoreHuman = g_scoreAI = g_scoreDraw = 0;
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        return 0;

    case WM_KEYDOWN:
        if (wp == 'R' || wp == 'r') {
            resetGame(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
        } else if (wp == VK_ESCAPE) {
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

// ---------- Entry point ----------
int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, LPWSTR, int) {
    rng.seed(static_cast<unsigned int>(time(0)));
    g_hInst = hi;
    initBoard();

    const wchar_t* CLASS_NAME = L"TicTacToeWnd";
    WNDCLASSW wc = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Tic Tac Toe",
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
