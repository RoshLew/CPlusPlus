#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>

namespace
{
    constexpr int ID_EDIT = 101;
    constexpr int ID_FILE_NEW = 40001;
    constexpr int ID_FILE_OPEN = 40002;
    constexpr int ID_FILE_SAVE = 40003;
    constexpr int ID_FILE_SAVE_AS = 40004;
    constexpr int ID_FILE_EXIT = 40005;
    constexpr int ID_EDIT_UNDO = 40101;
    constexpr int ID_EDIT_CUT = 40102;
    constexpr int ID_EDIT_COPY = 40103;
    constexpr int ID_EDIT_PASTE = 40104;
    constexpr int ID_EDIT_DELETE = 40105;
    constexpr int ID_EDIT_SELECT_ALL = 40106;
    constexpr int ID_FORMAT_FONT = 40201;

    HWND g_edit = nullptr;
    HFONT g_editorFont = nullptr;
    std::wstring g_currentFile;
    bool g_isDirty = false;
    bool g_ignoreEditChanges = false;

    void ApplyEditorFont(const LOGFONTW& logFont)
    {
        HFONT newFont = CreateFontIndirectW(&logFont);
        if (newFont == nullptr)
        {
            return;
        }

        HFONT oldFont = g_editorFont;
        g_editorFont = newFont;
        SendMessageW(g_edit, WM_SETFONT, reinterpret_cast<WPARAM>(g_editorFont), TRUE);

        if (oldFont != nullptr)
        {
            DeleteObject(oldFont);
        }
    }

