//
// Created by yangbin on 2022/1/27.
//

#ifndef MULTI_WINDOW_WINDOWS_BASE_FLUTTER_WINDOW_H_
#define MULTI_WINDOW_WINDOWS_BASE_FLUTTER_WINDOW_H_

#include "window_channel.h"

class BaseFlutterWindow {

 public:
  bool is_frameless_ = false;

  virtual ~BaseFlutterWindow() = default;

  virtual WindowChannel *GetWindowChannel() = 0;

  void Show();

  void Hide();

  void Close();

  void SetTitle(const std::string &title);

  void SetFullScreen(bool isFullScreen);

  void Maximize(bool vertically);

  void SetAsFrameless();

  void SetBounds(double_t x, double_t y, double_t width, double_t height);

  void Center();

 protected:

  virtual HWND GetWindowHandle() = 0;
  
 private:

  bool g_is_window_fullscreen = false;

  std::string g_title_bar_style_before_fullscreen;

  bool g_is_frameless_before_fullscreen;

  RECT g_frame_before_fullscreen;

  bool g_maximized_before_fullscreen;

  LONG g_style_before_fullscreen;

  LONG g_ex_style_before_fullscreen;
};

#endif //MULTI_WINDOW_WINDOWS_BASE_FLUTTER_WINDOW_H_
