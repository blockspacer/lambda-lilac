#pragma once

namespace lambda
{
	namespace world
	{
		class IWorld;
	}

  namespace gui
  {
		extern void Create(world::IWorld* world);
  }
}
