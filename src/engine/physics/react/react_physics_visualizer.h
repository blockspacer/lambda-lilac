#pragma once
#include <glm/glm.hpp>
#include <iostream>

namespace lambda
{
  namespace platform
  {
    class DebugRenderer;
  }
  namespace physics
  {
    ///////////////////////////////////////////////////////////////////////////
    class ReactPhysicVisualizer
    {
    public:
      void initialize(
		  platform::DebugRenderer* debug_renderer, 
		  bool draw_enabled
	  );
	  void draw();
	  bool getDrawEnabled() const;
	  void setDrawEnabled(bool draw_enabled);

    private:
      platform::DebugRenderer* debug_renderer_;
	  bool draw_enabled_;
    };
  }
}