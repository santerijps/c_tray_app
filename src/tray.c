#include "tray.h"

#ifndef _STRSAFE_H_INCLUDED_
  #include <strsafe.h>
#endif

TrayItem tray_item_init(HWND window_handle) {
  TrayItem tray_item = {};
  tray_item.cbSize = sizeof(TrayItem);
  tray_item.hWnd = window_handle;
  tray_item.hIcon = LoadIcon(NULL, IDI_INFORMATION);
  tray_item.uCallbackMessage = TRAY_ITEM_EVENT;
  tray_item.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
  tray_item.uVersion = NOTIFYICON_VERSION_4;
  StringCchCopy(tray_item.szTip, 128, TEXT("C Tray Application"));
  return tray_item;
}

BOOL tray_item_add(TrayItem *tray_item) {
  return Shell_NotifyIcon(NIM_ADD, tray_item);
}

BOOL tray_item_delete(TrayItem *tray_item) {
  return Shell_NotifyIcon(NIM_DELETE, tray_item);
}