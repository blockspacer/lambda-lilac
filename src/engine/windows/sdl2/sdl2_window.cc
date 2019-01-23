#include "sdl2_window.h"
#include <utils/console.h>

#include <SDL.h>
#include <SDL_syswm.h>
#include "input/mouse.h"
#include "input/keyboard.h"

namespace lambda
{
  namespace window
  {
    io::MouseButtons sdl2ToLmbButton(const uint8_t& button)
    {
      switch (button)
      {
      case SDL_BUTTON_LEFT:   return io::MouseButtons::kLMB;
      case SDL_BUTTON_MIDDLE: return io::MouseButtons::kMMB;
      case SDL_BUTTON_RIGHT:  return io::MouseButtons::kRMB;
      default:                return (io::MouseButtons)button;
      }
    }

    io::KeyboardKeys sdl2ToLmbKey(const unsigned int& key)
    {
      switch (key)
      {
      case SDL_SCANCODE_A: return io::KeyboardKeys::kA;
      case SDL_SCANCODE_B: return io::KeyboardKeys::kB;
      case SDL_SCANCODE_C: return io::KeyboardKeys::kC;
      case SDL_SCANCODE_D: return io::KeyboardKeys::kD;
      case SDL_SCANCODE_E: return io::KeyboardKeys::kE;
      case SDL_SCANCODE_F: return io::KeyboardKeys::kF;
      case SDL_SCANCODE_G: return io::KeyboardKeys::kG;
      case SDL_SCANCODE_H: return io::KeyboardKeys::kH;
      case SDL_SCANCODE_I: return io::KeyboardKeys::kI;
      case SDL_SCANCODE_J: return io::KeyboardKeys::kJ;
      case SDL_SCANCODE_K: return io::KeyboardKeys::kK;
      case SDL_SCANCODE_L: return io::KeyboardKeys::kL;
      case SDL_SCANCODE_M: return io::KeyboardKeys::kM;
      case SDL_SCANCODE_N: return io::KeyboardKeys::kN;
      case SDL_SCANCODE_O: return io::KeyboardKeys::kO;
      case SDL_SCANCODE_P: return io::KeyboardKeys::kP;
      case SDL_SCANCODE_Q: return io::KeyboardKeys::kQ;
      case SDL_SCANCODE_R: return io::KeyboardKeys::kR;
      case SDL_SCANCODE_S: return io::KeyboardKeys::kS;
      case SDL_SCANCODE_T: return io::KeyboardKeys::kT;
      case SDL_SCANCODE_U: return io::KeyboardKeys::kU;
      case SDL_SCANCODE_V: return io::KeyboardKeys::kV;
      case SDL_SCANCODE_W: return io::KeyboardKeys::kW;
      case SDL_SCANCODE_X: return io::KeyboardKeys::kX;
      case SDL_SCANCODE_Y: return io::KeyboardKeys::kY;
      case SDL_SCANCODE_Z: return io::KeyboardKeys::kZ;
      case SDL_SCANCODE_1: return io::KeyboardKeys::k1;
      case SDL_SCANCODE_2: return io::KeyboardKeys::k2;
      case SDL_SCANCODE_3: return io::KeyboardKeys::k3;
      case SDL_SCANCODE_4: return io::KeyboardKeys::k4;
      case SDL_SCANCODE_5: return io::KeyboardKeys::k5;
      case SDL_SCANCODE_6: return io::KeyboardKeys::k6;
      case SDL_SCANCODE_7: return io::KeyboardKeys::k7;
      case SDL_SCANCODE_8: return io::KeyboardKeys::k8;
      case SDL_SCANCODE_9: return io::KeyboardKeys::k9;
      case SDL_SCANCODE_0: return io::KeyboardKeys::k0;
      case SDL_SCANCODE_RETURN: return io::KeyboardKeys::kReturn;
      case SDL_SCANCODE_ESCAPE: return io::KeyboardKeys::kEscape;
      case SDL_SCANCODE_BACKSPACE: return io::KeyboardKeys::kBackspace;
      case SDL_SCANCODE_TAB: return io::KeyboardKeys::kTab;
      case SDL_SCANCODE_SPACE: return io::KeyboardKeys::kSpace;
      case SDL_SCANCODE_MINUS: return io::KeyboardKeys::kMinus;
      case SDL_SCANCODE_EQUALS: return io::KeyboardKeys::kEqual;
      case SDL_SCANCODE_LEFTBRACKET: return io::KeyboardKeys::kLeftBracket;
      case SDL_SCANCODE_RIGHTBRACKET: return io::KeyboardKeys::kRightBracket;
      case SDL_SCANCODE_BACKSLASH: return io::KeyboardKeys::kBackSlash;
      case SDL_SCANCODE_SEMICOLON: return io::KeyboardKeys::kSemicolon;
      case SDL_SCANCODE_APOSTROPHE: return io::KeyboardKeys::kApostrophe;
      case SDL_SCANCODE_GRAVE: return io::KeyboardKeys::kGrave;
      case SDL_SCANCODE_COMMA: return io::KeyboardKeys::kComma;
      case SDL_SCANCODE_PERIOD: return io::KeyboardKeys::kPeriod;
      case SDL_SCANCODE_SLASH: return io::KeyboardKeys::kSlash;
      case SDL_SCANCODE_CAPSLOCK: return io::KeyboardKeys::kCapsLock;
      case SDL_SCANCODE_F1: return io::KeyboardKeys::kF1;
      case SDL_SCANCODE_F2: return io::KeyboardKeys::kF2;
      case SDL_SCANCODE_F3: return io::KeyboardKeys::kF3;
      case SDL_SCANCODE_F4: return io::KeyboardKeys::kF4;
      case SDL_SCANCODE_F5: return io::KeyboardKeys::kF5;
      case SDL_SCANCODE_F6: return io::KeyboardKeys::kF6;
      case SDL_SCANCODE_F7: return io::KeyboardKeys::kF7;
      case SDL_SCANCODE_F8: return io::KeyboardKeys::kF8;
      case SDL_SCANCODE_F9: return io::KeyboardKeys::kF9;
      case SDL_SCANCODE_F10: return io::KeyboardKeys::kF10;
      case SDL_SCANCODE_F11: return io::KeyboardKeys::kF11;
      case SDL_SCANCODE_F12: return io::KeyboardKeys::kF12;
      case SDL_SCANCODE_F13: return io::KeyboardKeys::kF13;
      case SDL_SCANCODE_F14: return io::KeyboardKeys::kF14;
      case SDL_SCANCODE_F15: return io::KeyboardKeys::kF15;
      case SDL_SCANCODE_F16: return io::KeyboardKeys::kF16;
      case SDL_SCANCODE_F17: return io::KeyboardKeys::kF17;
      case SDL_SCANCODE_F18: return io::KeyboardKeys::kF18;
      case SDL_SCANCODE_F19: return io::KeyboardKeys::kF19;
      case SDL_SCANCODE_F20: return io::KeyboardKeys::kF20;
      case SDL_SCANCODE_F21: return io::KeyboardKeys::kF21;
      case SDL_SCANCODE_F22: return io::KeyboardKeys::kF22;
      case SDL_SCANCODE_F23: return io::KeyboardKeys::kF23;
      case SDL_SCANCODE_F24: return io::KeyboardKeys::kF24;
      case SDL_SCANCODE_PRINTSCREEN: return io::KeyboardKeys::kPrintScreen;
      case SDL_SCANCODE_SCROLLLOCK: return io::KeyboardKeys::kScrollLock;
      case SDL_SCANCODE_PAUSE: return io::KeyboardKeys::kPause;
      case SDL_SCANCODE_INSERT: return io::KeyboardKeys::kInsert;
      case SDL_SCANCODE_HOME: return io::KeyboardKeys::kHome;
      case SDL_SCANCODE_PAGEUP: return io::KeyboardKeys::kPageUp;
      case SDL_SCANCODE_DELETE: return io::KeyboardKeys::kDelete;
      case SDL_SCANCODE_END: return io::KeyboardKeys::kEnd;
      case SDL_SCANCODE_PAGEDOWN: return io::KeyboardKeys::kPageDown;
      case SDL_SCANCODE_RIGHT: return io::KeyboardKeys::kRight;
      case SDL_SCANCODE_LEFT: return io::KeyboardKeys::kLeft;
      case SDL_SCANCODE_DOWN: return io::KeyboardKeys::kDown;
      case SDL_SCANCODE_UP: return io::KeyboardKeys::kUp;
      case SDL_SCANCODE_KP_1: return io::KeyboardKeys::kNum1;
      case SDL_SCANCODE_KP_2: return io::KeyboardKeys::kNum2;
      case SDL_SCANCODE_KP_3: return io::KeyboardKeys::kNum3;
      case SDL_SCANCODE_KP_4: return io::KeyboardKeys::kNum4;
      case SDL_SCANCODE_KP_5: return io::KeyboardKeys::kNum5;
      case SDL_SCANCODE_KP_6: return io::KeyboardKeys::kNum6;
      case SDL_SCANCODE_KP_7: return io::KeyboardKeys::kNum7;
      case SDL_SCANCODE_KP_8: return io::KeyboardKeys::kNum8;
      case SDL_SCANCODE_KP_9: return io::KeyboardKeys::kNum9;
      case SDL_SCANCODE_KP_0: return io::KeyboardKeys::kNum0;
      case SDL_SCANCODE_LCTRL: return io::KeyboardKeys::kLControl;
      case SDL_SCANCODE_LSHIFT: return io::KeyboardKeys::kLShift;
      case SDL_SCANCODE_LALT: return io::KeyboardKeys::kLAlt;
      case SDL_SCANCODE_LGUI: return io::KeyboardKeys::kLSuper;
      case SDL_SCANCODE_RCTRL: return io::KeyboardKeys::kRControl;
      case SDL_SCANCODE_RSHIFT: return io::KeyboardKeys::kRShift;
      case SDL_SCANCODE_RALT: return io::KeyboardKeys::kRAlt;
      case SDL_SCANCODE_RGUI: return io::KeyboardKeys::kRSuper;
      default: return (io::KeyboardKeys)key;
      }
    }

