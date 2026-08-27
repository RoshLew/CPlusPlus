// compile: g++ HelloG.cpp -o HelloG.exe -mwindows -luser32

#include <windows.h>
#include <ctime> // For generating random numbers

// Global Variables for Color, Text Size, and Font Name
DWORD textColor;
int textSize;

// Array of possible fonts
const char* fontFamily[] = {"Arial", "Times New Roman", "Verdana", "Courier New"};
int numFonts = sizeof(fontFamily) / sizeof(fontFamily[0]);

// Function to get a random color
COLORREF getRandomColor() {
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}

// Function to get a random font size between minSize and maxSize
int getRandomFontSize(int minSize, int maxSize) {
    return rand() % (maxSize - minSize + 1) + minSize;
}

// Function to get a random font from the array
const char* getRandomFont() {
    return fontFamily[rand() % numFonts];
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Define the text to be displayed and its attributes
            char text[] = "Hello, World!";
            RECT rect;
            GetClientRect(hwnd, &rect);
            SetBkMode(hdc, TRANSPARENT);
            textColor = getRandomColor();  // Random color
            textSize = getRandomFontSize(8, 24);  // Random font size between 8 and 24

            const char* font = getRandomFont();  // Random font

            SelectObject(hdc, CreateFont(textSize, 0, 0, 0,
                                         FW_NORMAL, FALSE, FALSE, FALSE,
                                         ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                                         CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                         DEFAULT_PITCH | FF_SWISS, font));

            // Draw the text centered in the window
            SetTextColor(hdc, textColor); // Use SetTextColor to apply the random color
            DrawText(hdc, text, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

            DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));  // Clean up font

            EndPaint(hwnd, &ps);
        }
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    srand(time(NULL));  // Seed the random number generator

    const char* CLASS_NAME = "Sample Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        "Hello, World! - Graphical Mode", // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
