#include "newton_physics_visualizer.h"
#include "platform/debug_renderer.h"
#include <utils/console.h>

namespace lambda
{
  namespace physics
  {
    ///////////////////////////////////////////////////////////////////////////
    void NewtonPhysicVisualizer::initialize(
		platform::DebugRenderer* debug_renderer, 
		bool draw_enabled)
    {
	  draw_enabled_ = draw_enabled;
      debug_renderer_ = debug_renderer;
    }

	///////////////////////////////////////////////////////////////////////////
	void NewtonPhysicVisualizer::draw()
	{
		if (draw_enabled_)
		{

		}
	}

	///////////////////////////////////////////////////////////////////////////
	bool NewtonPhysicVisualizer::getDrawEnabled() const
	{
		return draw_enabled_;
	}

	///////////////////////////////////////////////////////////////////////////
	void NewtonPhysicVisualizer::setDrawEnabled(bool draw_enabled)
	{
		draw_enabled_ = draw_enabled;
	}
  }
}
