#pragma once
#include "interfaces/iscript_context.h"
#include "scripting/script_function.h"
#include <memory/memory.h>

class asIScriptEngine;
class asIScriptModule;
class asIScriptContext;
class asIScriptFunction;
class asIScriptObject;
class CDebugger;

namespace lambda
{
  namespace scripting
  {
    class AngelScriptContext : public IScriptContext
    {
    public:
      virtual bool initialize(const Map<String, void*>& functions) override;
      virtual bool loadScripts(const Vector<String>& files) override;
      virtual bool terminate() override;
      virtual void collectGarbage() override;
      virtual ScriptValue executeFunction(const String& declaration, const Vector<ScriptValue>& args) override;
      virtual ScriptValue executeFunction(const void* object, const void* function, const Vector<ScriptValue>& args) override;
      virtual void freeHandle(void* handle) override;
      virtual void setBreakPoint(const String& file, const int16_t& line) override;
      virtual ScriptArray scriptArray(const void* data);
      virtual void setWorld(world::IWorld* world) override;

    private:
      class AngelScriptFunction : public IScriptFunction
      {
      public:
        AngelScriptFunction(asIScriptFunction* function);
        asIScriptFunction* function;
      };

    private:
      void ExecuteWithDebugger();

    private:
      CDebugger* debugger_;
      UnorderedMap<String, AngelScriptFunction> functions_;
      asIScriptEngine*   engine_  = nullptr;
      asIScriptModule*   module_  = nullptr;
      asIScriptContext*  context_ = nullptr;
      asIScriptObject*   game_    = nullptr;
      asIScriptFunction* game_initialize_   = nullptr;
      asIScriptFunction* game_terminate_    = nullptr;
      asIScriptFunction* game_update_       = nullptr;
      asIScriptFunction* game_fixed_update_ = nullptr;
    };
  }
}