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
    ///////////////////////////////////////////////////////////////////////////
    extern void WrenBind(void* config);
    extern void WrenSetWorld(world::IWorld* world);
    extern void WrenRelease(WrenVM* vm);
  }
}