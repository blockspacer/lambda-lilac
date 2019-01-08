#pragma once

namespace chaiscript
{
  class ChaiScript;
}
namespace world
{
  class IWorld;
}

namespace lambda
{
  namespace scripting
  {
    void bind(chaiscript::ChaiScript* context);
    void setWorld(world::IWorld* world);
  }
}