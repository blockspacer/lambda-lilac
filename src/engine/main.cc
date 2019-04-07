#include <memory/memory.h>
#include <memory/frame_heap.h>
void* operator new  (std::size_t count)
{
	return lambda::foundation::Memory::allocate(count, lambda::foundation::Memory::new_allocator());
}
void* operator new[](std::size_t count)
{
	return lambda::foundation::Memory::allocate(count, lambda::foundation::Memory::new_allocator());
}
void* operator new  (std::size_t count, const std::nothrow_t& tag)
{
	return lambda::foundation::Memory::allocate(count, lambda::foundation::Memory::new_allocator());
}
void* operator new[](std::size_t count, const std::nothrow_t& tag)
{
	return lambda::foundation::Memory::allocate(count, lambda::foundation::Memory::new_allocator());
}
void operator delete  (void* ptr)
{
	lambda::foundation::Memory::deallocate(ptr);
}
void operator delete[](void* ptr)
{
	lambda::foundation::Memory::deallocate(ptr);
}
void operator delete  (void* ptr, const std::nothrow_t& tag)
{
	lambda::foundation::Memory::deallocate(ptr);
}
void operator delete[](void* ptr, const std::nothrow_t& tag)
{
	lambda::foundation::Memory::deallocate(ptr);
}
void operator delete  (void* ptr, std::size_t sz)
{
	lambda::foundation::Memory::deallocate(ptr);
}
void operator delete[](void* ptr, std::size_t sz)
{
	lambda::foundation::Memory::deallocate(ptr);
}

#include "assets/shader.h"
#include "assets/mesh.h"
#include "assets/shader.h"
#include "assets/wave.h"

#include "systems/entity_system.h"
#include "systems/transform_system.h"
#include "systems/camera_system.h"
#include "systems/mesh_render_system.h"
#include "systems/light_system.h"
#include "utils/mt_manager.h"
#include "utils/file_system.h"
#include "utils/utilities.h"
#include "utils/profiler.h"

#include "systems/transform_system.h"
#include "platform/scene.h"
#include "scripting/binding/script_binding.h"
#include <containers/containers.h>

#include <INIReader.h>

#include <fstream>
#include <iostream>
#include <algorithm>

#if defined VIOLET_RENDERER_D3D11
#include "renderers/d3d11/d3d11_renderer.h"
#elif defined VIOLET_RENDERER_METAL
#include "renderers/metal/metal_renderer.h"
#elif defined VIOLET_RENDERER_VULKAN
#include "renderers/vulkan/vulkan_renderer.h"
#elif defined VIOLET_RENDERER_NO
#include "renderers/no/no_renderer.h"
#endif

#if defined VIOLET_WINDOW_WIN32
#include "windows/win32/win32_window.h"
#endif
#if defined VIOLET_WINDOW_GLFW
#include "windows/glfw/glfw_window.h"
#endif
#if defined VIOLET_WINDOW_SDL2
#include "windows/sdl2/sdl2_window.h"
#endif
  
//#undef VIOLET_SCRIPTING_WREN

#if defined VIOLET_SCRIPTING_ANGEL
#include "scripting/angel-script/angel_script_context.h"
#endif
#if defined VIOLET_SCRIPTING_WREN
#include "scripting/wren/wren_context.h"
#endif

#if defined VIOLET_IMGUI_NUKLEAR
#include "imgui/nuklear_imgui.h"
#endif
#if defined VIOLET_IMGUI_DEAR
#include "imgui/dear_imgui.h"
#endif
#if defined VIOLET_IMGUI_NO
#include "imgui/no_imgui.h"
#endif


using namespace lambda;

