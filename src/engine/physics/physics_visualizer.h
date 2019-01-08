#pragma once
#include <LinearMath/btIDebugDraw.h>
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
    class PhysicVisualizer : public btIDebugDraw
    {
    public:
      void initialize(platform::DebugRenderer* debug_renderer);
      virtual void drawLine(
        const btVector3& from, 
        const btVector3& to, 
        const btVector3& fromColor, 
        const btVector3& toColor
      );
      virtual void drawLine(
        const btVector3& from, 
        const btVector3& to, 
        const btVector3& color
      );
      virtual void drawSphere(
        const btVector3& p, 
        btScalar radius, 
        const btVector3& color
      );
      virtual void drawTriangle(
        const btVector3& a, 
        const btVector3& b, 
        const btVector3& c,
        const btVector3& color, 
        btScalar alpha
      );
      virtual void drawContactPoint(
        const btVector3& PointOnB, 
        const btVector3& normalOnB, 
        btScalar distance, 
        int lifeTime, 
        const btVector3& color
      );
      virtual void reportErrorWarning(const char* warningString);
      virtual void draw3dText(
        const btVector3& location, 
        const char* textString
      );

      virtual void setDebugMode(int debug_mode);
      virtual int	 getDebugMode() const;

    private:
      glm::vec3 toGlm(const btVector3& v) const;

    private:
      int debug_mode_ = 0;
      platform::DebugRenderer* debug_renderer_;
    };
  }
}