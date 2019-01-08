#pragma once
#include "interfaces/iscript_context.h"
#include "scripting/script_function.h"

namespace chaiscript
{
  class ChaiScript;
}

namespace lambda
{
  namespace scripting
  {
    class ChaiScriptContext : public IScriptContext
    {
    public:
      virtual bool initialize(const Map<String, void*>& functions) override;
      virtual bool loadScripts(const Vector<String>& files) override;
      virtual bool terminate() override;
      virtual void collectGarbage() override;
      virtual ScriptValue executeFunction(const String& declaration, const Vector<ScriptValue>& args) override;
      virtual void setBreakPoint(const String& file, const int16_t& line) override;
      virtual ScriptArray scriptArray(const void* data);
      virtual void setWorld(world::IWorld* world) override;

    private:
      void ExecuteWithDebugger();

    private:
      chaiscript::ChaiScript* context_;
    };
  }
}