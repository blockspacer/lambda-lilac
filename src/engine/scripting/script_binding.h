#pragma once
#include <memory/memory.h>

namespace lambda
{
  namespace world
  {
    class IWorld;
  }

  namespace scripting
  {
    void ScriptBinding(world::IWorld* world);
    void ScriptRelease();
  }
}