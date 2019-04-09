#pragma once
#include "gui_value.h"
#include <assets/texture.h>
#include <interfaces/iwindow.h>

#include <containers/containers.h>

#include <Ultralight/Renderer.h>
#include <Ultralight/View.h>
#include <JavaScriptCore/JavaScript.h>

#include <functional>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace lambda
{
	namespace scene
	{
		struct Scene;
	}

	namespace gui
	{
		typedef std::function<void(const Vector<JSVal>&, const void*)> JavaScriptCallback;
		typedef std::function<JSVal(const Vector<JSVal>&, const void*)> JavaScriptCallbackWithRetval;

		struct JSCW;
		struct JSCWRW;

		class GUI
		{
		public:
			GUI();
			~GUI();

			void init(scene::Scene& scene);

			void loadURL(String url);

			void update(double delta_time);

			void render(scene::Scene& scene);

			bool handleWindowMessage(const platform::WindowMessage& message);

			void executeJavaScript(String js);

			void bindJavaScriptCallback(String name, JavaScriptCallback callback, const void* user_data = nullptr);
			void bindJavaScriptCallback(String name, JavaScriptCallbackWithRetval callback, const void* user_data = nullptr);

			void setEnabled(bool enabled);
			bool getEnabled() const;

		private:
			bool enabled_;
			double switch_time_;
			double switch_;
			glm::ivec2 mouse_position_;
			asset::VioletTextureHandle texture_;
			scene::Scene* scene_;
			ultralight::RefPtr<ultralight::Renderer> renderer_;
			ultralight::RefPtr<ultralight::View> view_;
			Vector<JSCW*>   jscw_;
			Vector<JSCWRW*> jscwrw_;
		};
	}
}

#define BindJavaScriptCallback(fn) (lambda::gui::JavaScriptCallback)std::bind(fn, this, std::placeholders::_1, std::placeholders::_2)
#define BindJavaScriptCallbackWithRetval(fn) (lambda::gui::JavaScriptCallbackWithRetval)std::bind(fn, this, std::placeholders::_1, std::placeholders::_2)
