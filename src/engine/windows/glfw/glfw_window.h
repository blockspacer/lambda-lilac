#pragma once
#include "interfaces/iwindow.h"
#include <containers/containers.h>

struct GLFWwindow;

namespace lambda
{
  namespace window
  {
    class GLFWWindow : public platform::IWindow
    {
    public:
      ~GLFWWindow();
      virtual String name() const override { return "glfw"; };
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

      static GLFWWindow* getInstance(GLFWwindow* window);
      static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int modifiers);
      static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
      static void glfwCursorPosCallback(GLFWwindow* window, double x, double y);
      static void glfwCharCallback(GLFWwindow* window, unsigned int ch);
      static void glfwCharCallbackMod(GLFWwindow* window, unsigned int ch, int modifiers);
      static void glfwCloseCallback(GLFWwindow* window);
      static void glfwSizeCallback(GLFWwindow* window, int width, int height);

    private:
      void pollMessage();

      static UnorderedMap<GLFWwindow*, GLFWWindow*>& GetWindows();
    private:
      glm::uvec2 size_;
      GLFWwindow* window_ = nullptr;
      bool is_open_ = false;
      Queue<platform::WindowMessage> messages_;
    };
  }
}