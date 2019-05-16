#include "iworld.h"
#include "utils/profiler.h"

#define USE_MT_GC 0

#if USE_MT_GC
#if VIOLET_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef near
#undef far
#undef min
#undef max
#endif
#endif

namespace lambda
{
	namespace world
	{
		///////////////////////////////////////////////////////////////////////////
		IWorld::IWorld(
			platform::IWindow* window,
			platform::IRenderer* renderer,
			scripting::IScriptContext* scripting
		)
			: input_manager_(keyboard_, mouse_, controller_manager_)
			, scripting_(scripting)
		{
			scene_.scripting = scripting;
			scene_.renderer  = renderer;
			scene_.gui       = &gui_;
			scene_.post_process_manager = &post_process_manager_;
			scene_.fixed_time_step = 1.0 / 60.0;
			scene_.time_scale = 1.0;

			asset::TextureManager::setRenderer(scene_.renderer);
			asset::ShaderManager::setRenderer(scene_.renderer);
			asset::MeshManager::setRenderer(scene_.renderer);

			scene_.renderer->initialize(scene_);
			setWindow(window);
			gui_.init(scene_);

			scene::sceneInitialize(scene_);
			
			scripting_->setWorld(this);
		}

		///////////////////////////////////////////////////////////////////////////
		IWorld::~IWorld()
		{
		}

#if USE_MT_GC
		std::thread k_gc_thread;
		scripting::IScriptContext* k_gc_data = nullptr;
		bool k_gc_keep_alive = true;
		std::mutex k_gc_mutex;

		void gcCanRead()
		{
			bool can_get = false;
			do
			{
				k_gc_mutex.lock();
				can_get = k_gc_data != nullptr;
				k_gc_mutex.unlock();
				if (!can_get)
					std::this_thread::sleep_for(std::chrono::microseconds(1));
			} while (!can_get);
		}

		void gcCanWrite()
		{
			bool can_set = false;
			do
			{
				k_gc_mutex.lock();
				can_set = k_gc_data == nullptr;
				k_gc_mutex.unlock();
				if (!can_set)
					std::this_thread::sleep_for(std::chrono::microseconds(1));
			} while (!can_set);
		}

		void gcSet(scripting::IScriptContext* data)
		{
			k_gc_mutex.lock();
			k_gc_data = data;
			k_gc_mutex.unlock();
		}

		void gcLoop()
		{
			while (k_gc_keep_alive)
			{
				gcCanRead();

				k_gc_data->collectGarbage();
				
				k_gc_mutex.lock();
				k_gc_data = nullptr;
				k_gc_mutex.unlock();
			}
		}

#endif

