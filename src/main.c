#include <assert.h>
#include <windows.h>
#include "tray.h"

#ifdef DEBUG
  #include <stdio.h>
  #define DebugLog(...) (printf(__VA_ARGS__), puts(""));
#else
  #define DebugLog(...);
#endif

#define POPUP_MENU_TOGGLE 1
#define POPUP_MENU_EXIT 2

void DebugLog2(const char *msg) {
#ifdef DEBUG
  HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (stdOut != NULL && stdOut != INVALID_HANDLE_VALUE) {
      DWORD written = 0;
      WriteConsoleA(stdOut, msg, strlen(msg), &written, NULL);
  }
#else
  assert(msg != NULL);
#endif
}

LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ToggleMainWindowVisibility(void);
void ShowPopupMenu(HWND window_handle);

const wchar_t WINDOW_CLASS[]  = L"C Tray Application";
const wchar_t WINDOW_TITLE[] = L"C Tray Application";

int screen_width, screen_height;
int window_width, window_height;
int search_query_font_size, search_result_font_size;

HWND main_window_handle;
HWND search_handle;
BOOL main_window_is_visible = FALSE;

// Display the window with ctrl + alt + f
BOOL alt_key_down = FALSE;
BOOL ctrl_key_down = FALSE;
BOOL f_key_down = FALSE;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

  // Assert unused parameters to quiet compiler warnings
  assert(hPrevInstance == NULL);
  assert(pCmdLine != NULL);
  assert(nCmdShow > 0);

  screen_width = GetSystemMetrics(SM_CXSCREEN);
  screen_height = GetSystemMetrics(SM_CYSCREEN);
  window_width = screen_width / 3;
  window_height = screen_height / 3;
  search_query_font_size = window_height / 8;

  // Exit if an instance of this program already exists
  if (FindWindow(WINDOW_CLASS, WINDOW_TITLE) != NULL) {
    return 0;
  }

  WNDCLASS window_class = {};
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = hInstance;
  window_class.lpszClassName = WINDOW_CLASS;
  window_class.hCursor = LoadCursor(hInstance, IDC_ARROW);
  // window_class.hbrBackground = COLOR_WINDOW;
  RegisterClass(&window_class);

  main_window_handle = CreateWindowEx(
    WS_EX_TOPMOST, // Always on top
    WINDOW_CLASS, WINDOW_TITLE,
    0, /*WS_OVERLAPPEDWINDOW,*/
    (screen_width - window_width) / 2, (screen_height - window_height) / 2,
    window_width, window_height,
    NULL, NULL,
    hInstance, NULL
  );

  if (main_window_handle == NULL) {
    MessageBox(NULL, L"Failed to create main window!", L"Fatal error", 0);
    return 1;
  }

  // Remove title bar from the main window
  SetWindowLong(main_window_handle, GWL_STYLE, 0);

  search_handle = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    L"EDIT",
    NULL,
    WS_CHILD | WS_VISIBLE,
    0, 0,
    window_width, search_query_font_size + 2,
    main_window_handle,
    NULL, NULL, NULL
  );

  HFONT font = CreateFontA(search_query_font_size, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
  SendMessageW(search_handle, WM_SETFONT, (WPARAM) font, TRUE);

  #ifdef DEBUG
    ToggleMainWindowVisibility();
  #endif

  TrayItem tray_item = tray_item_init(main_window_handle);
  tray_item_add(&tray_item);

  HHOOK hook_handle = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
  if (hook_handle == NULL) {
    MessageBox(NULL, L"Failed to create keyboard hook!", L"Fatal error", 0);
    return 1;
  }

  MSG window_message = {};
  while (GetMessage(&window_message, NULL, 0, 0) > 0) {
    TranslateMessage(&window_message);
    DispatchMessage(&window_message);
  }

  // Clean up
  UnhookWindowsHookEx(hook_handle);
  tray_item_delete(&tray_item);
  UnregisterClass(WINDOW_CLASS, hInstance);

  return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      // All painting occurs here, between BeginPaint and EndPaint.
      FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
      EndPaint(hwnd, &ps);
      return 0;
    }
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case POPUP_MENU_TOGGLE:
          ToggleMainWindowVisibility();
          return 0;
        case POPUP_MENU_EXIT:
          PostQuitMessage(0);
          return 0;  
      }
      return 0;
    case TRAY_ITEM_EVENT:
      switch (LOWORD(lParam)) {
        // Right-click
        case 517:
          ShowPopupMenu(hwnd);
          break;
      }
      return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void ToggleMainWindowVisibility(void) {
  if (main_window_is_visible) {
    main_window_is_visible = FALSE;
    ShowWindow(main_window_handle, SW_HIDE);
  } else {
    main_window_is_visible = TRUE;
    ShowWindow(main_window_handle, SW_SHOWDEFAULT);
    SetFocus(main_window_handle);
    SetFocus(search_handle);
  }
}

void ShowPopupMenu(HWND window_handle) {
  HMENU hMenu = CreatePopupMenu();
  InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, POPUP_MENU_TOGGLE, L"Toggle");
  InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING, POPUP_MENU_EXIT, L"Exit");
  POINT point;
  GetCursorPos(&point);
  WORD cmd = TrackPopupMenu(
    hMenu,
    TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, 
    point.x, point.y, 0, window_handle, NULL
  );
  SendMessage(window_handle, WM_COMMAND, cmd, 0);
}

LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
  KBDLLHOOKSTRUCT *key_struct = (KBDLLHOOKSTRUCT*) lParam;
  switch (LOWORD(wParam)) {
    case WM_KEYDOWN:
      switch (key_struct->vkCode) {
        case VK_LMENU:
          alt_key_down = TRUE;
          break;
        case VK_LCONTROL:
          ctrl_key_down = TRUE;
          break;
        case 70:
          f_key_down = TRUE;
          break;
      }
      break;
    case WM_KEYUP:
      switch (key_struct->vkCode) {
        case VK_LMENU:
          alt_key_down = FALSE;
          break;
        case VK_LCONTROL:
          ctrl_key_down = FALSE;
          break;
        case 70:
          f_key_down = FALSE;
          break;
      }
      break;
  }
  if (ctrl_key_down && alt_key_down && f_key_down) {
    ToggleMainWindowVisibility();
  }
  return CallNextHookEx(NULL, code, wParam, lParam);
}