#include <gui/gui.h>
#include <containers/containers.h>
#include <utils/console.h>
#include <memory/memory.h>
#include <utils/timer.h>

#include <cef_browser.h>
#include <cef_command_line.h>
#include <views/cef_browser_view.h>
#include <views/cef_window.h>
#include <wrapper/cef_helpers.h>

#include <include/base/cef_bind.h>
#include <include/cef_app.h>
#include <include/views/cef_browser_view.h>
#include <include/views/cef_window.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/wrapper/cef_helpers.h>

namespace lambda
{
	namespace gui
	{
    ///////////////////////////////////////////////////////////////////////////
    RenderHandler::RenderHandler(asset::VioletTextureHandle back_buffer)
      : size_(1280, 720)
      , m_lastBrowser(nullptr)
      , back_buffer_(back_buffer)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    bool lambda::gui::RenderHandler::GetViewRect(
      CefRefPtr<CefBrowser> browser, 
      CefRect& rect)
		{
			rect.Set(0, 0, size_.x, size_.y);
			return true;
		}

    ///////////////////////////////////////////////////////////////////////////
    void lambda::gui::RenderHandler::OnPaint(
      CefRefPtr<CefBrowser> browser, 
      PaintElementType type, 
      const RectList& dirtyRects, 
      const void* buffer, 
      int width, 
      int height)
		{
      m_lastBrowser = browser;

			auto& layer = back_buffer_->getLayer(0u);
			layer.resize((uint32_t)width, (uint32_t)height);
			Vector<char> data(width * height * 4u);
			memcpy(data.data(), buffer, data.size());
			layer.setData(data);

			/*std::cout << toString(width).c_str() << " " << 
        toString(height).c_str() << "\n";
			for (const auto& rect : dirtyRects)
			{
				std::cout << "\t" << toString(rect.x).c_str() << " " << 
          toString(rect.y).c_str() << " " << toString(rect.width).c_str() << 
          " " << toString(rect.height).c_str() << "\n";
			}*/
		}

    ///////////////////////////////////////////////////////////////////////////
    void RenderHandler::loadURL(const String& file)
    {
      m_lastBrowser->GetMainFrame()->LoadURL(file.c_str());
    }

    ///////////////////////////////////////////////////////////////////////////
    CefRefPtr<CefBrowser> RenderHandler::GetLastBrowser() const
    {
      return m_lastBrowser;
    }

