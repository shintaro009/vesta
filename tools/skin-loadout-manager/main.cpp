#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

namespace {
constexpr int IDC_CATEGORY = 1001, IDC_ITEM = 1002, IDC_WEAR = 1003;
constexpr int IDC_STATTRAK = 1004, IDC_ADD = 1005, IDC_REMOVE = 1006;
constexpr int IDC_SAVE = 1007, IDC_LOAD = 1008, IDC_CLEAR = 1009;
constexpr int IDC_LIST = 1010, IDC_STATUS = 1011;\nconstexpr int IDC_PRESET = 1012, IDC_APPLY_PRESET = 1013;

struct LoadoutItem {
    std::wstring category;
    std::wstring name;
    double wear = 0.0;
    bool stattrak = false;
};
std::vector<LoadoutItem> g_items;
HWND g_category{}, g_item{}, g_wear{}, g_stattrak{}, g_list{}, g_status{}, g_preset{};

std::wstring windowText(HWND h) {
    int n = GetWindowTextLengthW(h);
    std::wstring s(static_cast<size_t>(n) + 1, L'\0');
    GetWindowTextW(h, s.data(), n + 1);
    s.resize(static_cast<size_t>(n));
    return s;
}
std::wstring cleanField(std::wstring s) {
    for (auto& c : s) if (c == L'|' || c == L'\r' || c == L'\n') c = L' ';
    return s;
}
std::string toUtf8(const std::wstring& s) {
    if (s.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}
std::wstring fromUtf8(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring out(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n);
    return out;
}
void status(const std::wstring& s) { SetWindowTextW(g_status, s.c_str()); }
struct SkinPreset { const wchar_t* player; std::vector<LoadoutItem> items; };
const std::vector<SkinPreset>& presets() {
    static const std::vector<SkinPreset> data = {
        {L"s1mple-inspired", {
            {L"Rifle", L"AK-47 | Case Hardened", 0.08, false},
            {L"Rifle", L"M4A1-S | Printstream", 0.12, false},
            {L"Sniper", L"AWP | Dragon Lore", 0.18, false},
            {L"Pistol", L"USP-S | Kill Confirmed", 0.16, false},
            {L"Pistol", L"Desert Eagle | Blaze", 0.03, false},
            {L"Knife", L"Karambit | Doppler", 0.02, false},
            {L"Gloves", L"Sport Gloves | Pandora's Box", 0.22, false}
        }},
        {L"ZywOo-inspired", {
            {L"Rifle", L"AK-47 | Bloodsport", 0.11, false},
            {L"Rifle", L"M4A1-S | Hot Rod", 0.04, false},
            {L"Sniper", L"AWP | Asiimov", 0.19, false},
            {L"Pistol", L"Glock-18 | Fade", 0.02, false},
            {L"Pistol", L"Desert Eagle | Code Red", 0.14, false},
            {L"Knife", L"Butterfly Knife | Slaughter", 0.08, false},
            {L"Gloves", L"Sport Gloves | Vice", 0.20, false}
        }},
        {L"donk-inspired", {
            {L"Rifle", L"AK-47 | Neon Rider", 0.09, false},
            {L"Rifle", L"M4A1-S | Decimator", 0.13, false},
            {L"Sniper", L"AWP | Neo-Noir", 0.10, false},
            {L"Pistol", L"Glock-18 | Vogue", 0.08, false},
            {L"Pistol", L"Desert Eagle | Printstream", 0.12, false},
            {L"Knife", L"Butterfly Knife | Doppler", 0.03, false},
            {L"Gloves", L"Specialist Gloves | Fade", 0.18, false}
        }},
        {L"m0NESY-inspired", {
            {L"Rifle", L"AK-47 | Inheritance", 0.10, false},
            {L"Rifle", L"M4A1-S | Golden Coil", 0.15, false},
            {L"Sniper", L"AWP | Gungnir", 0.07, false},
            {L"Pistol", L"Glock-18 | Gamma Doppler", 0.04, false},
            {L"Pistol", L"USP-S | Orion", 0.09, false},
            {L"Knife", L"Karambit | Marble Fade", 0.03, false},
            {L"Gloves", L"Driver Gloves | King Snake", 0.16, false}
        }},
        {L"NiKo-inspired", {
            {L"Rifle", L"AK-47 | Fire Serpent", 0.18, false},
            {L"Rifle", L"M4A1-S | Icarus Fell", 0.05, false},
            {L"Sniper", L"AWP | Medusa", 0.20, false},
            {L"Pistol", L"Desert Eagle | Blaze", 0.02, false},
            {L"Pistol", L"USP-S | Kill Confirmed", 0.14, false},
            {L"Knife", L"Butterfly Knife | Fade", 0.01, false},
            {L"Gloves", L"Sport Gloves | Hedge Maze", 0.24, false}
        }},
        {L"ropz-inspired", {
            {L"Rifle", L"AK-47 | Vulcan", 0.10, false},
            {L"Rifle", L"M4A1-S | Blue Phosphor", 0.03, false},
            {L"Sniper", L"AWP | Lightning Strike", 0.02, false},
            {L"Pistol", L"USP-S | Neo-Noir", 0.11, false},
            {L"Pistol", L"Desert Eagle | Cobalt Disruption", 0.05, false},
            {L"Knife", L"M9 Bayonet | Doppler", 0.03, false},
            {L"Gloves", L"Specialist Gloves | Crimson Kimono", 0.21, false}
        }}
    };
    return data;
}
void applyPreset() {
    int index = (int)SendMessageW(g_preset, CB_GETCURSEL, 0, 0);
    if (index < 0 || index >= (int)presets().size()) return;
    g_items = presets()[index].items;
    refreshList();
    status(std::wstring(presets()[index].player) + L" preset loaded (inspired set, not a verified player inventory).");
}
void refreshList() {
    SendMessageW(g_list, LB_RESETCONTENT, 0, 0);
    for (const auto& x : g_items) {
        std::wostringstream line;
        line << L"[" << x.category << L"] " << x.name << L"   | Wear: "
             << std::fixed << std::setprecision(4) << x.wear
             << (x.stattrak ? L"   | StatTrak" : L"");
        SendMessageW(g_list, LB_ADDSTRING, 0, (LPARAM)line.str().c_str());
    }
    std::wstring s = L"Items in loadout: " + std::to_wstring(g_items.size());
    status(s);
}
void addControl(HWND parent, const wchar_t* cls, const wchar_t* text, DWORD style,
                int x, int y, int w, int h, int id) {
    CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, x, y, w, h,
                    parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
}
void saveProfile(HWND owner) {
    wchar_t path[MAX_PATH] = L"vesta-loadout.vsl";
    OPENFILENAMEW ofn{}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = owner;
    ofn.lpstrFile = path; ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Vesta Loadout (*.vsl)\0*.vsl\0Text file (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1; ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L"vsl";
    if (!GetSaveFileNameW(&ofn)) return;
    std::ofstream f(path, std::ios::binary);
    if (!f) { status(L"Could not write the selected file."); return; }
    f << "VESTA_LOADOUT_V1\n";
    for (const auto& x : g_items) {
        std::wstring row = cleanField(x.category) + L"|" + cleanField(x.name) + L"|" +
                           std::to_wstring(x.wear) + L"|" + (x.stattrak ? L"1" : L"0") + L"\n";
        f << toUtf8(row);
    }
    status(L"Loadout profile saved.");
}
void loadProfile(HWND owner) {
    wchar_t path[MAX_PATH] = L"";
    OPENFILENAMEW ofn{}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = owner;
    ofn.lpstrFile = path; ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Vesta Loadout (*.vsl)\0*.vsl\0Text file (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1; ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn)) return;
    std::ifstream f(path, std::ios::binary);
    if (!f) { status(L"Could not open the selected file."); return; }
    std::string bytes((std::istreambuf_iterator<char>(f)), {});
    std::wstring content = fromUtf8(bytes);
    std::wistringstream input(content);
    std::wstring line;
    if (!std::getline(input, line) || line != L"VESTA_LOADOUT_V1") {
        status(L"Unsupported profile format."); return;
    }
    std::vector<LoadoutItem> loaded;
    while (std::getline(input, line)) {
        std::wistringstream row(line);
        std::wstring category, name, wear, st;
        if (!std::getline(row, category, L'|') || !std::getline(row, name, L'|') ||
            !std::getline(row, wear, L'|') || !std::getline(row, st, L'|')) continue;
        try {
            double value = std::stod(wear);
            if (value < 0.0 || value > 1.0) continue;
            loaded.push_back({category, name, value, st == L"1"});
        } catch (...) {}
    }
    g_items = std::move(loaded);
    refreshList();
    status(L"Loadout profile loaded.");
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        addControl(hwnd, L"STATIC", L"Category", 0, 18, 18, 90, 22, 0);
        addControl(hwnd, L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL, 18, 42, 180, 240, IDC_CATEGORY);
        g_category = GetDlgItem(hwnd, IDC_CATEGORY);
        for (const wchar_t* s : {L"Rifle", L"Pistol", L"SMG", L"Shotgun", L"Sniper", L"Machine gun", L"Knife", L"Gloves", L"Other"})
            SendMessageW(g_category, CB_ADDSTRING, 0, (LPARAM)s);
        SendMessageW(g_category, CB_SETCURSEL, 0, 0);
        addControl(hwnd, L"STATIC", L"Skin / item name", 0, 216, 18, 180, 22, 0);
        addControl(hwnd, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 216, 42, 300, 26, IDC_ITEM);
        g_item = GetDlgItem(hwnd, IDC_ITEM);
        addControl(hwnd, L"STATIC", L"Wear / Float (0.00 - 1.00)", 0, 18, 84, 220, 22, 0);
        addControl(hwnd, L"EDIT", L"0.0000", WS_BORDER | ES_AUTOHSCROLL, 18, 108, 130, 26, IDC_WEAR);
        g_wear = GetDlgItem(hwnd, IDC_WEAR);
        addControl(hwnd, L"BUTTON", L"StatTrak", BS_AUTOCHECKBOX, 170, 108, 120, 26, IDC_STATTRAK);
        g_stattrak = GetDlgItem(hwnd, IDC_STATTRAK);
        addControl(hwnd, L"BUTTON", L"Add item", BS_PUSHBUTTON, 320, 106, 92, 30, IDC_ADD);
        addControl(hwnd, L"STATIC", L"Pro player-inspired preset", 0, 18, 150, 210, 22, 0);
        addControl(hwnd, L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL, 18, 174, 250, 180, IDC_PRESET);
        g_preset = GetDlgItem(hwnd, IDC_PRESET);
        for (const auto& p : presets()) SendMessageW(g_preset, CB_ADDSTRING, 0, (LPARAM)p.player);
        SendMessageW(g_preset, CB_SETCURSEL, 0, 0);
        addControl(hwnd, L"BUTTON", L"Apply preset", BS_PUSHBUTTON, 286, 172, 112, 30, IDC_APPLY_PRESET);
        addControl(hwnd, L"STATIC", L"Current loadout", 0, 18, 214, 160, 22, 0);
        addControl(hwnd, L"LISTBOX", L"", WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, 18, 240, 498, 180, IDC_LIST);
        g_list = GetDlgItem(hwnd, IDC_LIST);
        addControl(hwnd, L"BUTTON", L"Remove selected", BS_PUSHBUTTON, 18, 432, 130, 30, IDC_REMOVE);
        addControl(hwnd, L"BUTTON", L"Save profile", BS_PUSHBUTTON, 166, 432, 110, 30, IDC_SAVE);
        addControl(hwnd, L"BUTTON", L"Load profile", BS_PUSHBUTTON, 294, 432, 110, 30, IDC_LOAD);
        addControl(hwnd, L"BUTTON", L"Clear all", BS_PUSHBUTTON, 422, 432, 94, 30, IDC_CLEAR);
        g_status = CreateWindowExW(0, L"STATIC", L"Items in loadout: 0", WS_CHILD | WS_VISIBLE,
                                   18, 474, 498, 24, hwnd, (HMENU)IDC_STATUS, GetModuleHandleW(nullptr), nullptr);
        for (HWND child = GetWindow(hwnd, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT))
            SendMessageW(child, WM_SETFONT, (WPARAM)font, TRUE);
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_ADD: {
            std::wstring name = windowText(g_item);
            if (name.empty()) { status(L"Enter a skin or item name first."); return 0; }
            wchar_t* end = nullptr;
            std::wstring wearText = windowText(g_wear);
            double wear = wcstod(wearText.c_str(), &end);
            if (end == wearText.c_str() || *end != L'\0' || wear < 0.0 || wear > 1.0) {
                status(L"Wear must be a number between 0.00 and 1.00."); return 0;
            }
            int idx = (int)SendMessageW(g_category, CB_GETCURSEL, 0, 0);
            wchar_t category[64]{}; SendMessageW(g_category, CB_GETLBTEXT, idx, (LPARAM)category);
            g_items.push_back({category, name, wear, SendMessageW(g_stattrak, BM_GETCHECK, 0, 0) == BST_CHECKED});
            SetWindowTextW(g_item, L""); SendMessageW(g_stattrak, BM_SETCHECK, BST_UNCHECKED, 0);
            refreshList(); return 0;
        }
        case IDC_REMOVE: {
            int i = (int)SendMessageW(g_list, LB_GETCURSEL, 0, 0);
            if (i != LB_ERR && i < (int)g_items.size()) g_items.erase(g_items.begin() + i);
            refreshList(); return 0;
        }
        case IDC_APPLY_PRESET: applyPreset(); return 0;\n        case IDC_SAVE: saveProfile(hwnd); return 0;
        case IDC_LOAD: loadProfile(hwnd); return 0;
        case IDC_CLEAR: g_items.clear(); refreshList(); return 0;
        }
        break;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);
    const wchar_t CLASS_NAME[] = L"VestaSkinLoadoutManager";
    WNDCLASSW wc{}; wc.lpfnWndProc = WndProc; wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME; wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    if (!RegisterClassW(&wc)) return 1;
    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Vesta Skin Loadout Manager",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 552, 560, nullptr, nullptr, instance, nullptr);
    if (!hwnd) return 1;
    ShowWindow(hwnd, show); UpdateWindow(hwnd);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg); DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}