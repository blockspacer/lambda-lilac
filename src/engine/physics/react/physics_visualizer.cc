#include "physics_visualizer.h"
#include "platform/debug_renderer.h"
#include <utils/console.h>

namespace lambda
{
  namespace physics
  {
    ///////////////////////////////////////////////////////////////////////////
    void PhysicVisualizer::initialize(
		platform::DebugRenderer* debug_renderer, 
		bool draw_enabled)
    {
	  draw_enabled_ = draw_enabled;
      debug_renderer_ = debug_renderer;
    }

	///////////////////////////////////////////////////////////////////////////
	void PhysicVisualizer::draw()
	{
		if (draw_enabled_)
		{

		}
	}

	///////////////////////////////////////////////////////////////////////////
	bool PhysicVisualizer::getDrawEnabled() const
	{
		return draw_enabled_;
	}

	///////////////////////////////////////////////////////////////////////////
	void PhysicVisualizer::setDrawEnabled(bool draw_enabled)
	{
		draw_enabled_ = draw_enabled;
	}
  }
}
