#pragma once
#include <glm/vec2.hpp>
#include <containers/containers.h>

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    enum class WindowMessageType : uint8_t
    {
      kClose,
      kFocus,
      kResize,
      kMouseButton,
      kMouseMove,
      kMouseScroll,
      kKeyboardButton,
      kChar,
      kKeyboardModifier,
      kUnknown // Just for now, while this class is still being developed. 
               // Should be removed soon.
    };

    ///////////////////////////////////////////////////////////////////////////
    struct WindowMessage
    {
      WindowMessageType type = WindowMessageType::kUnknown;
      uint32_t data[2];
    };

    ///////////////////////////////////////////////////////////////////////////
    class IWindow
    {
    public:
      virtual ~IWindow() {};
      virtual String name() const = 0;
      virtual void create(const glm::uvec2& size, const char* title) = 0;
      virtual void* getWindow() const = 0;
      virtual bool pollMessage(WindowMessage& message) = 0;
      virtual void sendMessage(const WindowMessage& message) = 0;
      virtual void close() = 0;
      virtual bool isOpen() const = 0;
      virtual glm::uvec2 getSize() const = 0;
      virtual float getAspectRatio() const = 0;
      virtual float getDPIMultiplier() const = 0;
      virtual void setSize(const glm::uvec2& size) = 0;
      virtual bool showCursor() const = 0;
      virtual void setShowCursor(const bool& show_cursor) = 0;
      virtual void setCursorPosition(const glm::ivec2& position) = 0;
      virtual glm::ivec2 getCursorPosition() const = 0;
      virtual bool inFocus() const = 0;
    };
  }
}