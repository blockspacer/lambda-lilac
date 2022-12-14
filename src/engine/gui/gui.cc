#include <gui/gui.h>

#include <interfaces/irenderer.h>
#include <platform/scene.h>

#if VIOLET_GUI_ULTRALIGHT
#include <gui/gpu_driver.h>
#include <gui/file_system.h>
#include <gui/font_handler.h>

#include <Ultralight/platform/Platform.h>
#include <Ultralight/platform/Config.h>
#include <Ultralight/Ultralight.h>
#include <JavaScriptCore/JavaScript.h>
#endif

namespace lambda
{
	namespace gui
	{
		std::mutex k_mutex;

#if VIOLET_GUI_ULTRALIGHT
		///////////////////////////////////////////////////////////////////////////
		String exceptionToString(JSValueRef exception, JSContextRef js_context)
		{
			JSStringRef js_string = JSValueToStringCopy(js_context, exception, nullptr);
			String str(JSStringGetLength(js_string) + 1u, '\0');
			JSStringGetUTF8CString(js_string, (char*)str.c_str(), str.length());
			return str;
		}

		///////////////////////////////////////////////////////////////////////////
		void handleException(JSValueRef exception, JSContextRef js_context)
		{
			JSType type = JSValueGetType(js_context, exception);

			if (type != kJSTypeUndefined && type != kJSTypeNull)
				LMB_ASSERT(false, exceptionToString(exception, js_context).c_str());
		}


		///////////////////////////////////////////////////////////////////////////
		///// lOAD LISTENER IMPL //////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////
		class LoadListenerImpl : public ultralight::LoadListener
		{
		public:
			/////////////////////////////////////////////////////////////////////////
			LoadListenerImpl()
				: finished_(false)
			{}

			/////////////////////////////////////////////////////////////////////////
			virtual ~LoadListenerImpl() override
			{}

			/////////////////////////////////////////////////////////////////////////
			virtual void OnFinishLoading(ultralight::View* caller) override
			{
				finished_ = true;
			}

			/////////////////////////////////////////////////////////////////////////
			bool isFinished() const
			{
				return finished_;
			}

		private:
			bool finished_;
		};





		///////////////////////////////////////////////////////////////////////////
		///// VIEW LISTENER IMPL //////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////
		class ViewListenerImpl : public ultralight::ViewListener
		{
		public:
			/////////////////////////////////////////////////////////////////////////
			virtual ~ViewListenerImpl() override
			{}

			/////////////////////////////////////////////////////////////////////////
			virtual void OnAddConsoleMessage(
				ultralight::View* caller,
				ultralight::MessageSource source,
				ultralight::MessageLevel level,
				const ultralight::String& message,
				uint32_t line_number,
				uint32_t column_number,
				const ultralight::String& source_id) override
			{
				String msg = " <" + String(source_id.utf8().data()) + ", ";

				switch (source)
				{
				case ultralight::kMessageSource_XML:            msg += "XML"; break;
				case ultralight::kMessageSource_JS:             msg += "JS"; break;
				case ultralight::kMessageSource_Network:        msg += "NETWORK"; break;
				case ultralight::kMessageSource_ConsoleAPI:     msg += "CONSOLE"; break;
				case ultralight::kMessageSource_Storage:        msg += "STORAGE"; break;
				case ultralight::kMessageSource_AppCache:       msg += "APP CACHE"; break;
				case ultralight::kMessageSource_Rendering:      msg += "RENDERING"; break;
				case ultralight::kMessageSource_CSS:            msg += "CSS"; break;
				case ultralight::kMessageSource_Security:       msg += "SECURITY"; break;
				case ultralight::kMessageSource_ContentBlocker: msg += "CONTENT BLOCKER"; break;
				case ultralight::kMessageSource_Other:          msg += "OTHER"; break;
				}

				msg += ", " + toString(line_number) + ", " + toString(column_number) + "> " + message.utf8().data();

				switch (level)
				{
				case ultralight::kMessageLevel_Log:     foundation::InfoNP("[LOG]" + msg); break;
				case ultralight::kMessageLevel_Warning: foundation::Warning(msg); break;
				case ultralight::kMessageLevel_Error:   foundation::Error(msg); break;
				case ultralight::kMessageLevel_Debug:   foundation::Debug(msg); break;
				case ultralight::kMessageLevel_Info:    foundation::Info(msg); break;
				}
			}
		};
#endif



		///////////////////////////////////////////////////////////////////////////
		GUI::GUI()
			: switch_time_(1.0 / 60.0)
			, switch_(0.0)
			, scene_(nullptr)
			, enabled_(true)
#if VIOLET_GUI_ULTRALIGHT
			, renderer_(nullptr)
			, view_(nullptr)
#endif
		{
		}

