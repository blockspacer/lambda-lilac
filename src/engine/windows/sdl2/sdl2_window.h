#pragma once
#include "interfaces/iwindow.h"
#include <containers/containers.h>

struct SDL_Window;

namespace lambda
{
  namespace window
  {
    class SDL2Window : public platform::IWindow
    {
    public:
      ~SDL2Window();
      virtual String name() const override { return "sdl2"; };
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

      static SDL2Window* getInstance(SDL_Window* window);

    private:
      void pollMessage();

      static UnorderedMap<SDL_Window*, SDL2Window*>& GetWindows();
    private:
      glm::uvec2 size_;
      SDL_Window* window_ = nullptr;
      bool focus_ = false;
      bool is_open_ = false;
      Queue<platform::WindowMessage> messages_;
    };
  }
}