#pragma once

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif

#define TRAY_ITEM_EVENT (WM_USER + 0x100)

typedef NOTIFYICONDATA TrayItem;
TrayItem tray_item_init(HWND window_handle);
BOOL tray_item_add(TrayItem *tray_item);
BOOL tray_item_delete(TrayItem *tray_item);