glm::vec4 RGBtoHSV(const glm::vec4& rgb) {
  glm::vec4 hsv = rgb;
  float fCMax = std::fmaxf(std::fmaxf(rgb.x, rgb.y), rgb.z);
  float fCMin = std::fminf(std::fminf(rgb.x, rgb.y), rgb.z);
  float fDelta = fCMax - fCMin;
  
  if(fDelta > 0) {
    if(fCMax == rgb.x) {
      hsv.x = 60 * (fmodf(((rgb.y - rgb.z) / fDelta), 6));
    } else if(fCMax == rgb.y) {
      hsv.x = 60 * (((rgb.z - rgb.x) / fDelta) + 2);
    } else if(fCMax == rgb.z) {
      hsv.x = 60 * (((rgb.x - rgb.y) / fDelta) + 4);
    }
    
    if(fCMax > 0) {
      hsv.y = fDelta / fCMax;
    } else {
      hsv.y = 0;
    }
    
    hsv.z = fCMax;
  } else {
    hsv.x = 0;
    hsv.y = 0;
    hsv.z = fCMax;
  }
  
  if(hsv.x < 0) {
    hsv.x = 360 + hsv.x;
  }

  return hsv;
}
glm::vec4 HSVtoRGB(const glm::vec4& hsv) {
  glm::vec4 rgb = hsv;
  float fC = hsv.z * hsv.y; // Chroma
  float fHPrime = fmodf(hsv.x / 60.0f, 6.0f);
  float fX = fC * (1 - fabsf(fmodf(fHPrime, 2) - 1));
  float fM = hsv.z - fC;
  
  if(0 <= fHPrime&& fHPrime < 1) {
    rgb.x = fC;
    rgb.y = fX;
    rgb.z = 0;
  } else if(1 <= fHPrime&& fHPrime < 2) {
    rgb.x = fX;
    rgb.y = fC;
    rgb.z = 0;
  } else if(2 <= fHPrime&& fHPrime < 3) {
    rgb.x = 0;
    rgb.y = fC;
    rgb.z = fX;
  } else if(3 <= fHPrime&& fHPrime < 4) {
    rgb.x = 0;
    rgb.y = fX;
    rgb.z = fC;
  } else if(4 <= fHPrime&& fHPrime < 5) {
    rgb.x = fX;
    rgb.y = 0;
    rgb.z = fC;
  } else if(5 <= fHPrime&& fHPrime < 6) {
    rgb.x = fC;
    rgb.y = 0;
    rgb.z = fX;
  } else {
    rgb.x = 0;
    rgb.y = 0;
    rgb.z = 0;
  }
  
  rgb.x += fM;
  rgb.y += fM;
  rgb.z += fM;

  return rgb;
}

class FrameCounter
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

class MyWorld : public world::IWorld
{
public:
  MyWorld(
    foundation::SharedPointer<platform::IWindow> window,
    foundation::SharedPointer<platform::IRenderer> renderer,
    foundation::SharedPointer<scripting::IScriptContext> scripting,
    foundation::SharedPointer<platform::IImGUI> imgui
  ) : IWorld(window, renderer, scripting, imgui) {}

  virtual ~MyWorld() {};

