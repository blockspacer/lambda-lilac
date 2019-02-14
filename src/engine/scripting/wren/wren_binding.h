#pragma once

struct WrenVM;

namespace lambda
{
  namespace world
  {
    class IWorld;
  }
  namespace scripting
  {
		class ScriptValue;

    ///////////////////////////////////////////////////////////////////////////
    extern void WrenBind(void* config);
		extern void WrenSetWorld(world::IWorld* world);
		extern void WrenHandleValue(WrenVM* vm, const ScriptValue& value, int slot);
    extern void WrenRelease(WrenVM* vm);
  }
}