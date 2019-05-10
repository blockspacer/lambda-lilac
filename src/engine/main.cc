#include <memory/memory.h>
#include <memory/frame_heap.h>
//void* operator new  (std::size_t count)
//{
//	return lambda::foundation::Memory::allocate(count, lambda::foundation::Memory::new_allocator());
//}
//void* operator new[](std::size_t count)
//{
//	return lambda::foundation::Memory::allocate(count, lambda::foundation::Memory::new_allocator());
//}
//void* operator new  (std::size_t count, const std::nothrow_t& tag)
//{
//	return lambda::foundation::Memory::allocate(count, lambda::foundation::Memory::new_allocator());
//}
//void* operator new[](std::size_t count, const std::nothrow_t& tag)
//{
//	return lambda::foundation::Memory::allocate(count, lambda::foundation::Memory::new_allocator());
//}
//void operator delete  (void* ptr)
//{
//	lambda::foundation::Memory::deallocate(ptr);
//}
//void operator delete[](void* ptr)
//{
//	lambda::foundation::Memory::deallocate(ptr);
//}
//void operator delete  (void* ptr, const std::nothrow_t& tag)
//{
//	lambda::foundation::Memory::deallocate(ptr);
//}
//void operator delete[](void* ptr, const std::nothrow_t& tag)
//{
//	lambda::foundation::Memory::deallocate(ptr);
//}
//void operator delete  (void* ptr, std::size_t sz)
//{
//	lambda::foundation::Memory::deallocate(ptr);
//}
//void operator delete[](void* ptr, std::size_t sz)
//{
//	lambda::foundation::Memory::deallocate(ptr);
//}

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
#include "interfaces/iworld.h"
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
    platform::IWindow* window,
    platform::IRenderer* renderer,
    scripting::IScriptContext* scripting
  ) : IWorld(window, renderer, scripting) {}

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
    Average()
	{
		memset(averages, 0, sizeof(averages));
		total = 0.0f;
		idx = 0u;
	}
    void add(float entry)
	{
      idx = (idx + 1u) % kCount;
	  total = total - averages[idx] + entry;
      averages[idx] = entry;
    }
    float average() const
    {
		return total / kCount;
    }
  private:
    uint32_t idx;
	float total;
    static constexpr uint32_t kCount = 100u;
    float averages[kCount];
  };

  void update(const double& delta_time) override
  {
	  frame_counter.tick();

	  float mem_def = (float)(foundation::Memory::default_allocator()->allocated() + foundation::Memory::new_allocator()->allocated()) / (1024.0f * 1024.0f);
	  getGUI().executeJavaScript("if (gameState == Game) { updateAllocatedMemory(" + toString(round(mem_def, 3)) + ") }");

	  static constexpr uint32_t kTimerCount = 4u;
	  static Average kTimers[kTimerCount];
	  static constexpr char* kTimerNames[kTimerCount]   = { "FixedUpdate",  "Update",       "CollectGarbage", "ConstructRender"/*, "OnRender"*/     };
	  static constexpr bool  kGameTimer[kTimerCount]    = { true,           true,           true,             false/*,             false*/          };
	  static constexpr char* kTimerColours[kTimerCount] = { "rgb(100,0,0)", "rgb(144,0,0)", "rgb(188,0,0)",   "rgb(0,0,144)"/*,    "rgb(0,0,188)"*/ };
	  String execute_string = "";

	  for (uint32_t i = 0; i < kTimerCount; ++i)
	  {
		  kTimers[i].add((float)getProfiler().getTime(kTimerNames[i]));

		  // name, colour, value
			String name = kTimerNames[i];
			name = /*toString((int)(kTimers[i].average() * 1000.0f));// +"\n" +*/ kTimerNames[i];
		  execute_string += "\"" + String(name) + "\", " + toString(kTimers[i].average()) + ", \"" + kTimerColours[i] + "\"";
		  if (i < kTimerCount - 1)
			  execute_string += ", ";
	  }

	  getGUI().executeJavaScript("if (gameState == Game) { setAllTimers([" + execute_string + "] ) }");
	  
	  float dynamic_resolution_scale = 1.0f;

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

		// TODO (Hilze): Get rid of this ASAP!
		dynamic_resolution_scale = 1.0f;
  }

  virtual void fixedUpdate() override
  {
  }

  virtual void handleWindowMessage(const platform::WindowMessage& message) override
  {
    switch (message.type)
    {
    case platform::WindowMessageType::kClose:
			getScene().window->close();
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
		platform::IRenderer* renderer = foundation::Memory::construct<windows::D3D11Renderer>();
#elif defined VIOLET_RENDERER_METAL
		platform::IRenderer* renderer = foundation::Memory::construct<osx::MetalRenderer>();
#elif defined VIOLET_RENDERER_VULKAN
		platform::IRenderer* renderer = foundation::Memory::construct<linux::VulkanRenderer>();
#elif defined VIOLET_RENDERER_NO
		platform::IRenderer* renderer = foundation::Memory::construct<windows::NoRenderer>();
#else
#error No valid renderer found!
#endif

		{
#if defined VIOLET_WINDOW_WIN32
			platform::IWindow* window = foundation::Memory::construct<window::Win32Window>();
#elif defined VIOLET_WINDOW_GLFW
			platform::IWindow* window = foundation::Memory::construct<window::GLFWWindow>();
#elif defined VIOLET_WINDOW_SDL2
			platform::IWindow* window = foundation::Memory::construct<window::SDL2Window>();
#else
#error No valid window found!
#endif

#if defined VIOLET_SCRIPTING_WREN
			scripting::IScriptContext* scripting = foundation::Memory::construct<scripting::WrenContext>();
			String script = "resources/scripts/wren/main.wren";
#elif defined VIOLET_SCRIPTING_ANGEL
			scripting::IScriptContext* scripting = foundation::Memory::construct<scripting::AngelScriptContext>();
			String script = "resources/scripts/angelscript/main.as";
#else
#error No valid scripting engine found!
#endif

			window->create(glm::uvec2(1280u, 720u), "Engine");

			{
				MyWorld world(window, renderer, scripting);

				//scripting::ScriptBinding(&world);
				scripting->initialize({});
				scripting->loadScripts({ script });

				world.run();
			}

			//scripting::ScriptRelease();

			foundation::Memory::destruct(asset::ShaderManager::getInstance());
			foundation::Memory::destruct(asset::TextureManager::getInstance());
			foundation::Memory::destruct(asset::WaveManager::getInstance());
			foundation::Memory::destruct(asset::MeshManager::getInstance());
			foundation::Memory::destruct(foundation::GetFrameHeap());

			window->close();
			renderer->deinitialize();

			foundation::Memory::destruct(scripting);
			foundation::Memory::destruct(renderer);
			foundation::Memory::destruct(window);
		}
	}

	asset::VioletRefHandler<asset::Shader>::releaseAll();
	asset::VioletRefHandler<asset::Texture>::releaseAll();
	asset::VioletRefHandler<asset::Wave>::releaseAll();
	asset::VioletRefHandler<asset::Mesh>::releaseAll();
	lambda::FileSystem::SetBaseDir("");

	return 0;
}
