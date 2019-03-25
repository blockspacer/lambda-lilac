#include "wren_context.h"
#include "scripting/wren/wren_binding.h"
#include <memory/memory.h>
#include <assets/texture.h>
#include <assets/mesh.h>
#include <assets/mesh_io.h>
#include <assets/shader.h>
#include <assets/shader_io.h>
#include <systems/transform_system.h>
#include <utils/file_system.h>
#include <platform/post_process_manager.h>
#include <interfaces/iworld.h>
#include <systems/entity_system.h>
#include <systems/transform_system.h>
#include <systems/camera_system.h>
#include <systems/light_system.h>
#include <systems/mesh_render_system.h>
#include <glm/gtx/norm.hpp>

#include <wren.hpp>

namespace lambda
{ 
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    bool WrenContext::initialize(const Map<String, void*>& functions)
    {
      WrenConfiguration configuration;
      wrenInitConfiguration(&configuration);
      configuration.errorFn = [](
        WrenVM* vm, 
        WrenErrorType type, 
        const char* module, 
        int line, 
        const char* message)
      {
        String str = (type == WREN_ERROR_COMPILE) ? ("COMPILE: ") : 
          ((type == WREN_ERROR_RUNTIME) ? ("RUNTIME: ") : 
          ((type == WREN_ERROR_STACK_TRACE) ? ("STACK TRACE: ") : ("")));
        if (module != nullptr) str += "<" + String(module) + ", " + 
          toString(line) + "> ";
        foundation::Error(str + String(message) + "\n");
      };
      
      configuration.reallocateFn = [](void* memory, size_t newSize) {
        return foundation::Memory::reallocate(memory, newSize); 
      };

      configuration.writeFn = [](WrenVM* vm, const char* str) {
        foundation::InfoNP(str); 
      };
      
      WrenBind(&configuration);
      vm_ = wrenNewVM(&configuration);

      return true;
    }

    bool WrenContext::loadScripts(const Vector<String>& files)
    {
      for (const String& file : files)
      {
        WrenInterpretResult result = wrenInterpret(
          vm_, 
          "main", 
          FileSystem::FileToString(file).c_str()
        );

        switch (result)
        {
        case WREN_RESULT_COMPILE_ERROR:
        case WREN_RESULT_RUNTIME_ERROR:
          return false;
        case WREN_RESULT_SUCCESS: // Do nothing.
          break;
        }
      }

      wrenEnsureSlots(vm_, 1);
      wrenGetVariable(vm_, "main", "World", 0);
      world_.class_       = wrenGetSlotHandle(vm_, 0);
      world_.constructor  = wrenMakeCallHandle(vm_, "new()");
      world_.initialize   = wrenMakeCallHandle(vm_, "initialize()");
      world_.terminate    = wrenMakeCallHandle(vm_, "terminate()");
      world_.update       = wrenMakeCallHandle(vm_, "update()");
      world_.fixed_update = wrenMakeCallHandle(vm_, "fixedUpdate()");
      
      wrenSetSlotHandle(vm_, 0, world_.class_);
      wrenCall(vm_, world_.constructor);
      world_.instance = wrenGetSlotHandle(vm_, 0);

      return true;
    }

    bool WrenContext::terminate()
    {
      wrenReleaseHandle(vm_, world_.instance);
      wrenReleaseHandle(vm_, world_.initialize);
      wrenReleaseHandle(vm_, world_.terminate);
      wrenReleaseHandle(vm_, world_.update);
      wrenReleaseHandle(vm_, world_.fixed_update);
      wrenReleaseHandle(vm_, world_.class_);

      wrenFreeVM(vm_);
      vm_ = nullptr;

      return true;
    }
    
    void WrenContext::collectGarbage()
    {
      wrenCollectGarbage(vm_);
    }

    ScriptValue WrenContext::executeFunction(
      const String& declaration, 
      const Vector<ScriptValue>& args)
    {
      if (declaration.find("Game::") != String::npos)
      {
        String decl = declaration.substr(strlen("Game::"));

        if (declaration.find("Initialize") != String::npos)
          return executeFunction(world_.instance, world_.initialize, args);
        else if (declaration.find("Terminate") != String::npos)
          return executeFunction(world_.instance, world_.terminate, args);
        else if (declaration.find("FixedUpdate") != String::npos)
          return executeFunction(world_.instance, world_.fixed_update, args);
        else if (declaration.find("Update") != String::npos)
          return executeFunction(world_.instance, world_.update, args);
      }
      /*else
      {
        LMB_ASSERT(false, "WREN: Don't know what to do??");
      }*/

      return ScriptValue();
    }

    ScriptValue WrenContext::executeFunction(
      const void* object, 
      const void* function, 
      const Vector<ScriptValue>& args)
    {
      wrenEnsureSlots(vm_, (int16_t)args.size() + 1);
      for (int16_t i = 0; i < (int16_t)args.size(); ++i)
				WrenHandleValue(vm_, args[i], i + 1);

      try
      {
        wrenSetSlotHandle(vm_, 0, (WrenHandle*)object);
        wrenCall(vm_, (WrenHandle*)function);
      }
      catch (std::exception e)
      {
        LMB_ASSERT(false, e.what());
      }

      return ScriptValue();
    }

    ///////////////////////////////////////////////////////////////////////////
    void WrenContext::freeHandle(void* handle)
    {
      wrenReleaseHandle(vm_, (WrenHandle*)handle);
    }

    ///////////////////////////////////////////////////////////////////////////
    void WrenContext::setBreakPoint(const String& file, const int16_t& line)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    ScriptArray WrenContext::scriptArray(const void* data)
    {
      return ScriptArray();
    }

    ///////////////////////////////////////////////////////////////////////////
    void WrenContext::setWorld(world::IWorld* world)
    {
      WrenSetWorld(world);
    }

    ///////////////////////////////////////////////////////////////////////////
    void WrenContext::ExecuteWithDebugger()
    {
    }
  }
}