  void initialize() override
  {
	}
  void deinitialize() override
  {
  }
  double round(double value, unsigned int power)
  {
    double p = (double)pow(10, power);
    return floor(value * p) / p;
  }
  class Average
  {
  public:
    void add(float entry)
    {
      idx = (idx + 1u) % kCount;
      averages[idx] = entry;
    }
    float average() const
    {
      float average = 0.0f;
      for (uint8_t i = 0u; i < kCount; ++i)
        average += averages[i];
      return average / (float)kCount;
    }
  private:
    uint8_t idx = 0u;
    static constexpr uint8_t kCount = 100u;
    float averages[kCount];
  };
  void frameInfo()
  {
    foundation::SharedPointer<platform::IImGUI> imgui = getImGUI();

    static double frame_target_max = 60.0;
    static double frame_target_min = 40.0;
    double frame_fps = (double)frame_counter.getFrames();
    double frame_budget = frame_target_max / frame_fps;
    double frame_budget_h;
    if (frame_fps < frame_target_min)
      frame_budget_h = 0.0;
    else if (frame_fps > frame_target_max)
      frame_budget_h = 120.0;
    else
      frame_budget_h = 120.0 * ((frame_fps - frame_target_min) / (frame_target_max - frame_target_min));

    static bool open = true;
    if (imgui->imBegin("info", open, glm::vec2(0.0f, 0.0f), glm::vec2(170.0f, 600.0f)))
    {
      imgui->imText("fps:");
      imgui->imText(toString(round(frame_fps, 3u)));
      imgui->imText("ms:");
      imgui->imText(toString(round(1.0 / frame_fps, 3u)));
      imgui->imText("budget:");
      imgui->imTextColoured(toString(round(frame_budget, 3u)), HSVtoRGB(glm::vec4(frame_budget_h, 1.0f, 1.0f, 1.0f)));

			// Profiler.

#define XX(x) imgui->imTextMultiLine(toString(round(getProfiler().getTime(x), 3u)) + " - " + x);
			XX("Scripting: CollectGarbage");
			XX("Scripting: FixedUpdate");
			XX("Scripting: Update");
			XX("Systems: FixedUpdate");
			XX("Systems: Update");
			XX("Systems: OnRender");
			XX("Renderer: Update");
			XX("Renderer: StartFrame");
			XX("Renderer: EndFrame");
			XX("ImGUI: Update");
			XX("ImGUI: GenerateCommandList");

			imgui->imTextMultiLine(getScene().getSystem<components::MeshRenderSystem>()->profilerInfo());

      /*static Average timer_clear_everything;
      static Average timer_main_camera;
      static Average timer_lighting;
      static Average timer_post_processing;
      static Average timer_debug_rendering;
      static Average timer_copy_to_screen;
      static Average timer_imgui;
      timer_clear_everything.add((float)getRenderer()->getTimerMicroSeconds("Clear Everything") / 1000.0f);
      timer_main_camera.add((float)getRenderer()->getTimerMicroSeconds("Main Camera") / 1000.0f);
      timer_lighting.add((float)getRenderer()->getTimerMicroSeconds("Lighting") / 1000.0f);
      timer_post_processing.add((float)getRenderer()->getTimerMicroSeconds("Post Processing") / 1000.0f);
      timer_debug_rendering.add((float)getRenderer()->getTimerMicroSeconds("Debug Rendering") / 1000.0f);
      timer_copy_to_screen.add((float)getRenderer()->getTimerMicroSeconds("Copy To Screen") / 1000.0f);
      timer_imgui.add((float)getRenderer()->getTimerMicroSeconds("ImGUI") / 1000.0f);

      imgui->imText(toString(std::round(timer_clear_everything.average() * 100.0f) / 100.0f) + " - Clear Everything");
      imgui->imText(toString(std::round(timer_main_camera.average() * 100.0f) / 100.0f) + " - Main Camera");
      imgui->imText(toString(std::round(timer_lighting.average() * 100.0f) / 100.0f) + " - Lighting");
      imgui->imText(toString(std::round(timer_post_processing.average() * 100.0f) / 100.0f) + " - Post Processing");
      imgui->imText(toString(std::round(timer_debug_rendering.average() * 100.0f) / 100.0f) + " - Debug Rendering");
      imgui->imText(toString(std::round(timer_copy_to_screen.average() * 100.0f) / 100.0f) + " - Copy To Screen");
      imgui->imText(toString(std::round(timer_imgui.average() * 100.0f) / 100.0f) + " - ImGUI");*/

      static float dynamic_resolution_scale = 1.0f;
      if (imgui->imFloat1("DRS", dynamic_resolution_scale))
        getRenderer()->setShaderVariable(platform::ShaderVariable(Name("dynamic_resolution_scale"), dynamic_resolution_scale));

      static float ambient_intensity = 1.0f;
      if (imgui->imFloat1("AI", ambient_intensity))
        getRenderer()->setShaderVariable(platform::ShaderVariable(Name("ambient_intensity"), ambient_intensity));
    }

    imgui->imEnd();
  }

  void update(const double& delta_time) override
  {
	  frame_counter.tick();

	  float mem_def = (float)(foundation::Memory::default_allocator()->allocated() + foundation::Memory::new_allocator()->allocated()) / (1024.0f * 1024.0f);
	  getGUI().executeJavaScript("updateAllocatedMemory(" + toString(round(mem_def, 3)) + ")");

	  float dynamic_resolution_scale =
		  getShaderVariableManager().getShaderVariable(
			  Name("dynamic_resolution_scale")
		  ).data.at(0);

	  static const float drs[] = {
		  0.2f,
		  0.4f,
		  0.6f,
		  0.8f,
		  1.0f
	  };

	  bool scale_up   = false;
	  bool scale_down = false;

	  if (frame_counter.getFrames() < 60)
	  {
		  if (dynamic_resolution_scale < drs[(int)(((float)frame_counter.getFrames()) / 60.0f * 5.0f)])
			  scale_up = true;
		  else
			  scale_down = true;
	  }
	  else if (dynamic_resolution_scale < 1.0f)
		  scale_up = true;

	  if (scale_down)
		  dynamic_resolution_scale = max(dynamic_resolution_scale - 0.01f, drs[(int)(((float)frame_counter.getFrames()) / 60.0f * 5.0f)]);
	  if (scale_up)
		  dynamic_resolution_scale = min(dynamic_resolution_scale + 0.01f, 1.0f);

	  getShaderVariableManager().setVariable(platform::ShaderVariable(Name("dynamic_resolution_scale"), dynamic_resolution_scale));
  }

