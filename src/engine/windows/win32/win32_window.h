#ifdef VIOLET_WIN32
#pragma once
#include "interfaces/iwindow.h"
#include <containers/containers.h>

namespace lambda
{
  namespace window
  {
    class Win32Window : public platform::IWindow
    {
    public:
      ~Win32Window();
      virtual String name() const override { return "win32"; };
      void create(const glm::uvec2& size, const char* title) override;
      void* getWindow() const override;
      bool pollMessage(platform::WindowMessage& message) override;
      void sendMessage(const platform::WindowMessage& message) override;
      void close() override;
      bool isOpen() const override;
      glm::uvec2 getSize() const override;
      float getAspectRatio() const override;
      float getDPIMultiplier() const override;
      void setSize(const glm::uvec2& size) override;
      bool showCursor() const override;
      void setShowCursor(const bool& show_cursor) override;
      void setCursorPosition(const glm::ivec2& position) override;
      glm::ivec2 getCursorPosition() const override;
      bool inFocus() const override;

      static Win32Window* getInstance(void* window);

    private:
      void pollMessage();

    private:
      glm::uvec2 size_;
      bool focus_;
      void* window_ = nullptr;
      bool is_open_ = false;
      Queue<platform::WindowMessage> messages_;
	  struct WindowIndex
	  {
		  void* ptr;
		  Win32Window* window;
	  };
	  static uint8_t kNumWindows_;
      static WindowIndex* kWindows_;
    };
  }
}
#endif