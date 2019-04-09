#pragma once
#include "platform/scene.h"
#include <memory/memory.h>
#include "iwindow.h"
#include "irenderer.h"
#include "utils/timer.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/controller.h"
#include "input/input_manager.h"
#include "interfaces/iscript_context.h"
#include "iimgui.h"
#include "platform/shader_variable_manager.h"
#include "platform/debug_renderer.h"
#include "platform/post_process_manager.h"
#include "utils/profiler.h"
#include "gui/gui.h"

namespace lambda
{
  namespace world
  {
    ///////////////////////////////////////////////////////////////////////////
    class IWorld
    {
    public:
      IWorld(
        platform::IWindow* window, 
        platform::IRenderer* renderer,
        scripting::IScriptContext* scripting,
        foundation::SharedPointer<platform::IImGUI> imgui
      );
      ~IWorld();

      void run();

    protected:
      virtual void initialize() = 0;
      virtual void deinitialize() = 0;
      virtual void update(const double& delta_time) = 0;
      virtual void fixedUpdate() = 0;
      virtual void handleWindowMessage(
        const platform::WindowMessage& message
      ) = 0;

    private:
      void handleWindowMessages();

    public:
      double getDeltaTime() const;
      scene::Scene& getScene();
      scripting::IScriptContext* getScripting();
      foundation::SharedPointer<platform::IImGUI> getImGUI();
      void setImGUI(foundation::SharedPointer<platform::IImGUI> imgui);
      void setWindow(platform::IWindow* window);
      const io::Input<io::Mouse::State>& getMouse();
      const io::Input<io::Keyboard::State>& getKeyboard();
      const io::ControllerManager& getControllerManager();
      io::InputManager& getInputManager();
			gui::GUI& getGUI();
			utilities::Profiler& getProfiler();

    private:
			double delta_time_;
      scene::Scene scene_;
      utilities::Timer timer_;
      io::Input<io::Mouse::State> mouse_;
      io::Input<io::Keyboard::State> keyboard_;
      io::ControllerManager controller_manager_;
      io::InputManager input_manager_;
      scripting::IScriptContext* scripting_;
      foundation::SharedPointer<platform::IImGUI> imgui_;
			gui::GUI gui_;
			utilities::Profiler profiler_;
    };
  }
}