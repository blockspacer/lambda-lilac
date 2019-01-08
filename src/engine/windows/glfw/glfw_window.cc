#include "glfw_window.h"
#include <console.h>
#include "keyboard.h"

#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace lambda
{
  namespace window
  {
    io::KeyboardKeys glfwToLmbKey(const int& key)
    {
      switch (key)
      {
      case GLFW_KEY_ESCAPE:        return io::KeyboardKeys::kEscape;
      case GLFW_KEY_ENTER:         return io::KeyboardKeys::kEnter;
      case GLFW_KEY_TAB:           return io::KeyboardKeys::kTab;
      case GLFW_KEY_BACKSPACE:     return io::KeyboardKeys::kBackspace;
      case GLFW_KEY_INSERT:        return io::KeyboardKeys::kInsert;
      case GLFW_KEY_DELETE:        return io::KeyboardKeys::kDelete;
      case GLFW_KEY_RIGHT:         return io::KeyboardKeys::kRight;
      case GLFW_KEY_LEFT:          return io::KeyboardKeys::kLeft;
      case GLFW_KEY_DOWN:          return io::KeyboardKeys::kDown;
      case GLFW_KEY_UP:            return io::KeyboardKeys::kUp;
      case GLFW_KEY_PAGE_UP:       return io::KeyboardKeys::kPageUp;
      case GLFW_KEY_PAGE_DOWN:     return io::KeyboardKeys::kPageDown;
      case GLFW_KEY_HOME:          return io::KeyboardKeys::kHome;
      case GLFW_KEY_END:           return io::KeyboardKeys::kEnd;
      case GLFW_KEY_CAPS_LOCK:     return io::KeyboardKeys::kCapsLock;
      case GLFW_KEY_SCROLL_LOCK:   return io::KeyboardKeys::kScrollLock;
      case GLFW_KEY_NUM_LOCK:      return io::KeyboardKeys::kNumLock;
      case GLFW_KEY_PRINT_SCREEN:  return io::KeyboardKeys::kPrintScreen;
      case GLFW_KEY_PAUSE:         return io::KeyboardKeys::kPause;
      case GLFW_KEY_F1:            return io::KeyboardKeys::kF1;
      case GLFW_KEY_F2:            return io::KeyboardKeys::kF2;
      case GLFW_KEY_F3:            return io::KeyboardKeys::kF3;
      case GLFW_KEY_F4:            return io::KeyboardKeys::kF4;
      case GLFW_KEY_F5:            return io::KeyboardKeys::kF5;
      case GLFW_KEY_F6:            return io::KeyboardKeys::kF6;
      case GLFW_KEY_F7:            return io::KeyboardKeys::kF7;
      case GLFW_KEY_F8:            return io::KeyboardKeys::kF8;
      case GLFW_KEY_F9:            return io::KeyboardKeys::kF9;
      case GLFW_KEY_F10:           return io::KeyboardKeys::kF10;
      case GLFW_KEY_F11:           return io::KeyboardKeys::kF11;
      case GLFW_KEY_F12:           return io::KeyboardKeys::kF12;
      case GLFW_KEY_F13:           return io::KeyboardKeys::kF13;
      case GLFW_KEY_F14:           return io::KeyboardKeys::kF14;
      case GLFW_KEY_F15:           return io::KeyboardKeys::kF15;
      case GLFW_KEY_F16:           return io::KeyboardKeys::kF16;
      case GLFW_KEY_F17:           return io::KeyboardKeys::kF17;
      case GLFW_KEY_F18:           return io::KeyboardKeys::kF18;
      case GLFW_KEY_F19:           return io::KeyboardKeys::kF19;
      case GLFW_KEY_F20:           return io::KeyboardKeys::kF20;
      case GLFW_KEY_F21:           return io::KeyboardKeys::kF21;
      case GLFW_KEY_F22:           return io::KeyboardKeys::kF22;
      case GLFW_KEY_F23:           return io::KeyboardKeys::kF23;
      case GLFW_KEY_F24:           return io::KeyboardKeys::kF24;
      case GLFW_KEY_F25:           return io::KeyboardKeys::kF25;
      case GLFW_KEY_KP_0:          return io::KeyboardKeys::kNum0;
      case GLFW_KEY_KP_1:          return io::KeyboardKeys::kNum1;
      case GLFW_KEY_KP_2:          return io::KeyboardKeys::kNum2;
      case GLFW_KEY_KP_3:          return io::KeyboardKeys::kNum3;
      case GLFW_KEY_KP_4:          return io::KeyboardKeys::kNum4;
      case GLFW_KEY_KP_5:          return io::KeyboardKeys::kNum5;
      case GLFW_KEY_KP_6:          return io::KeyboardKeys::kNum6;
      case GLFW_KEY_KP_7:          return io::KeyboardKeys::kNum7;
      case GLFW_KEY_KP_8:          return io::KeyboardKeys::kNum8;
      case GLFW_KEY_KP_9:          return io::KeyboardKeys::kNum9;
      case GLFW_KEY_KP_DECIMAL:    return io::KeyboardKeys::kDecimal;
      case GLFW_KEY_KP_DIVIDE:     return io::KeyboardKeys::kDivide;
      case GLFW_KEY_KP_MULTIPLY:   return io::KeyboardKeys::kMultiply;
      case GLFW_KEY_KP_SUBTRACT:   return io::KeyboardKeys::kSubtract;
      case GLFW_KEY_KP_ADD:        return io::KeyboardKeys::kAdd;
      case GLFW_KEY_KP_ENTER:      return io::KeyboardKeys::kEnter;
      case GLFW_KEY_KP_EQUAL:      return io::KeyboardKeys::kEqual;
      case GLFW_KEY_LEFT_SHIFT:    return io::KeyboardKeys::kLShift;
      case GLFW_KEY_LEFT_CONTROL:  return io::KeyboardKeys::kLControl;
      case GLFW_KEY_LEFT_ALT:      return io::KeyboardKeys::kLAlt;
      case GLFW_KEY_LEFT_SUPER:    return io::KeyboardKeys::kLSuper;
      case GLFW_KEY_RIGHT_SHIFT:   return io::KeyboardKeys::kRShift;
      case GLFW_KEY_RIGHT_CONTROL: return io::KeyboardKeys::kRControl;
      case GLFW_KEY_RIGHT_ALT:     return io::KeyboardKeys::kRAlt;
      case GLFW_KEY_RIGHT_SUPER:   return io::KeyboardKeys::kRSuper;
      case GLFW_KEY_MENU:          return io::KeyboardKeys::kMenu;
      default:                     return (io::KeyboardKeys)key;
      }
    }
    void GLFWWindow::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int modifiers)
    {
      platform::WindowMessage message;
      message.type    = platform::WindowMessageType::kMouseButton;
      message.data[0] = (uint32_t)button;
      message.data[1] = (uint32_t)action;
      getInstance(window)->sendMessage(message);
    }

    void GLFWWindow::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
    {
      // TODO (Hilze): Make this cross platform.
      platform::WindowMessage message;
      message.type = platform::WindowMessageType::kKeyboardButton;
      message.data[0] = (uint32_t)glfwToLmbKey(key);
      message.data[1] = (uint32_t)action;
      getInstance(window)->sendMessage(message);
    }

    void GLFWWindow::glfwCursorPosCallback(GLFWwindow* window, double x, double y)
    {
      platform::WindowMessage message;
      message.type = platform::WindowMessageType::kMouseMove;
      message.data[0] = (uint32_t)x;
      message.data[1] = (uint32_t)y;
      getInstance(window)->sendMessage(message);
    }
    void GLFWWindow::glfwCharCallback(GLFWwindow* window, unsigned int ch)
    {
      platform::WindowMessage message;
      message.type = platform::WindowMessageType::kChar;
      message.data[0] = (uint32_t)glfwToLmbKey(ch);
      getInstance(window)->sendMessage(message);
    }
    void GLFWWindow::glfwCharCallbackMod(GLFWwindow* window, unsigned int ch, int modifiers)
    {
      platform::WindowMessage message;
      message.type = platform::WindowMessageType::kChar;
      message.data[0] = (uint32_t)glfwToLmbKey(ch);
      getInstance(window)->sendMessage(message);
    }
    void GLFWWindow::glfwCloseCallback(GLFWwindow* window)
    {
      foundation::Error("Close window!");
      platform::WindowMessage message;
      message.type = platform::WindowMessageType::kClose;
      getInstance(window)->sendMessage(message);
    }
    void glfwErrorCallback(int error, const char* description)
    {
      LMB_LOG_ERR("GLFW: %s\n", description);
    }

    GLFWWindow::~GLFWWindow()
    {
      if (window_ != nullptr)
      {
        GetWindows().erase(window_);
        close();
      }
    }
    void GLFWWindow::create(const glm::uvec2& size, const char* title)
    {
      static bool initalized = false;
      if (!initalized)
      {
        initalized = true;
        LMB_ASSERT(glfwInit() > 0 ? true : false, "Failed to initialize GLFW");

        glfwSetErrorCallback(glfwErrorCallback);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      }

      window_ = glfwCreateWindow(size.x, size.y, title, nullptr, nullptr);
      glfwSetMouseButtonCallback(window_, glfwMouseButtonCallback);
      glfwSetKeyCallback(window_, glfwKeyCallback);
      glfwSetCursorPosCallback(window_, glfwCursorPosCallback);
      //glfwSetCharCallback(window_, glfwCharCallback);
      glfwSetCharModsCallback(window_, glfwCharCallbackMod);
      glfwSetWindowCloseCallback(window_, glfwCloseCallback);

      is_open_ = true;
      GetWindows().insert(eastl::make_pair(window_, this));
    }
    void* GLFWWindow::getWindow() const
    {
#ifdef WIN32
      return glfwGetWin32Window(window_);
#else
      return window_;
#endif
    }
    bool GLFWWindow::pollMessage(platform::WindowMessage& message)
    {
      if (window_ == nullptr)
      {
        return false;
      }
      /*if (glfwWindowShouldClose(window_))
      {
        message.type = platform::WindowMessageType::kClose;
        return true;
      }*/

      pollMessage();

      if (messages_.empty() == false)
      {
        message = messages_.front();
        messages_.pop();
        if (message.type == platform::WindowMessageType::kResize)
        {
          size_ = glm::uvec2(message.data[0], message.data[1]);
        }
        return true;
      }

      return false;
    }
    void GLFWWindow::sendMessage(const platform::WindowMessage& message)
    {
      messages_.push(message);
    }
    void GLFWWindow::close()
    {
      if (is_open_ == true)
      {
        is_open_ = false;
        glfwDestroyWindow(window_);
        window_ = nullptr;
      }
    }
    bool GLFWWindow::isOpen() const
    {
      return is_open_;
    }
    glm::uvec2 GLFWWindow::getSize() const
    {
      glm::ivec2 size;
      glfwGetWindowSize(window_,&size.x,&size.y);
      return (glm::uvec2)size;
    }
    float GLFWWindow::getAspectRatio() const
    {
      return (float)getSize().x / (float)getSize().y;
    }
    float GLFWWindow::getDPIMultiplier() const
    {
      // TODO (Hilze): Add this.
      return 1.0f;
    }
    void GLFWWindow::setSize(const glm::uvec2& size)
    {
      glfwSetWindowSize(window_, (int)size.x, (int)size.y);
    }
    bool GLFWWindow::showCursor() const
    {
      return glfwGetInputMode(window_, GLFW_CURSOR) == GLFW_CURSOR_NORMAL ? true : false;
    }
    void GLFWWindow::setShowCursor(const bool& show_cursor)
    {
      glfwSetInputMode(window_, GLFW_CURSOR, show_cursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
    void GLFWWindow::setCursorPosition(const glm::ivec2& position)
    {
      glfwSetCursorPos(window_, (double)position.x, (double)position.y);
    }
    glm::ivec2 GLFWWindow::getCursorPosition() const
    {
      glm::dvec2 pos;
      glfwGetCursorPos(window_,&pos.x,&pos.y);
      return (glm::ivec2)pos;
    }
    bool GLFWWindow::inFocus() const
    {
      return glfwGetWindowAttrib(window_, GLFW_FOCUSED) > 0 ? true : false;
    }
    GLFWWindow* GLFWWindow::getInstance(GLFWwindow* window)
    {
      auto it = GetWindows().find(window);
      if (it != GetWindows().end())
      {
        return it->second;
      }
      else
      {
        return nullptr;
      }
    }
    void GLFWWindow::pollMessage()
    {
      glfwPollEvents();
    }
    UnorderedMap<GLFWwindow*, GLFWWindow*>& GLFWWindow::GetWindows()
    {
      static UnorderedMap<GLFWwindow*, GLFWWindow*> windows_;
      return windows_;
    }
  }
}