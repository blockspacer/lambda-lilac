#pragma once
#include <containers/containers.h>
#include "scripting/script_value.h"

namespace lambda
{
	namespace world
	{
		class IWorld;
	}
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    class IScriptContext
    {
    public:
      virtual bool initialize(const Map<String, void*>& functions) = 0;
      virtual bool loadScripts(const Vector<String>& files) = 0;
      virtual bool terminate() = 0;
      virtual void collectGarbage() = 0;
      virtual ScriptValue executeFunction(
        const String& declaration, 
        const Vector<ScriptValue>& args
      ) = 0;
      virtual ScriptValue executeFunction(
        const void* object, 
        const void* function, 
        const Vector<ScriptValue>& args
      ) = 0;
      virtual void freeHandle(void* handle) = 0;
      virtual void setBreakPoint(const String& file, const int16_t& line) = 0;
      virtual ScriptArray scriptArray(const void* data) = 0;
      virtual void setWorld(world::IWorld* world) = 0;
    };
  }
}