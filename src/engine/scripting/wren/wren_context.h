#pragma once
#include "interfaces/iscript_context.h"
#include "scripting/script_function.h"

struct WrenVM;
struct WrenHandle;

namespace lambda
{
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    class WrenContext : public IScriptContext
    {
    public:
      virtual ~WrenContext() {};
      virtual bool initialize(const Map<String, void*>& functions) override;
      virtual bool loadScripts(const Vector<String>& files) override;
      virtual bool terminate() override;
      virtual void collectGarbage() override;
      virtual ScriptValue executeFunction(
        const String& declaration, 
        const Vector<ScriptValue>& args
      ) override;
      virtual ScriptValue executeFunction(
        const void* object, 
        const void* function, 
        const Vector<ScriptValue>& args
      ) override;
      virtual void freeHandle(void* handle) override;
      virtual void setBreakPoint(
        const String& file, 
        const int16_t& line
      ) override;
      virtual ScriptArray scriptArray(const void* data) override;
      virtual void setWorld(world::IWorld* world) override;

    private:
      void ExecuteWithDebugger();

    private:
      WrenVM* vm_;
      
      /////////////////////////////////////////////////////////////////////////
      struct World {
        WrenHandle* class_;
        WrenHandle* constructor;
        WrenHandle* instance;
        WrenHandle* initialize;
        WrenHandle* deinitialize;
        WrenHandle* update;
        WrenHandle* fixed_update;
      } world_;
    };
  }
}
