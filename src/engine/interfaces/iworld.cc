#include "iworld.h"
#include "utils/profiler.h"

namespace lambda
{
  namespace world
  {
    ///////////////////////////////////////////////////////////////////////////
    IWorld::IWorld(
      foundation::SharedPointer<platform::IWindow> window,
      foundation::SharedPointer<platform::IRenderer> renderer,
      foundation::SharedPointer<asset::AssetManager> asset_manager,
      foundation::SharedPointer<scripting::IScriptContext> scripting,
      foundation::SharedPointer<platform::IImGUI> imgui
    )
      : window_(nullptr)
      , renderer_(renderer)
      , asset_manager_(asset_manager)
      , input_manager_(keyboard_, mouse_, controller_manager_)
      , scripting_(scripting)
      , imgui_(nullptr)
    {
			shader_variable_manager_.setVariable(platform::ShaderVariable(Name("dynamic_resolution_scale"), 1.0f));

			renderer_->initialize(this);
			setWindow(window);
			setImGUI(imgui);
			gui_.init(this);

			scene_.initialize(this);
			scripting_->setWorld(this);
    }

    ///////////////////////////////////////////////////////////////////////////
    IWorld::~IWorld()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void IWorld::run()
    {
      initialize();
      scripting_->executeFunction("Game::Initialize", {});
      renderer_->resize();

      const double time_step = 1.0 / 60.0;
      double time_step_remainer = 0.0;

      while (window_->isOpen())
      {
        handleWindowMessages();
        if (window_->isOpen() == false)
          return;

        controller_manager_.update();

        delta_time_ = timer_.elapsed().seconds();
        timer_.reset();

        if (window_->getSize().x == 0.0f || window_->getSize().y == 0.0f)
          continue;

        scripting_->executeFunction("Input::InputHelper::UpdateAxes", {});

				static unsigned char max_step_count_count = 8u;
        unsigned char time_step_count = 0u;
        time_step_remainer += delta_time_;
        while (time_step_remainer >= time_step && 
          time_step_count++ < max_step_count_count)
        {
					fixedUpdate();
					utilities::Profiler::getInstance().startTimer("Scripting: FixedUpdate");
					time_step_remainer -= time_step;
          scripting_->executeFunction(
            "Game::FixedUpdate", 
            { 
              scripting::ScriptValue((float)time_step) 
            }
          );
					utilities::Profiler::getInstance().endTimer("Scripting: FixedUpdate");

					utilities::Profiler::getInstance().startTimer("Systems: FixedUpdate");
					for (auto& system : getScene().getAllSystems())
            system->fixedUpdate(time_step);
					utilities::Profiler::getInstance().endTimer("Systems: FixedUpdate");

					utilities::Profiler::getInstance().startTimer("Systems: FixedCollectGarbage");
					for (auto& system : getScene().getAllSystems())
						system->collectGarbage();
					utilities::Profiler::getInstance().endTimer("Systems: FixedCollectGarbage");
        }
        if (time_step_remainer >= time_step)
        {
          time_step_remainer -= 
            std::floor(time_step_remainer / time_step) * time_step;
        }

				update(delta_time_);

				utilities::Profiler::getInstance().startTimer("GUI: Update");
				gui_.update(delta_time_);
				utilities::Profiler::getInstance().endTimer("GUI: Update");

				utilities::Profiler::getInstance().startTimer("Scripting: Update");
				scripting_->executeFunction(
          "Game::Update", 
          { 
            scripting::ScriptValue((float)delta_time_) 
          }
        );
				utilities::Profiler::getInstance().endTimer("Scripting: Update");

				utilities::Profiler::getInstance().startTimer("Systems: Update");
				for (auto& system : getScene().getAllSystems())
          system->update(delta_time_);
				utilities::Profiler::getInstance().endTimer("Systems: Update");

				utilities::Profiler::getInstance().startTimer("ImGUI: Update");
				imgui_->update(delta_time_);
				utilities::Profiler::getInstance().endTimer("ImGUI: Update");
        
				utilities::Profiler::getInstance().startTimer("Renderer: Update");
				renderer_->update(delta_time_);
				utilities::Profiler::getInstance().endTimer("Renderer: Update");

				utilities::Profiler::getInstance().startTimer("Scripting: CollectGarbage");
				scripting_->collectGarbage();
				utilities::Profiler::getInstance().endTimer("Scripting: CollectGarbage");

				utilities::Profiler::getInstance().startTimer("Systems: CollectGarbage");
				for (auto& system : getScene().getAllSystems())
					system->collectGarbage();
				utilities::Profiler::getInstance().endTimer("Systems: CollectGarbage");

				utilities::Profiler::getInstance().startTimer("Renderer: StartFrame");
				renderer_->startFrame();
				utilities::Profiler::getInstance().endTimer("Renderer: StartFrame");

				utilities::Profiler::getInstance().startTimer("Systems: OnRender");
				for (auto& system : getScene().getAllSystems())
          system->onRender();
				utilities::Profiler::getInstance().endTimer("Systems: OnRender");

				utilities::Profiler::getInstance().startTimer("ImGUI: GenerateCommandList");
				imgui_->generateCommandList();
				utilities::Profiler::getInstance().endTimer("ImGUI: GenerateCommandList");
        
				utilities::Profiler::getInstance().startTimer("Renderer: EndFrame");
				renderer_->endFrame();
				utilities::Profiler::getInstance().endTimer("Renderer: EndFrame");
      }

      scripting_->executeFunction("Game::Terminate", {});
      deinitialize();
    }

