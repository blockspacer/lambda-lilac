#pragma once
#include <cef_app.h>
#include <cef_render_handler.h>
#include <cef_client.h>
#include <list>
#include <assets/texture.h>
#include <interfaces/iwindow.h>

namespace lambda
{
  namespace gui
  {
    ///////////////////////////////////////////////////////////////////////////
    class RenderHandler : public CefRenderHandler {
	  public:
      RenderHandler(asset::VioletTextureHandle back_buffer);
		  // Include the default reference counting implementation.
		  IMPLEMENT_REFCOUNTING(RenderHandler);

		  // Inherited via CefRenderHandler
		  virtual bool GetViewRect(
        CefRefPtr<CefBrowser> browser, 
        CefRect& rect
      ) override;
		  virtual void OnPaint(
        CefRefPtr<CefBrowser> browser, 
        PaintElementType type, 
        const RectList& dirtyRects, 
        const void* buffer, 
        int width, 
        int height
      ) override;
      
    public:
      void loadURL(const String& file);
      CefRefPtr<CefBrowser> GetLastBrowser() const;
      void setSize(const glm::uvec2& size);

	  private:
      glm::uvec2 size_;
      CefRefPtr<CefBrowser> m_lastBrowser;
      asset::VioletTextureHandle back_buffer_;
	  };

    ///////////////////////////////////////////////////////////////////////////
    class BrowserClient : public CefClient
	  {
	  public:
		  BrowserClient(RenderHandler *renderHandler) :
			  m_renderHandler(renderHandler)
		  {
		  }

		  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { 
        return m_renderHandler; 
      }

	  private:
		  CefRefPtr<CefRenderHandler> m_renderHandler;

		  IMPLEMENT_REFCOUNTING(BrowserClient);
	  };

    ///////////////////////////////////////////////////////////////////////////
    class SimpleApp : public CefApp, public CefBrowserProcessHandler {
	  public:
		  SimpleApp(asset::VioletTextureHandle back_buffer,
      CefRefPtr<RenderHandler> renderer,
      CefRefPtr<BrowserClient> client);

		  // CefApp methods:
		  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
			  OVERRIDE {
			  return this;
		  }

		  // CefBrowserProcessHandler methods:
		  virtual void OnContextInitialized() OVERRIDE;

	  private:
		  // Include the default reference counting implementation.
		  IMPLEMENT_REFCOUNTING(SimpleApp);
      asset::VioletTextureHandle back_buffer_;
      CefRefPtr<RenderHandler> renderer_;
      CefRefPtr<BrowserClient> client_;
    };


    ///////////////////////////////////////////////////////////////////////////
    class GUI
    {
    public:
      GUI();
      ~GUI();
      void initialize(asset::VioletTextureHandle back_buffer);
      void update(const double& delta_time);
      bool handleWindowMessage(const platform::WindowMessage& message);
      void loadURL(const String& file);

    private:
      glm::ivec2 mouse_position_;
      asset::VioletTextureHandle back_buffer_;
      CefRefPtr<RenderHandler> renderer_;
      CefRefPtr<BrowserClient> client_;
      CefRefPtr<SimpleApp> app_;
    };
  }
}