		///////////////////////////////////////////////////////////////////////////
		GUI::~GUI()
		{
#if VIOLET_GUI_ULTRALIGHT
			ultralight::Platform& platform = ultralight::Platform::instance();

			if (platform.gpu_driver())
			{
				foundation::Memory::destruct(platform.gpu_driver());
				platform.set_gpu_driver(nullptr);
			}
			if (platform.file_system())
			{
				foundation::Memory::destruct(platform.file_system());
				platform.set_file_system(nullptr);
			}
			if (platform.font_loader())
			{
				foundation::Memory::destruct(platform.font_loader());
				platform.set_font_loader(nullptr);
			}

			foundation::Memory::destruct(view_->view_listener());
#endif

			for (auto it : jscw_)
				foundation::Memory::destruct(it);
			for (auto it : jscwrw_)
				foundation::Memory::destruct(it);
		}

		///////////////////////////////////////////////////////////////////////////
		void GUI::init(scene::Scene& scene)
		{
			scene_ = &scene;

#if VIOLET_GUI_ULTRALIGHT
			ultralight::Platform& platform = ultralight::Platform::instance();

			ultralight::Config config;
			config.face_winding = ultralight::kFaceWinding_Clockwise;
			config.device_scale_hint = 1.0;
			config.font_family_standard = "Arial";

			{
				MyGPUDriver* gpu_driver =
					foundation::Memory::construct<MyGPUDriver>(scene);
				MyFileSystem* file_system =
					foundation::Memory::construct<MyFileSystem>();
				MyFontLoader* font_loader =
					foundation::Memory::construct<MyFontLoader>();

				platform.set_config(config);
				platform.set_gpu_driver(gpu_driver);
				platform.set_file_system(file_system);
				platform.set_font_loader(font_loader);
			}

			renderer_ = ultralight::Renderer::Create();
			view_     = renderer_->CreateView(1280u, 720u, true);
			ViewListenerImpl* view_listener =
				foundation::Memory::construct<ViewListenerImpl>();
			view_->set_view_listener(view_listener);
#endif

			texture_ = asset::TextureManager::getInstance()->create(Name("__gui"));

			VioletTexture texture;
			texture.flags = kTextureFlagIsRenderTarget;
			texture.format = TextureFormat::kB8G8R8A8;
			texture.width = 1280u;
			texture.height = 720u;
			texture.mip_count = 1u;
			texture_->addLayer(asset::TextureLayer(texture));

			scene_->post_process_manager->addTarget(platform::RenderTarget(Name("gui"), texture_));
		}

		///////////////////////////////////////////////////////////////////////////
		void GUI::loadURL(String url)
		{
			k_mutex.lock();

			if (!enabled_)
			{
				k_mutex.unlock();
				return;
			}

#if VIOLET_GUI_ULTRALIGHT
			LoadListenerImpl* load_listener =
				foundation::Memory::construct<LoadListenerImpl>();
			view_->set_load_listener(load_listener);

			view_->LoadURL(url.c_str());

			while (!load_listener->isFinished())
				renderer_->Update();

			view_->set_load_listener(nullptr);
			foundation::Memory::destruct(load_listener);
#endif
			k_mutex.unlock();
		}

		///////////////////////////////////////////////////////////////////////////
		void GUI::update(double delta_time)
		{
			k_mutex.lock();

			if (!enabled_)
			{
				k_mutex.unlock();
				return;
			}

			switch_ += delta_time;
			if (switch_ < switch_time_)
			{
				k_mutex.unlock();
				return;
			}
			switch_ -= switch_time_;

#if VIOLET_GUI_ULTRALIGHT
			renderer_->Update();
			JSGarbageCollect(view_->js_context());
#endif
			k_mutex.unlock();
		}

		///////////////////////////////////////////////////////////////////////////
		void GUI::render(scene::Scene& scene)
		{
			k_mutex.lock();

#if VIOLET_GUI_ULTRALIGHT
			gui::MyGPUDriver* driver = (gui::MyGPUDriver*)ultralight::Platform::instance().gpu_driver();
			driver->setScene(scene);

			if (driver)
			{
				driver->BeginSynchronize();

				// Render all active views to command lists 
				// and dispatch calls to GPUDriver
				renderer_->Render();

				driver->EndSynchronize();

				// Draw any pending commands to screen
				if (driver->HasCommandsPending())
				{
					driver->DrawCommandList();

					scene.renderer->copyToTexture(driver->GetRenderBuffer(view_->render_target().render_buffer_id), texture_);
				}
			}
#endif

			k_mutex.unlock();
		}