    ///////////////////////////////////////////////////////////////////////////
    void RenderHandler::setSize(const glm::uvec2& size)
    {
      size_ = size;
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace
		{
			// When using the Views framework this object provides the delegate
			// implementation for the CefWindow that hosts the Views-based browser.
			class SimpleWindowDelegate : public CefWindowDelegate {
			public:
				explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
					: browser_view_(browser_view) {}

				void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE {
					// Add the browser view and show the window.
					window->AddChildView(browser_view_);
					window->Show();

					// Give keyboard focus to the browser view.
					browser_view_->RequestFocus();
				}

				void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE {
					browser_view_ = NULL;
				}

				bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE {
					// Allow the window to close if the browser says it's OK.
					CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
					if (browser)
						return browser->GetHost()->TryCloseBrowser();
					return true;
				}

			private:
				CefRefPtr<CefBrowserView> browser_view_;

				IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
				DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
			};

		}  // namespace

    ///////////////////////////////////////////////////////////////////////////
		SimpleApp::SimpleApp(asset::VioletTextureHandle back_buffer,
      CefRefPtr<RenderHandler> renderer,
      CefRefPtr<BrowserClient> client) : 
      back_buffer_(back_buffer)
      , renderer_(renderer)
      , client_(client)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void SimpleApp::OnContextInitialized() {
			CEF_REQUIRE_UI_THREAD();

			// Specify CEF browser settings here.
			CefBrowserSettings browser_settings;
			browser_settings.windowless_frame_rate = 60;

      std::string url = "C:/github/lambda-engine/test.html";

			CefWindowInfo window_info;
			window_info.SetAsWindowless(NULL);
			//window_info.SetAsPopup(NULL, "cefsimple");
			CefBrowserHost::CreateBrowser(
        window_info, 
        client_.get(),
        url, 
        browser_settings, 
        NULL
      );
		}

    ///////////////////////////////////////////////////////////////////////////
    GUI::GUI()
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    GUI::~GUI()
    {
      delete renderer_;
      delete client_;
      delete app_;

      CefShutdown();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void GUI::initialize(asset::VioletTextureHandle back_buffer)
    {
      back_buffer_ = back_buffer;
      renderer_ = new RenderHandler(back_buffer_);
      client_ = new BrowserClient(renderer_);
      app_ = new SimpleApp(back_buffer_, renderer_, client_);

#ifdef VIOLET_WIN32
      CefMainArgs main_args(GetModuleHandle(NULL));
#else
      int argc = 0;
      char** argv = nullptr;
      CefMainArgs main_args(argc, argv);
#endif

      int exit_code = CefExecuteProcess(main_args, nullptr, nullptr);
      LMB_ASSERT(exit_code < 0, "CEF: Failed to execute process");

      CefSettings settings;
      settings.no_sandbox = true;
      settings.windowless_rendering_enabled = true;
      CefString(&settings.browser_subprocess_path).FromASCII("cef-helper.exe");

      bool ret = CefInitialize(main_args, settings, app_.get(), nullptr);
      LMB_ASSERT(ret, "CEF: Failed to initialize");
    }

    ///////////////////////////////////////////////////////////////////////////
    class FrameCounterXX
    {
    public:
      void tick()
      {
        frames++;
        if (timer.elapsed().seconds() >= timer_switch)
        {
          timer.reset();
          last_frames = (unsigned int)((double)frames * (1.0 / timer_switch));
          frames = 0u;
        }
      }
      void setSwitch(const double& timer_switch)
      {
        this->timer_switch = timer_switch;
      }
      unsigned int getFrames() const
      {
        return last_frames;
      }

    private:
      utilities::Timer timer;
      double timer_switch = 1.0;
      unsigned int frames = 0u;
      unsigned int last_frames = 0u;
    };
  
    ///////////////////////////////////////////////////////////////////////////
    glm::vec4 HSVtoRGBXX(const glm::vec4& hsv) {
      glm::vec4 rgb = hsv;
      float fC = hsv.z * hsv.y; // Chroma
      float fHPrime = fmodf(hsv.x / 60.0f, 6.0f);
      float fX = fC * (1 - fabsf(fmodf(fHPrime, 2) - 1));
      float fM = hsv.z - fC;

      if (0 <= fHPrime&& fHPrime < 1) {
        rgb.x = fC;
        rgb.y = fX;
        rgb.z = 0;
      }
      else if (1 <= fHPrime&& fHPrime < 2) {
        rgb.x = fX;
        rgb.y = fC;
        rgb.z = 0;
      }
      else if (2 <= fHPrime&& fHPrime < 3) {
        rgb.x = 0;
        rgb.y = fC;
        rgb.z = fX;
      }
      else if (3 <= fHPrime&& fHPrime < 4) {
        rgb.x = 0;
        rgb.y = fX;
        rgb.z = fC;
      }
      else if (4 <= fHPrime&& fHPrime < 5) {
        rgb.x = fX;
        rgb.y = 0;
        rgb.z = fC;
      }
      else if (5 <= fHPrime&& fHPrime < 6) {
        rgb.x = fC;
        rgb.y = 0;
        rgb.z = fX;
      }
      else {
        rgb.x = 0;
        rgb.y = 0;
        rgb.z = 0;
      }

      rgb.x += fM;
      rgb.y += fM;
      rgb.z += fM;

      return rgb;
    }

    ///////////////////////////////////////////////////////////////////////////
    void GUI::update(const double& delta_time)
    {
      CefDoMessageLoopWork();

      CefRefPtr<CefBrowser> last_browser = static_cast<RenderHandler*>(
        renderer_.get()
      )->GetLastBrowser();
      if (!last_browser)
        return;
      
      static FrameCounterXX frame_counter;
      frame_counter.tick();

      static constexpr double frame_target_max = 60.0;
      static constexpr double frame_target_min = 40.0;
      double frame_fps = (double)frame_counter.getFrames();
      double frame_budget = (frame_target_max / frame_fps) * 100.0f;
      double frame_budget_h;
      if (frame_fps < frame_target_min)
      {
        frame_budget_h = 0.0;
      }
      else if (frame_fps > frame_target_max)
      {
        frame_budget_h = 120.0;
      }
      else
      {
        frame_budget_h = 
          120.0 * ((frame_fps - frame_target_min) / 
          (frame_target_max - frame_target_min));
      }

      last_browser->GetMainFrame()->ExecuteJavaScript(
        "document.getElementById('fps').innerHTML = 'FPS: " + 
        std::to_string((int)frame_fps) + "';", CefString(), 0);

      last_browser->GetMainFrame()->ExecuteJavaScript(
        "document.getElementById('ms').innerHTML = 'MS: " + 
        std::to_string((int)((1.0 / frame_fps) * 1000.0)) + "';", 
        CefString(), 0);

      last_browser->GetMainFrame()->ExecuteJavaScript(
        "document.getElementById('budget').innerHTML = 'Budget: " + 
        std::to_string((int)frame_budget) + "';", CefString(), 0);

      last_browser->GetMainFrame()->ExecuteJavaScript(
        "document.getElementById('budget').style.color = 'hsl(" + 
        std::to_string(frame_budget_h) + ",100%,50%)';", CefString(), 0);
    }

    ///////////////////////////////////////////////////////////////////////////
    bool GUI::handleWindowMessage(const platform::WindowMessage& message)
    {
      CefRefPtr<CefBrowser> last_browser = static_cast<RenderHandler*>(
        renderer_.get()
      )->GetLastBrowser();
      
      if (!last_browser)
        return false;

      if (message.type == platform::WindowMessageType::kMouseMove)
      {
        mouse_position_.x = (int)message.data[0u];
        mouse_position_.y = (int)message.data[1u];

        CefMouseEvent event;
        event.x = mouse_position_.x;
        event.y = mouse_position_.y;
        last_browser->GetHost()->SendMouseMoveEvent(event, false);
      }
      else if (message.type == platform::WindowMessageType::kMouseButton)
      {
        CefMouseEvent event;
        event.x = mouse_position_.x;
        event.y = mouse_position_.y;
        cef_mouse_button_type_t button;
        switch (message.data[0u])
        {
        case 0u: button = cef_mouse_button_type_t::MBT_LEFT;   break;
        case 1u: button = cef_mouse_button_type_t::MBT_MIDDLE; break;
        case 2u: button = cef_mouse_button_type_t::MBT_RIGHT;  break;
        }
        bool up = message.data[1u] ? false : true;
        uint32_t todo = 0u; // TODO (Hilze): Fill in.
        last_browser->GetHost()->SendMouseClickEvent(event, button, up, todo);
      }
      else if (message.type == platform::WindowMessageType::kMouseScroll)
      {
        CefMouseEvent event;
        event.x = mouse_position_.x;
        event.y = mouse_position_.y;
        int scroll = (int)((float)message.data[0] / 120.0f);
        last_browser->GetHost()->SendMouseWheelEvent(event, scroll, 0);
      }
      else if (message.type == platform::WindowMessageType::kFocus)
      {
        bool focus = message.data[0] ? true : false;
        last_browser->GetHost()->SendFocusEvent(focus);
      }
      else if (message.type == platform::WindowMessageType::kResize)
      {
        renderer_->setSize(glm::uvec2(message.data[0u], message.data[1u]));
        last_browser->GetHost()->WasResized();
      }

      return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    void GUI::loadURL(const String& file)
    {
      renderer_->loadURL(file);
    }
	}
}
