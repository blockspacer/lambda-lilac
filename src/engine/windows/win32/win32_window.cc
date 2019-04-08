#ifdef VIOLET_WIN32
#include "win32_window.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <iostream>
#include <memory/memory.h>
#ifdef HIGH_DPI
#include <ShellScalingAPI.h>
#endif
#undef min
#undef max

namespace lambda
{
  namespace window
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Win32Window::WindowIndex* Win32Window::kWindows_ = nullptr;
	uint8_t Win32Window::kNumWindows_ = 0u;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    __int64 WndProc(void* h_wnd, unsigned int message, unsigned __int64 word_param, __int64 long_param)
    {
      platform::WindowMessage msg;

      switch (message)
      {
      case WM_CLOSE:
        msg.type = platform::WindowMessageType::kClose;
        break;
      case WM_DESTROY:
        PostQuitMessage(0);
        break;
      case WM_SETFOCUS:
        msg.type = platform::WindowMessageType::kFocus;
        msg.data[0] = 1;
        break;
      case WM_KILLFOCUS:
        msg.type = platform::WindowMessageType::kFocus;
        msg.data[0] = 0;
        break;
      case WM_SIZE:
        msg.type = platform::WindowMessageType::kResize;
        msg.data[0] = LOWORD(long_param); // Width.
        msg.data[1] = HIWORD(long_param); // Height.
        break;
      case WM_KEYDOWN:
        msg.type    = platform::WindowMessageType::kKeyboardButton;
        msg.data[0] = (uint32_t)word_param;
        msg.data[1] = 1;
        break;
      case WM_KEYUP:
        msg.type    = platform::WindowMessageType::kKeyboardButton;
        msg.data[0] = (uint32_t)word_param;
        msg.data[1] = 0;
        break;
      case WM_LBUTTONDOWN:
        msg.type = platform::WindowMessageType::kMouseButton;
        msg.data[0] = 0;
        msg.data[1] = 1;
        break;
      case WM_LBUTTONUP:
        msg.type = platform::WindowMessageType::kMouseButton;
        msg.data[0] = 0;
        msg.data[1] = 0;
        break;
      case WM_MBUTTONDOWN:
        msg.type = platform::WindowMessageType::kMouseButton;
        msg.data[0] = 1;
        msg.data[1] = 1;
        break;
      case WM_MBUTTONUP:
        msg.type = platform::WindowMessageType::kMouseButton;
        msg.data[0] = 1;
        msg.data[1] = 0;
        break;
      case WM_RBUTTONDOWN:
        msg.type = platform::WindowMessageType::kMouseButton;
        msg.data[0] = 2;
        msg.data[1] = 1;
        break;
      case WM_RBUTTONUP:
        msg.type = platform::WindowMessageType::kMouseButton;
        msg.data[0] = 2;
        msg.data[1] = 0;
        break;
      case WM_LBUTTONDBLCLK:
        msg.type = platform::WindowMessageType::kMouseButton;
        msg.data[0] = 3;
        msg.data[1] = 1;
        break;
      case WM_MBUTTONDBLCLK:
        msg.type = platform::WindowMessageType::kMouseButton;
        msg.data[0] = 4;
        msg.data[1] = 1;
        break;
      case WM_RBUTTONDBLCLK:
        msg.type = platform::WindowMessageType::kMouseButton;
        msg.data[0] = 5;
        msg.data[1] = 1;
        break;
      case WM_MOUSEMOVE:
        msg.type = platform::WindowMessageType::kMouseMove;
        msg.data[0] = (uint32_t)GET_X_LPARAM(long_param);
        msg.data[1] = (uint32_t)GET_Y_LPARAM(long_param);
        break;
      case WM_MOUSEWHEEL:
        msg.type = platform::WindowMessageType::kMouseScroll;
        msg.data[0] = GET_WHEEL_DELTA_WPARAM(word_param);
        break;
      case WM_CHAR:
        if (word_param > 0&& word_param < 0x10000)
        {
          msg.type = platform::WindowMessageType::kChar;
          msg.data[0] = (uint32_t)word_param;
        }
        break;
      default:
        return DefWindowProc((HWND)h_wnd, message, word_param, long_param);
      }

      Win32Window* window = Win32Window::getInstance(h_wnd);
      if (window != nullptr&& msg.type != platform::WindowMessageType::kUnknown)
      {
        window->sendMessage(msg);
      }
      return 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Win32Window::~Win32Window()
    {
      if (window_ != nullptr)
      {
        close();
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Win32Window::create(const glm::uvec2& size, const char* title)
    {
      static size_t window_idx = 0u;
#ifdef HIGH_DPI
      SetProcessDPIAware();
      SetProcessDpiAwareness(PROCESS_DPI_AWARENESS::PROCESS_PER_MONITOR_DPI_AWARE);
#endif

      WNDCLASSEX wc;
      HWND hwnd;
      HINSTANCE h_instance = GetModuleHandle(NULL);
      
      String class_name = title + toString(window_idx++);

      //Step 1: Registering the Window Class
      wc.cbSize = sizeof(WNDCLASSEX);
      wc.style = /*CS_DBLCLKS*/0;
      wc.lpfnWndProc = (WNDPROC)WndProc;
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hInstance = h_instance;
      wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
      wc.lpszMenuName = NULL;
      wc.lpszClassName = class_name.c_str();
      wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

      if (!RegisterClassEx(&wc))
      {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
          MB_ICONEXCLAMATION | MB_OK);
        return;
      }

      // Step 2: Creating the Window
      hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        class_name.c_str(),
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 512, 512,
        NULL, NULL, h_instance, NULL);

      if (hwnd == NULL)
      {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
          MB_ICONEXCLAMATION | MB_OK);
        return;
      }

      ShowWindow(hwnd, 1);
      UpdateWindow(hwnd);

      window_  = hwnd;
      focus_   = false;
      is_open_ = true;

	  kWindows_ = (WindowIndex*)foundation::Memory::reallocate(kWindows_, ++kNumWindows_ * sizeof(WindowIndex));
	  kWindows_[kNumWindows_ - 1].ptr    = window_;
	  kWindows_[kNumWindows_ - 1].window = this;
      setSize(size);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void* Win32Window::getWindow() const
    {
      return window_;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Win32Window::pollMessage(platform::WindowMessage& message)
    {
      pollMessage();

      if (messages_.empty() == false)
      {
        message = messages_.front();
        messages_.pop();
        if (message.type == platform::WindowMessageType::kResize)
        {
          size_ = glm::uvec2(message.data[0], message.data[1]);
        }
        if (message.type == platform::WindowMessageType::kFocus)
        {
          focus_ = message.data[0] > 0 ? true : false;
        }
        return true;
      }

      return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Win32Window::close()
    {
      if (is_open_ == true)
      {
        is_open_ = false;

		for (uint8_t i = 0; i < kNumWindows_; ++i)
		{
			if (kWindows_[i].ptr == window_)
			{
				memcpy(&kWindows_[i], &kWindows_[--kNumWindows_], sizeof(WindowIndex));
				kWindows_ = (WindowIndex*)foundation::Memory::reallocate(kWindows_, kNumWindows_ * sizeof(WindowIndex));
				break;
			}
		}

        DestroyWindow((HWND)window_);
        window_ = nullptr;
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Win32Window::isOpen() const
    {
      return is_open_;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    glm::uvec2 Win32Window::getSize() const
    {
      return size_;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float Win32Window::getAspectRatio() const
    {
      return (float)getSize().x / (float)getSize().y;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float Win32Window::getDPIMultiplier() const
    {
#ifdef HIGH_DPI
      return (float)GetDpiForWindow((HWND)window_) / 96.0f;
#else
      return 1.0f;
#endif
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Win32Window::setSize(const glm::uvec2& size)
    {
      RECT wr = { 0, 0, (LONG)size.x, (LONG)size.y };
      AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

      glm::ivec2 window_size(
        wr.right - wr.left,
        wr.bottom - wr.top
      );

      RECT wp;
      GetWindowRect((HWND)window_,&wp);

      glm::ivec2 window_position(
        wp.left,
        wp.top
      );

      SetWindowPos((HWND)window_, 0, window_position.x, window_position.y, window_size.x, window_size.y, 0);

      size_ = size;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Win32Window::showCursor() const
    {
      CURSORINFO cursor_info{};
      GetCursorInfo(&cursor_info);
      return cursor_info.flags == CURSOR_SHOWING;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Win32Window::setShowCursor(const bool& show_cursor)
    {
      ShowCursor(show_cursor);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Win32Window::setCursorPosition(const glm::ivec2& position)
    {
      POINT point;
      point.x = position.x;
      point.y = position.y;
      ClientToScreen((HWND)window_,&point);
      
      if (size_.y % 2 != 0)
      {
        point.y -= 1;
      }

      SetCursorPos(point.x, point.y);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    glm::ivec2 Win32Window::getCursorPosition() const
    {
      POINT point;
      GetCursorPos(&point);
      ScreenToClient((HWND)window_,&point);
      return glm::ivec2(point.x, point.y);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Win32Window::inFocus() const
    {
      return GetActiveWindow() == (HWND)window_;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Win32Window::sendMessage(const platform::WindowMessage& message)
    {
      messages_.push(message);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Win32Window* Win32Window::getInstance(void* window)
	{
		for (uint8_t i = 0; i < kNumWindows_; ++i)
			if (kWindows_[i].ptr == window)
				return kWindows_[i].window;

		return nullptr;
	}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Win32Window::pollMessage()
    {
      MSG msg;
      while (PeekMessage(&msg, (HWND)window_, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }
}
#endif