  virtual void fixedUpdate() override
  {
  }

  virtual void handleWindowMessage(const platform::WindowMessage& message) override
  {
    switch (message.type)
    {
    case platform::WindowMessageType::kClose:
      getWindow()->close();
      break;
    default:
      break;
    }
  }

private:
  FrameCounter frame_counter;
};

int main(int argc, char** argv)
{
	LMB_ASSERT(argc != 1, "No project folder was speficied!");

  lambda::FileSystem::SetBaseDir(argv[1]);

  {
#if defined VIOLET_RENDERER_D3D11
    foundation::SharedPointer<platform::IRenderer> renderer = foundation::Memory::constructShared<windows::D3D11Renderer>();
#elif defined VIOLET_RENDERER_METAL
	  foundation::SharedPointer<platform::IRenderer> renderer = foundation::Memory::constructShared<osx::MetalRenderer>();
#elif defined VIOLET_RENDERER_VULKAN
	  foundation::SharedPointer<platform::IRenderer> renderer = foundation::Memory::constructShared<linux::VulkanRenderer>();
#elif defined VIOLET_RENDERER_NO
    foundation::SharedPointer<platform::IRenderer> renderer = foundation::Memory::constructShared<windows::NoRenderer>();
#else
#error No valid renderer found!
#endif

    {
#if defined VIOLET_WINDOW_WIN32
      foundation::SharedPointer<platform::IWindow> window = foundation::Memory::constructShared<window::Win32Window>();
#elif defined VIOLET_WINDOW_GLFW
      foundation::SharedPointer<platform::IWindow> window = foundation::Memory::constructShared<window::GLFWWindow>();
#elif defined VIOLET_WINDOW_SDL2
      foundation::SharedPointer<platform::IWindow> window = foundation::Memory::constructShared<window::SDL2Window>();
#else
#error No valid window found!
#endif

#if defined VIOLET_SCRIPTING_WREN
      foundation::SharedPointer<scripting::IScriptContext> scripting = foundation::Memory::constructShared<scripting::WrenContext>();
	  String script = "resources/scripts/wren/main.wren";
#elif defined VIOLET_SCRIPTING_ANGEL
      foundation::SharedPointer<scripting::IScriptContext> scripting = foundation::Memory::constructShared<scripting::AngelScriptContext>();
	  String script = "resources/scripts/angelscript/main.as";
#else
#error No valid scripting engine found!
#endif

#if defined VIOLET_IMGUI_NUKLEAR
      foundation::SharedPointer<platform::IImGUI> imgui = foundation::Memory::constructShared<imgui::NuklearImGUI>();
#elif defined VIOLET_IMGUI_DEAR
      foundation::SharedPointer<platform::IImGUI> imgui = foundation::Memory::constructShared<imgui::DearImGUI>();
#elif defined VIOLET_IMGUI_NO
      foundation::SharedPointer<platform::IImGUI> imgui = foundation::Memory::constructShared<imgui::NoImGUI>();
#else
#error No valid imgui found!
#endif

      window->create(glm::uvec2(1280u, 720u), "Engine");

      MyWorld world(window, renderer, scripting, imgui);
      imgui->setFont("resources/fonts/DroidSans.ttf", 16.0f);

      scripting::ScriptBinding(&world);
      scripting->loadScripts({ script });

      world.run();

      scripting->terminate();
      scripting::ScriptRelease();

			foundation::Memory::destruct(asset::ShaderManager::getInstance());
			foundation::Memory::destruct(asset::TextureManager::getInstance());
			foundation::Memory::destruct(asset::WaveManager::getInstance());
			foundation::Memory::destruct(asset::MeshManager::getInstance());
			foundation::Memory::destruct(foundation::GetFrameHeap());
	  }
  }

  return 0;
}