    void InitializeEditorFont()
    {
        NONCLIENTMETRICSW metrics = {};
        metrics.cbSize = sizeof(metrics);
        if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0))
        {
            ApplyEditorFont(metrics.lfMessageFont);
        }
    }

    bool DecodeUtf8(const std::vector<char>& bytes, std::wstring& text)
    {
        if (bytes.empty())
        {
            text.clear();
            return true;
        }

        const int wideLength = MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            bytes.data(),
            static_cast<int>(bytes.size()),
            nullptr,
            0);
        if (wideLength <= 0)
        {
            return false;
        }

        text.resize(static_cast<std::size_t>(wideLength));
        return MultiByteToWideChar(
                   CP_UTF8,
                   MB_ERR_INVALID_CHARS,
                   bytes.data(),
                   static_cast<int>(bytes.size()),
                   text.data(),
                   wideLength) > 0;
    }

    bool DecodeUtf16Le(const std::vector<char>& bytes, std::wstring& text)
    {
        if (bytes.size() % 2 != 0)
        {
            return false;
        }

        text.resize(bytes.size() / 2);
        for (std::size_t i = 0; i < text.size(); ++i)
        {
            const auto low = static_cast<unsigned char>(bytes[i * 2]);
            const auto high = static_cast<unsigned char>(bytes[i * 2 + 1]);
            text[i] = static_cast<wchar_t>((high << 8) | low);
        }
        return true;
    }

    bool ReadFileText(const std::wstring& path, std::wstring& text)
    {
        HANDLE file = CreateFileW(
            path.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (file == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        LARGE_INTEGER size = {};
        if (!GetFileSizeEx(file, &size) || size.QuadPart > 0x7fffffff)
        {
            CloseHandle(file);
            return false;
        }

        std::vector<char> bytes(static_cast<std::size_t>(size.QuadPart));
        DWORD bytesRead = 0;
        const BOOL ok = bytes.empty() ? TRUE : ReadFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &bytesRead, nullptr);
        CloseHandle(file);
        if (!ok || bytesRead != bytes.size())
        {
            return false;
        }

        if (bytes.size() >= 2 &&
            static_cast<unsigned char>(bytes[0]) == 0xFF &&
            static_cast<unsigned char>(bytes[1]) == 0xFE)
        {
            return DecodeUtf16Le(std::vector<char>(bytes.begin() + 2, bytes.end()), text);
        }

        if (bytes.size() >= 3 &&
            static_cast<unsigned char>(bytes[0]) == 0xEF &&
            static_cast<unsigned char>(bytes[1]) == 0xBB &&
            static_cast<unsigned char>(bytes[2]) == 0xBF)
        {
            return DecodeUtf8(std::vector<char>(bytes.begin() + 3, bytes.end()), text);
        }

        return DecodeUtf8(bytes, text);
    }

    bool WriteFileText(const std::wstring& path, const std::wstring& text)
    {
        HANDLE file = CreateFileW(
            path.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (file == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        const char bom[] = {
            static_cast<char>(0xEF),
            static_cast<char>(0xBB),
            static_cast<char>(0xBF)
        };

        DWORD bytesWritten = 0;
        if (!WriteFile(file, bom, sizeof(bom), &bytesWritten, nullptr) || bytesWritten != sizeof(bom))
        {
            CloseHandle(file);
            return false;
        }

        const int byteLength = WideCharToMultiByte(
            CP_UTF8,
            0,
            text.c_str(),
            static_cast<int>(text.size()),
            nullptr,
            0,
            nullptr,
            nullptr);
        if (byteLength < 0)
        {
            CloseHandle(file);
            return false;
        }

        std::vector<char> bytes(static_cast<std::size_t>(byteLength));
        if (byteLength > 0)
        {
            WideCharToMultiByte(
                CP_UTF8,
                0,
                text.c_str(),
                static_cast<int>(text.size()),
                bytes.data(),
                byteLength,
                nullptr,
                nullptr);
        }

        DWORD textBytesWritten = 0;
        const BOOL ok = bytes.empty() ? TRUE : WriteFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &textBytesWritten, nullptr);
        CloseHandle(file);
        return ok && textBytesWritten == bytes.size();
    }

    std::wstring GetEditorText()
    {
        const int length = GetWindowTextLengthW(g_edit);
        std::vector<wchar_t> buffer(static_cast<std::size_t>(length) + 1, L'\0');
        GetWindowTextW(g_edit, buffer.data(), length + 1);
        return std::wstring(buffer.data());
    }

    void SetWindowCaption(HWND hwnd)
    {
        std::wstring title = L"Mini Notepad";
        if (g_isDirty)
        {
            title += L" *";
        }

        title += L" - ";
        title += g_currentFile.empty() ? L"Untitled" : g_currentFile;
        SetWindowTextW(hwnd, title.c_str());
    }

    bool DoSave(HWND hwnd, bool saveAs = false);

    bool PromptToSaveChanges(HWND hwnd)
    {
        if (!g_isDirty)
        {
            return true;
        }

        const int result = MessageBoxW(
            hwnd,
            L"You have unsaved changes. Do you want to save them?",
            L"Mini Notepad",
            MB_ICONWARNING | MB_YESNOCANCEL);

        if (result == IDCANCEL)
        {
            return false;
        }

        if (result == IDYES)
        {
            return DoSave(hwnd);
        }

        return true;
    }

    void ReplaceEditorText(HWND hwnd, const std::wstring& text)
    {
        g_ignoreEditChanges = true;
        SetWindowTextW(g_edit, text.c_str());
        SendMessageW(g_edit, EM_SETMODIFY, FALSE, 0);
        g_ignoreEditChanges = false;
        g_isDirty = false;
        SetWindowCaption(hwnd);
    }

    void DoNew(HWND hwnd)
    {
        g_currentFile.clear();
        ReplaceEditorText(hwnd, L"");
    }

    bool DoOpen(HWND hwnd)
    {
        wchar_t fileName[MAX_PATH] = {};
        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (!GetOpenFileNameW(&ofn))
        {
            return false;
        }

        std::wstring content;
        if (!ReadFileText(fileName, content))
        {
            MessageBoxW(hwnd, L"Could not open the file. UTF-8 and UTF-16 LE text files are supported.", L"Open Error", MB_ICONERROR);
            return false;
        }

        g_currentFile = fileName;
        ReplaceEditorText(hwnd, content);
        return true;
    }

    bool DoSave(HWND hwnd, bool saveAs)
    {
        wchar_t fileName[MAX_PATH] = {};
        if (!g_currentFile.empty())
        {
            lstrcpynW(fileName, g_currentFile.c_str(), MAX_PATH);
        }

        if (saveAs || g_currentFile.empty())
        {
            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = fileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
            ofn.lpstrDefExt = L"txt";
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

            if (!GetSaveFileNameW(&ofn))
            {
                return false;
            }
        }

        if (!WriteFileText(fileName, GetEditorText()))
        {
            MessageBoxW(hwnd, L"Could not save the file.", L"Save Error", MB_ICONERROR);
            return false;
        }

        g_currentFile = fileName;
        g_isDirty = false;
        SendMessageW(g_edit, EM_SETMODIFY, FALSE, 0);
        SetWindowCaption(hwnd);
        return true;
    }

    void DoEditCommand(UINT commandId)
    {
        switch (commandId)
        {
        case ID_EDIT_UNDO:
            SendMessageW(g_edit, WM_UNDO, 0, 0);
            break;
        case ID_EDIT_CUT:
            SendMessageW(g_edit, WM_CUT, 0, 0);
            break;
        case ID_EDIT_COPY:
            SendMessageW(g_edit, WM_COPY, 0, 0);
            break;
        case ID_EDIT_PASTE:
            SendMessageW(g_edit, WM_PASTE, 0, 0);
            break;
        case ID_EDIT_DELETE:
            SendMessageW(g_edit, WM_CLEAR, 0, 0);
            break;
        case ID_EDIT_SELECT_ALL:
            SendMessageW(g_edit, EM_SETSEL, 0, -1);
            break;
        }
    }

    void DoChooseFont(HWND hwnd)
    {
        LOGFONTW logFont = {};
        if (g_editorFont != nullptr)
        {
            GetObjectW(g_editorFont, sizeof(logFont), &logFont);
        }
        else
        {
            NONCLIENTMETRICSW metrics = {};
            metrics.cbSize = sizeof(metrics);
            if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0))
            {
                logFont = metrics.lfMessageFont;
            }
        }

        CHOOSEFONTW chooseFont = {};
        chooseFont.lStructSize = sizeof(chooseFont);
        chooseFont.hwndOwner = hwnd;
        chooseFont.lpLogFont = &logFont;
        chooseFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;

        if (ChooseFontW(&chooseFont))
        {
            ApplyEditorFont(logFont);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        g_edit = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            0,
            0,
            0,
            0,
            hwnd,
            reinterpret_cast<HMENU>(ID_EDIT),
            reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance,
            nullptr);

        if (g_edit == nullptr)
        {
            return -1;
        }

        InitializeEditorFont();
        SetFocus(g_edit);
        SetWindowCaption(hwnd);
        return 0;
    }
    case WM_SETFOCUS:
        if (g_edit != nullptr)
        {
            SetFocus(g_edit);
        }
        return 0;
    case WM_SIZE:
        if (g_edit != nullptr)
        {
            MoveWindow(g_edit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        }
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_FILE_NEW:
            if (PromptToSaveChanges(hwnd))
            {
                DoNew(hwnd);
            }
            return 0;
        case ID_FILE_OPEN:
            if (PromptToSaveChanges(hwnd))
            {
                DoOpen(hwnd);
            }
            return 0;
        case ID_FILE_SAVE:
            DoSave(hwnd);
            return 0;
        case ID_FILE_SAVE_AS:
            DoSave(hwnd, true);
            return 0;
        case ID_FILE_EXIT:
            SendMessageW(hwnd, WM_CLOSE, 0, 0);
            return 0;
        case ID_EDIT_UNDO:
        case ID_EDIT_CUT:
        case ID_EDIT_COPY:
        case ID_EDIT_PASTE:
        case ID_EDIT_DELETE:
        case ID_EDIT_SELECT_ALL:
            DoEditCommand(LOWORD(wParam));
            return 0;
        case ID_FORMAT_FONT:
            DoChooseFont(hwnd);
            return 0;
        case ID_EDIT:
            if (HIWORD(wParam) == EN_CHANGE && !g_ignoreEditChanges)
            {
                g_isDirty = true;
                SetWindowCaption(hwnd);
            }
            return 0;
        }
        break;
    case WM_INITMENUPOPUP:
        if (g_edit != nullptr)
        {
            EnableMenuItem(reinterpret_cast<HMENU>(wParam), ID_EDIT_UNDO, MF_BYCOMMAND | (SendMessageW(g_edit, EM_CANUNDO, 0, 0) ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(reinterpret_cast<HMENU>(wParam), ID_EDIT_CUT, MF_BYCOMMAND | (SendMessageW(g_edit, EM_GETSEL, 0, 0) ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(reinterpret_cast<HMENU>(wParam), ID_EDIT_COPY, MF_BYCOMMAND | (SendMessageW(g_edit, EM_GETSEL, 0, 0) ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(reinterpret_cast<HMENU>(wParam), ID_EDIT_DELETE, MF_BYCOMMAND | (SendMessageW(g_edit, EM_GETSEL, 0, 0) ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(reinterpret_cast<HMENU>(wParam), ID_EDIT_PASTE, MF_BYCOMMAND | (IsClipboardFormatAvailable(CF_UNICODETEXT) ? MF_ENABLED : MF_GRAYED));
        }
        return 0;
    case WM_CLOSE:
        if (PromptToSaveChanges(hwnd))
        {
            DestroyWindow(hwnd);
        }
        return 0;
    case WM_DESTROY:
        if (g_editorFont != nullptr)
        {
            DeleteObject(g_editorFont);
            g_editorFont = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

HMENU CreateMainMenu()
{
    HMENU menuBar = CreateMenu();
    HMENU fileMenu = CreatePopupMenu();
    HMENU editMenu = CreatePopupMenu();
    HMENU formatMenu = CreatePopupMenu();

    AppendMenuW(fileMenu, MF_STRING, ID_FILE_NEW, L"&New\tCtrl+N");
    AppendMenuW(fileMenu, MF_STRING, ID_FILE_OPEN, L"&Open...\tCtrl+O");
    AppendMenuW(fileMenu, MF_STRING, ID_FILE_SAVE, L"&Save\tCtrl+S");
    AppendMenuW(fileMenu, MF_STRING, ID_FILE_SAVE_AS, L"Save &As...");
    AppendMenuW(fileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(fileMenu, MF_STRING, ID_FILE_EXIT, L"E&xit");

    AppendMenuW(editMenu, MF_STRING, ID_EDIT_UNDO, L"&Undo\tCtrl+Z");
    AppendMenuW(editMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_CUT, L"Cu&t\tCtrl+X");
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_COPY, L"&Copy\tCtrl+C");
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_PASTE, L"&Paste\tCtrl+V");
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_DELETE, L"&Delete\tDel");
    AppendMenuW(editMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_SELECT_ALL, L"Select &All\tCtrl+A");

    AppendMenuW(formatMenu, MF_STRING, ID_FORMAT_FONT, L"&Font...");

    AppendMenuW(menuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(fileMenu), L"&File");
    AppendMenuW(menuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(editMenu), L"&Edit");
    AppendMenuW(menuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(formatMenu), L"F&ormat");
    return menuBar;
}

HACCEL CreateAccelerators()
{
    ACCEL accelerators[] = {
        { FCONTROL | FVIRTKEY, 'N', ID_FILE_NEW },
        { FCONTROL | FVIRTKEY, 'O', ID_FILE_OPEN },
        { FCONTROL | FVIRTKEY, 'S', ID_FILE_SAVE },
        { FCONTROL | FVIRTKEY, 'Z', ID_EDIT_UNDO },
        { FCONTROL | FVIRTKEY, 'X', ID_EDIT_CUT },
        { FCONTROL | FVIRTKEY, 'C', ID_EDIT_COPY },
        { FCONTROL | FVIRTKEY, 'V', ID_EDIT_PASTE },
        { FCONTROL | FVIRTKEY, 'A', ID_EDIT_SELECT_ALL },
        { FVIRTKEY, VK_DELETE, ID_EDIT_DELETE }
    };
    return CreateAcceleratorTableW(accelerators, static_cast<int>(std::size(accelerators)));
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"MiniNotepadWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassW(&wc))
    {
        return 0;
    }

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Mini Notepad",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        nullptr,
        CreateMainMenu(),
        hInstance,
        nullptr);

    if (hwnd == nullptr)
    {
        return 0;
    }

    HACCEL accelerators = CreateAccelerators();

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        if (accelerators == nullptr || !TranslateAcceleratorW(hwnd, accelerators, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    if (accelerators != nullptr)
    {
        DestroyAcceleratorTable(accelerators);
    }

    return static_cast<int>(msg.wParam);
}