		///////////////////////////////////////////////////////////////////////////
		void IWorld::run()
		{
#if USE_MT_GC
			k_gc_thread = std::thread(gcLoop);
#if VIOLET_WIN32
			SetThreadPriority(k_gc_thread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
			SetThreadPriorityBoost(k_gc_thread.native_handle(), TRUE);
			//SetThreadDescription(k_gc_thread.native_handle(), L"Construct Thread");
#endif
#endif

			initialize();
			scripting_->executeFunction("Game::Initialize", {});
			scene_.renderer->resize();

			scene_.debug_renderer.Initialize(scene_);
			
			double time_step_remainer = 0.0;
			
			while (scene_.window->isOpen())
			{
				handleWindowMessages();
				if (scene_.window->isOpen() == false)
					break;

				controller_manager_.update();

				delta_time_ = timer_.elapsed().seconds() * scene_.time_scale;
				timer_.reset();

				if (scene_.window->getSize().x == 0.0f || scene_.window->getSize().y == 0.0f)
					continue;

#if USE_MT_GC
				gcCanWrite();
#endif

				scripting_->executeFunction("Input::InputHelper::UpdateAxes", {});

				profiler_.startTimer("Total");
				profiler_.startTimer("FixedUpdate");
				static unsigned char max_step_count_count = 8u;
				unsigned char time_step_count = 0u;
				bool did_fixed_update = false;
				time_step_remainer += delta_time_;
				while (time_step_remainer >= scene_.fixed_time_step &&
					time_step_count++ < max_step_count_count)
				{
					did_fixed_update = true;
					fixedUpdate();
					time_step_remainer -= scene_.fixed_time_step;

					scripting_->executeFunction("Game::FixedUpdate", { scripting::ScriptValue((float)scene_.fixed_time_step) });

					scene::sceneFixedUpdate((float)scene_.fixed_time_step, scene_);

					scene::sceneCollectGarbage(scene_);
				}
				if (time_step_remainer >= scene_.fixed_time_step)
					time_step_remainer -= std::floor(time_step_remainer / scene_.fixed_time_step) * scene_.fixed_time_step;
				profiler_.endTimer("FixedUpdate");

				update(delta_time_);

				profiler_.startTimer("Update");
				gui_.update(delta_time_);
				scripting_->executeFunction("Game::Update", { scripting::ScriptValue((float)delta_time_) });
				scene::sceneUpdate((float)delta_time_, scene_);
				scene_.renderer->update(delta_time_);
				profiler_.endTimer("Update");

				profiler_.startTimer("CollectGarbage");
#if USE_MT_GC
				if (did_fixed_update)
					gcSet(scene_.scripting);
#else
				if (did_fixed_update)
					scene_.scripting->collectGarbage();
#endif
				scene::sceneCollectGarbage(scene_);
				profiler_.endTimer("CollectGarbage");

				profiler_.startTimer("ConstructRender");
				scene::sceneConstructRender(scene_);
				profiler_.endTimer("ConstructRender");
				
				profiler_.startTimer("OnRender");
				scene::sceneOnRender(scene_);
				profiler_.endTimer("OnRender");
				profiler_.endTimer("Total");
			}

			scripting_->executeFunction("Game::Deinitialize", {});
			deinitialize();
			scene_.debug_renderer.Deinitialize();
			scene_.scripting->terminate();
			scene::sceneDeinitialize(scene_);
			memset(&scene_, 0, sizeof(scene_));
		}

		///////////////////////////////////////////////////////////////////////////
		void IWorld::handleWindowMessages()
		{
			io::Mouse::State mouse_state = mouse_.getCurrentState();
			io::Keyboard::State keyboard_state = keyboard_.getCurrentState();
			mouse_state.setAxis(io::MouseAxes::kScroll, 0.0f);
			mouse_state.setButton(io::MouseButtons::kLMBD, false);
			mouse_state.setButton(io::MouseButtons::kMMBD, false);
			mouse_state.setButton(io::MouseButtons::kRMBD, false);

			platform::WindowMessage message;
			while (scene_.window->pollMessage(message))
			{
				bool was_handled = false;
				if (!was_handled && gui_.handleWindowMessage(message))
					was_handled = true;

				switch (message.type)
				{
				case platform::WindowMessageType::kResize:
					scene_.renderer->resize();
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
		}

		///////////////////////////////////////////////////////////////////////////
		double IWorld::getDeltaTime() const
		{
			return delta_time_;
		}

		///////////////////////////////////////////////////////////////////////////
		scene::Scene& IWorld::getScene()
		{
			return scene_;
		}

		///////////////////////////////////////////////////////////////////////////
		scripting::IScriptContext* IWorld::getScripting()
		{
			return scripting_;
		}

		///////////////////////////////////////////////////////////////////////////
		void IWorld::setWindow(platform::IWindow* window)
		{
			if (scene_.window)
				scene_.window->close();
			scene_.window = window;
			scene_.renderer->setWindow(scene_.window);
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
		gui::GUI& IWorld::getGUI()
		{
			return gui_;
		}

		utilities::Profiler& IWorld::getProfiler()
		{
			return profiler_;
		}
	}
}