		///////////////////////////////////////////////////////////////////////////
		bool GUI::handleWindowMessage(const platform::WindowMessage& message)
		{
			k_mutex.lock();
			
			if (!enabled_)
			{
				k_mutex.unlock();
				return false;
			}

#if VIOLET_GUI_ULTRALIGHT
			switch (message.type)
			{
			case platform::WindowMessageType::kMouseMove:
			{
				mouse_position_ =
					glm::ivec2((int)message.data[0], (int)message.data[1]);
				ultralight::MouseEvent mouse_event;
				mouse_event.type = ultralight::MouseEvent::kType_MouseMoved;
				mouse_event.x = mouse_position_.x;
				mouse_event.y = mouse_position_.y;
				mouse_event.button = ultralight::MouseEvent::kButton_None;
				view_->FireMouseEvent(mouse_event);
				break;
			}
			case platform::WindowMessageType::kMouseButton:
			{
				ultralight::MouseEvent mouse_event;
				bool up = message.data[1] > 0;
				ultralight::MouseEvent::Button button;
				if (message.data[0] == 0)
					button = ultralight::MouseEvent::kButton_Left;
				if (message.data[0] == 1)
					button = ultralight::MouseEvent::kButton_Middle;
				if (message.data[0] == 2)
					button = ultralight::MouseEvent::kButton_Right;
				mouse_event.type =
					up ? ultralight::MouseEvent::kType_MouseDown :
					ultralight::MouseEvent::kType_MouseUp;
				mouse_event.x = mouse_position_.x;
				mouse_event.y = mouse_position_.y;
				mouse_event.button = button;
				view_->FireMouseEvent(mouse_event);
				break;
			}
			case platform::WindowMessageType::kResize:
			{
				view_->Resize(message.data[0], message.data[1]);
				texture_->getLayer(0).resize(message.data[0], message.data[1]);
				break;
			}
			}
#endif

			k_mutex.unlock();
			return false;
		}

		///////////////////////////////////////////////////////////////////////////
		void GUI::executeJavaScript(String js)
		{
			k_mutex.lock();
			if (!enabled_)
			{
				k_mutex.unlock();
				return;
			}

#if VIOLET_GUI_ULTRALIGHT
			JSContextRef js_context = view_->js_context();
			JSStringRef str = JSStringCreateWithUTF8CString(js.c_str());

			JSValueRef exception = JSValueMakeNull(js_context);
			JSEvaluateScript(js_context, str, 0, 0, 0, &exception);
			handleException(exception, js_context);
			JSStringRelease(str);
#endif

			k_mutex.unlock();
		}

		///////////////////////////////////////////////////////////////////////////
		struct JSCW
		{
			JavaScriptCallback callback;
			void* user_data;
		};

		///////////////////////////////////////////////////////////////////////////
		struct JSCWRW
		{
			JavaScriptCallbackWithRetval callback;
			void* user_data;
		};

#if VIOLET_GUI_ULTRALIGHT
		///////////////////////////////////////////////////////////////////////////
		JSValueRef NativeFunctionCallback(
			JSContextRef ctx, 
			JSObjectRef function, 
			JSObjectRef thisObject,
			size_t argumentCount, 
			const JSValueRef arguments[],
			JSValueRef* exception) 
		{
			JSCW* callback = static_cast<JSCW*>(JSObjectGetPrivate(function));
			if (!callback)
				return JSValueMakeNull(ctx);

			Vector<JSVal> args;
			for (size_t i = 0; i < argumentCount; ++i)
			{
				JSVal val;
				if (JSValueIsBoolean(ctx, arguments[i]))
					val = JSVal(JSValueToBoolean(ctx, arguments[i]));
				else if (JSValueIsNumber(ctx, arguments[i]))
					val = JSVal((MarbleNumber)JSValueToNumber(ctx, arguments[i], nullptr));
				else if (JSValueIsString(ctx, arguments[i]))
					val = JSVal(exceptionToString(arguments[i], ctx).c_str());
				args.push_back(val);
			}

			callback->callback(args, callback->user_data);

			return JSValueMakeNull(ctx);
		}

		///////////////////////////////////////////////////////////////////////////
		void NativeFunctionFinalize(JSObjectRef function) {
			foundation::Memory::destruct(JSObjectGetPrivate(function));
		}

		///////////////////////////////////////////////////////////////////////////
		JSClassRef NativeFunctionClass() 
		{
			static JSClassRef instance = nullptr;
			if (!instance) 
			{
				JSClassDefinition def;
				memset(&def, 0, sizeof(def));
				def.className      = "NativeFunction";
				def.attributes     = kJSClassAttributeNone;
				def.callAsFunction = NativeFunctionCallback;
				def.finalize       = NativeFunctionFinalize;
				instance = JSClassCreate(&def);
			}
			return instance;
		}


