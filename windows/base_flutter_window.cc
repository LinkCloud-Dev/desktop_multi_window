//
// Created by yangbin on 2022/1/27.
//

#include "base_flutter_window.h"

namespace {
void CenterRectToMonitor(LPRECT prc) {
  HMONITOR hMonitor;
  MONITORINFO mi;
  RECT rc;
  int w = prc->right - prc->left;
  int h = prc->bottom - prc->top;

  //
  // get the nearest monitor to the passed rect.
  //
  hMonitor = MonitorFromRect(prc, MONITOR_DEFAULTTONEAREST);

  //
  // get the work area or entire monitor rect.
  //
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);

  rc = mi.rcMonitor;

  prc->left = rc.left + (rc.right - rc.left - w) / 2;
  prc->top = rc.top + (rc.bottom - rc.top - h) / 2;
  prc->right = prc->left + w;
  prc->bottom = prc->top + h;

}

std::wstring Utf16FromUtf8(const std::string &string) {
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, nullptr, 0);
  if (size_needed == 0) {
    return {};
  }
  std::wstring wstrTo(size_needed, 0);
  int converted_length = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, &wstrTo[0], size_needed);
  if (converted_length == 0) {
    return {};
  }
  return wstrTo;
}

}

void BaseFlutterWindow::Center() {
  auto handle = GetWindowHandle();
  if (!handle) {
    return;
  }
  RECT rc;
  GetWindowRect(handle, &rc);
  CenterRectToMonitor(&rc);
  SetWindowPos(handle, nullptr, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void BaseFlutterWindow::SetBounds(double_t x, double_t y, double_t width, double_t height) {
  auto handle = GetWindowHandle();
  if (!handle) {
    return;
  }
  MoveWindow(handle, int32_t(x), int32_t(y),
             static_cast<int>(width),
             static_cast<int>(height),
             TRUE);
}

void BaseFlutterWindow::SetTitle(const std::string &title) {
  auto handle = GetWindowHandle();
  if (!handle) {
    return;
  }
  SetWindowText(handle, Utf16FromUtf8(title).c_str());
}

void BaseFlutterWindow::SetFullScreen(bool isFullScreen) {

  HWND mainWindow = GetWindowHandle();

  // Inspired by how Chromium does this
  // https://src.chromium.org/viewvc/chrome/trunk/src/ui/views/win/fullscreen_handler.cc?revision=247204&view=markup

  // Save current window state if not already fullscreen.
  if (!g_is_window_fullscreen) {
    // Save current window information.
    g_maximized_before_fullscreen = !!::IsZoomed(mainWindow);
    g_style_before_fullscreen = GetWindowLong(mainWindow, GWL_STYLE);
    g_ex_style_before_fullscreen = GetWindowLong(mainWindow, GWL_EXSTYLE);
    if (g_maximized_before_fullscreen) {
      SendMessage(mainWindow, WM_SYSCOMMAND, SC_RESTORE, 0);
    }
    ::GetWindowRect(mainWindow, &g_frame_before_fullscreen);
    g_is_frameless_before_fullscreen = is_frameless_;
  }

  if (isFullScreen) {
    // Set new window style and size.
    ::SetWindowLong(mainWindow, GWL_STYLE,
                    g_style_before_fullscreen & ~(WS_CAPTION | WS_THICKFRAME));
    ::SetWindowLong(mainWindow, GWL_EXSTYLE,
                    g_ex_style_before_fullscreen &
                        ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE |
                          WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(monitor_info);
    ::GetMonitorInfo(::MonitorFromWindow(mainWindow, MONITOR_DEFAULTTONEAREST),
                     &monitor_info);
    ::SetWindowPos(mainWindow, NULL, monitor_info.rcMonitor.left,
                   monitor_info.rcMonitor.top,
                   monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                   monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    ::SendMessage(mainWindow, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
  } else {
    ::SetWindowLong(mainWindow, GWL_STYLE, g_style_before_fullscreen);
    ::SetWindowLong(mainWindow, GWL_EXSTYLE, g_ex_style_before_fullscreen);

    SendMessage(mainWindow, WM_SYSCOMMAND, SC_RESTORE, 0);

    if (g_is_frameless_before_fullscreen)
      SetAsFrameless();

    if (g_maximized_before_fullscreen) {
      Maximize(false);
    } else {
      ::SetWindowPos(
          mainWindow, NULL, g_frame_before_fullscreen.left,
          g_frame_before_fullscreen.top,
          g_frame_before_fullscreen.right - g_frame_before_fullscreen.left,
          g_frame_before_fullscreen.bottom - g_frame_before_fullscreen.top,
          SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
  }

  g_is_window_fullscreen = isFullScreen;
}

void BaseFlutterWindow::Maximize(const bool vertically) {
  HWND hwnd = GetWindowHandle();
  WINDOWPLACEMENT windowPlacement;
  GetWindowPlacement(hwnd, &windowPlacement);

  if (vertically) {
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    PostMessage(hwnd, WM_NCLBUTTONDBLCLK, HTTOP,
                MAKELPARAM(cursorPos.x, cursorPos.y));
  } else {
    if (windowPlacement.showCmd != SW_MAXIMIZE) {
      PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    }
  }
}

void BaseFlutterWindow::SetAsFrameless() {
  is_frameless_ = true;
  HWND hWnd = GetWindowHandle();

  RECT rect;

  GetWindowRect(hWnd, &rect);
  SetWindowPos(hWnd, nullptr, rect.left, rect.top, rect.right - rect.left,
               rect.bottom - rect.top,
               SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE |
                   SWP_FRAMECHANGED);
}

void BaseFlutterWindow::Close() {
  auto handle = GetWindowHandle();
  if (!handle) {
    return;
  }
  PostMessage(handle, WM_SYSCOMMAND, SC_CLOSE, 0);
}

void BaseFlutterWindow::Show() {
  auto handle = GetWindowHandle();
  if (!handle) {
    return;
  }
  ShowWindow(handle, SW_SHOW);

}

void BaseFlutterWindow::Hide() {
  auto handle = GetWindowHandle();
  if (!handle) {
    return;
  }
  ShowWindow(handle, SW_HIDE);
}