    ///////////////////////////////////////////////////////////////////////////
    void IWorld::handleWindowMessages()
    {
      imgui_->inputStart();

      io::Mouse::State mouse_state = mouse_.getCurrentState();
      io::Keyboard::State keyboard_state = keyboard_.getCurrentState();
      mouse_state.setAxis(io::MouseAxes::kScroll, 0.0f);
      mouse_state.setButton(io::MouseButtons::kLMBD, false);
      mouse_state.setButton(io::MouseButtons::kMMBD, false);
      mouse_state.setButton(io::MouseButtons::kRMBD, false);

      platform::WindowMessage message;
      while (getWindow()->pollMessage(message))
      {
        bool was_handled = false;
				if (!was_handled && gui_.handleWindowMessage(message))
					was_handled = true;
        if (!was_handled && imgui_->inputHandleMessage(message))
          was_handled = true;

				switch (message.type)
        {
        case platform::WindowMessageType::kResize:
          getRenderer()->resize();
          break;
        case platform::WindowMessageType::kMouseMove:
          mouse_state.setAxis(io::MouseAxes::kMouseX, (float)message.data[0]);
          mouse_state.setAxis(io::MouseAxes::kMouseY, (float)message.data[1]);
          break;
        case platform::WindowMessageType::kMouseScroll:
          mouse_state.setAxis(
            io::MouseAxes::kScroll, 
            (float)(short)message.data[0] / 120.0f
          );
          break;
        case platform::WindowMessageType::kMouseButton:
          mouse_state.setButton(
            (io::MouseButtons)message.data[0], 
            message.data[1] > 0 && !was_handled ? true : false
          );
          break;
        case platform::WindowMessageType::kKeyboardButton:
          keyboard_state.setKey(
            (io::KeyboardKeys)message.data[0], 
            message.data[1] > 0 && !was_handled ? true : false
          );
        default:
          break;
        }

        if (!was_handled)
          handleWindowMessage(message);
      }

      mouse_.update(mouse_state);
      keyboard_.update(keyboard_state);

      imgui_->inputEnd();
    }

    ///////////////////////////////////////////////////////////////////////////
    double IWorld::getDeltaTime() const
    {
      return delta_time_;
    }

    ///////////////////////////////////////////////////////////////////////////
    Scene& IWorld::getScene()
    {
      return scene_;
    }

    ///////////////////////////////////////////////////////////////////////////
    foundation::SharedPointer<platform::IRenderer> IWorld::getRenderer()
    {
      return renderer_;
    }

    ///////////////////////////////////////////////////////////////////////////
    foundation::SharedPointer<platform::IWindow> IWorld::getWindow()
    {
      return window_;
    }

    ///////////////////////////////////////////////////////////////////////////
    foundation::SharedPointer<asset::AssetManager> IWorld::getAssetManager()
    {
      return asset_manager_;
    }

    ///////////////////////////////////////////////////////////////////////////
    foundation::SharedPointer<scripting::IScriptContext> IWorld::getScripting()
    {
      return scripting_;
    }

    ///////////////////////////////////////////////////////////////////////////
    foundation::SharedPointer<platform::IImGUI> IWorld::getImGUI()
    {
      return imgui_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void IWorld::setImGUI(foundation::SharedPointer<platform::IImGUI> imgui)
    {
			if (imgui_)
				imgui_->deinitialize();
      imgui_ = imgui;
      imgui_->initialize(this);
    }

    ///////////////////////////////////////////////////////////////////////////
    void IWorld::setWindow(foundation::SharedPointer<platform::IWindow> window)
    {
			if (window_)
				window_->close();
      window_ = window;
      renderer_->setWindow(window_);
    }

    ///////////////////////////////////////////////////////////////////////////
    const io::Input<io::Mouse::State>& IWorld::getMouse()
    {
      return mouse_;
    }

    ///////////////////////////////////////////////////////////////////////////
    const io::Input<io::Keyboard::State>& IWorld::getKeyboard()
    {
      return keyboard_;
    }

    ///////////////////////////////////////////////////////////////////////////
    const io::ControllerManager& IWorld::getControllerManager()
    {
      return controller_manager_;
    }

    ///////////////////////////////////////////////////////////////////////////
    io::InputManager& IWorld::getInputManager()
    {
      return input_manager_;
    }

    ///////////////////////////////////////////////////////////////////////////
    platform::ShaderVariableManager& IWorld::getShaderVariableManager()
    {
      return shader_variable_manager_;
    }

    ///////////////////////////////////////////////////////////////////////////
    platform::DebugRenderer& IWorld::getDebugRenderer()
    {
      return debug_renderer_;
    }

    ///////////////////////////////////////////////////////////////////////////
    platform::PostProcessManager& IWorld::getPostProcessManager()
    {
      return post_process_manager_;
    }

		///////////////////////////////////////////////////////////////////////////
		gui::GUI& IWorld::getGUI()
		{
			return gui_;
		}
  }
}