    SDL2Window::~SDL2Window()
    {
      if (window_ != nullptr)
      {
        GetWindows().erase(window_);
        close();
        SDL_Quit();
      }
    }
    void SDL2Window::create(const glm::uvec2& size, const char* title)
    {
      LMB_ASSERT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0 ? true : false, "SDL2: Failed to initialize GLFW");

#if VIOLET_OSX
      SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
#else
      SDL_SetHint(SDL_HINT_RENDER_DRIVER, "none");
#endif

      window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size.x, size.y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
      LMB_ASSERT(window_ != nullptr, "SDL2: Failed to create window!");
      if (window_ == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
      }

      pollMessage();

      focus_ = true;
      is_open_ = true;
      GetWindows().insert(eastl::make_pair(window_, this));
    }
    void* SDL2Window::getWindow() const
    {
      SDL_SysWMinfo wm_info;
      SDL_VERSION(&wm_info.version);
      SDL_GetWindowWMInfo(window_, &wm_info);
      
#if VIOLET_WIN32
      return wm_info.info.win.window;
#elif VIOLET_OSX
      return wm_info.info.cocoa.window;
#else
      return nullptr;
#endif
    }
    bool SDL2Window::pollMessage(platform::WindowMessage& message)
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
        return true;
      }

      return false;
    }
    void SDL2Window::sendMessage(const platform::WindowMessage& message)
    {
      messages_.push(message);
    }
    void SDL2Window::close()
    {
      if (is_open_ == true)
      {
        is_open_ = false;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
      }
    }
    bool SDL2Window::isOpen() const
    {
      return is_open_;
    }
    glm::uvec2 SDL2Window::getSize() const
    {
      glm::ivec2 size;
      SDL_GetWindowSize(window_,&size.x,&size.y);
      return (glm::uvec2)size;
    }
    float SDL2Window::getAspectRatio() const
    {
      return (float)getSize().x / (float)getSize().y;
    }
    float SDL2Window::getDPIMultiplier() const
    {
      // TODO (Hilze): Add this.
      return 1.0f;
    }
    void SDL2Window::setSize(const glm::uvec2& size)
    {
      SDL_SetWindowSize(window_, (int)size.x, (int)size.y);
    }
    bool SDL2Window::showCursor() const
    {
      return SDL_ShowCursor(-1) > 0 ? true : false;
    }
    void SDL2Window::setShowCursor(const bool& show_cursor)
    {
      SDL_ShowCursor((int)show_cursor);
    }
    void SDL2Window::setCursorPosition(const glm::ivec2& position)
    {
      SDL_WarpMouseInWindow(window_, position.x, position.y);
    }
    glm::ivec2 SDL2Window::getCursorPosition() const
    {
      glm::ivec2 pos;
      SDL_GetMouseState(&pos.x,&pos.y);
      return pos;
    }
    bool SDL2Window::inFocus() const
    {
      return focus_;
    }
    SDL2Window* SDL2Window::getInstance(SDL_Window* window)
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
    void SDL2Window::pollMessage()
    {
      SDL_Event evnt;
      while (SDL_PollEvent(&evnt) != 0)
      {
        platform::WindowMessage message;
        //User requests quit
        switch (evnt.type)
        {
        case SDL_QUIT:
          message.type = platform::WindowMessageType::kClose;
          break;
        case SDL_MOUSEMOTION:
          message.type = platform::WindowMessageType::kMouseMove;
          message.data[0] = evnt.motion.x;
          message.data[1] = evnt.motion.y;
          break;
        case SDL_TEXTINPUT:
          message.type = platform::WindowMessageType::kChar;
          message.data[0] = (uint32_t)sdl2ToLmbKey(evnt.key.state);
          break;
        case SDL_KEYDOWN:
          message.type = platform::WindowMessageType::kKeyboardButton;
          message.data[0] = (uint32_t)sdl2ToLmbKey(evnt.key.keysym.scancode);
          message.data[1] = 1u;
          break;
        case SDL_KEYUP:
          message.type = platform::WindowMessageType::kKeyboardButton;
          message.data[0] = (uint32_t)sdl2ToLmbKey(evnt.key.keysym.scancode);
          message.data[1] = 0u;
          break;
          message.type    = platform::WindowMessageType::kMouseScroll;
          message.data[0] = evnt.wheel.y;
          break;
        case SDL_MOUSEBUTTONDOWN:
          message.type    = platform::WindowMessageType::kMouseButton;
          message.data[0] = (uint32_t)sdl2ToLmbButton(evnt.button.button);
          message.data[1] = 1u;
          break;
        case SDL_MOUSEBUTTONUP:
          message.type = platform::WindowMessageType::kMouseButton;
          message.data[0] = (uint32_t)sdl2ToLmbButton(evnt.button.button);
          message.data[1] = 0u;
          break;
        case SDL_WINDOWEVENT:
          switch (evnt.window.event) {
          case SDL_WINDOWEVENT_SIZE_CHANGED:
            message.type = platform::WindowMessageType::kResize;
            message.data[0] = (uint32_t)evnt.window.data1;
            message.data[1] = (uint32_t)evnt.window.data2;
            break;
          case SDL_WINDOWEVENT_FOCUS_GAINED:
            focus_ = true;
            message.type = platform::WindowMessageType::kFocus;
            message.data[0] = 1u;
            break;
          case SDL_WINDOWEVENT_FOCUS_LOST:
            focus_ = false;
            message.type = platform::WindowMessageType::kFocus;
            message.data[0] = 0u;
            break;
          }
          break;
        }
        sendMessage(message);
      }
    }
    UnorderedMap<SDL_Window*, SDL2Window*>& SDL2Window::GetWindows()
    {
      static UnorderedMap<SDL_Window*, SDL2Window*> windows_;
      return windows_;
    }
  }
}