		///////////////////////////////////////////////////////////////////////////
		JSValueRef NativeFunctionWithRetvalCallback(
			JSContextRef ctx, 
			JSObjectRef function, 
			JSObjectRef thisObject,
			size_t argumentCount, 
			const JSValueRef arguments[], 
			JSValueRef* exception) 
		{
			JSCWRW* callback = static_cast<JSCWRW*>(JSObjectGetPrivate(function));

			if (!callback)
				return JSValueMakeNull(ctx);

			Vector<JSVal> args;
			for (size_t i = 0; i < argumentCount; ++i)
			{
				JSVal val;
				if (JSValueIsBoolean(ctx, arguments[i]))
					val = JSVal(JSValueToBoolean(ctx, arguments[i]));
				else if (JSValueIsNumber(ctx, arguments[i]))
					val = JSVal((MarbleNumber)JSValueToNumber(ctx, arguments[i], nullptr));
				else if (JSValueIsString(ctx, arguments[i]))
					val = JSVal(exceptionToString(arguments[i], ctx).c_str());
				args.push_back(val);
			}

			JSVal res = callback->callback(args, callback->user_data);

			if (res.isString())
				return JSValueMakeString(ctx, JSStringCreateWithUTF8CString(res.asString()));
			else if (res.isBool())
				return JSValueMakeBoolean(ctx, res.asBool());
			else if (res.isNumber())
				return JSValueMakeNumber(ctx, (double)res.asNumber());
			else
				return JSValueMakeNull(ctx);
		}

		///////////////////////////////////////////////////////////////////////////
		void NativeFunctionWithRetvalFinalize(JSObjectRef function) 
		{
			foundation::Memory::destruct(JSObjectGetPrivate(function));
		}

		///////////////////////////////////////////////////////////////////////////
		JSClassRef NativeFunctionWithRetvalClass() 
		{
			static JSClassRef instance = nullptr;
			if (!instance)
			{
				JSClassDefinition def;
				memset(&def, 0, sizeof(def));
				def.className      = "NativeFunctionWithRetval";
				def.attributes     = kJSClassAttributeNone;
				def.callAsFunction = NativeFunctionWithRetvalCallback;
				def.finalize       = NativeFunctionWithRetvalFinalize;
				instance = JSClassCreate(&def);
			}
			return instance;
		}
#endif

		///////////////////////////////////////////////////////////////////////////
		void GUI::bindJavaScriptCallback(String name, JavaScriptCallback callback, const void* user_data)
		{
			k_mutex.lock();
#if VIOLET_GUI_ULTRALIGHT
			JSContextRef js_context = view_->js_context();
			JSStringRef str = JSStringCreateWithUTF8CString(name.c_str());

			JSObjectRef global_object = JSContextGetGlobalObject(js_context);

			JSCW* ud = foundation::Memory::construct<JSCW>();
			ud->callback  = callback;
			ud->user_data = (void*)user_data;

			jscw_.push_back(ud);

			JSObjectRef native_function = JSObjectMake(
				js_context,
				NativeFunctionClass(),
				ud
			);

			JSValueRef exception = JSValueMakeNull(js_context);

			JSObjectSetProperty(
				js_context,
				global_object,
				str,
				native_function,
				kJSPropertyAttributeNone,
				&exception
			);

			handleException(exception, js_context);
#endif
			k_mutex.unlock();
		}

		///////////////////////////////////////////////////////////////////////////
		void GUI::bindJavaScriptCallback(String name, JavaScriptCallbackWithRetval callback, const void* user_data)
		{
			k_mutex.lock();
#if VIOLET_GUI_ULTRALIGHT
			JSContextRef js_context = view_->js_context();
			JSStringRef str = JSStringCreateWithUTF8CString(name.c_str());

			JSObjectRef global_object = JSContextGetGlobalObject(js_context);

			JSCWRW* ud = foundation::Memory::construct<JSCWRW>();
			ud->callback = callback;
			ud->user_data = (void*)user_data;

			jscwrw_.push_back(ud);

			JSObjectRef native_function = JSObjectMake(
				js_context, 
				NativeFunctionWithRetvalClass(),
				ud
			);

			JSValueRef exception = JSValueMakeNull(js_context);

			JSObjectSetProperty(
				js_context,
				global_object,
				str,
				native_function,
				kJSPropertyAttributeNone,
				&exception
			);

			handleException(exception, js_context);
#endif
			k_mutex.unlock();
		}
		void GUI::setEnabled(bool enabled)
		{
			k_mutex.lock();
			enabled_ = enabled;
			k_mutex.unlock();
		}
		bool GUI::getEnabled() const
		{
			k_mutex.lock();
			bool enabled = enabled_;
			k_mutex.unlock();
			return enabled;
		}
	}
}