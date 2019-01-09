#include "physics_visualizer.h"
#include "platform/debug_renderer.h"
#include <utils/console.h>

namespace lambda
{
  namespace physics
  {
    ///////////////////////////////////////////////////////////////////////////
    void PhysicVisualizer::initialize(platform::DebugRenderer* debug_renderer)
    {
      debug_renderer_ = debug_renderer;
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicVisualizer::drawLine(
      const btVector3& from, 
      const btVector3& to, 
      const btVector3& fromColor, 
      const btVector3& toColor)
    {
      debug_renderer_->DrawLine(platform::DebugLine(
        toGlm(from),
        toGlm(to),
        glm::vec4(toGlm(fromColor), 1.0f),
        glm::vec4(toGlm(toColor), 1.0f)
      )); 
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicVisualizer::drawLine(
      const btVector3& from, 
      const btVector3& to, 
      const btVector3& color)
    {
      debug_renderer_->DrawLine(platform::DebugLine(
        toGlm(from),
        toGlm(to),
        glm::vec4(toGlm(color), 1.0f)
      ));
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicVisualizer::drawSphere(
      const btVector3& /*p*/,
      btScalar /*radius*/,
      const btVector3& /*color*/)
    {
      assert(false);
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicVisualizer::drawTriangle(
      const btVector3& a,
      const btVector3& b, 
      const btVector3& c, 
      const btVector3& color, 
      btScalar alpha)
    {
      debug_renderer_->DrawLine(
        platform::DebugLine(toGlm(a), toGlm(b), glm::vec4(toGlm(color), alpha))
      );
      
      debug_renderer_->DrawLine(
        platform::DebugLine(toGlm(b), toGlm(c), glm::vec4(toGlm(color), alpha))
      );
      
      debug_renderer_->DrawLine(
        platform::DebugLine(toGlm(c), toGlm(a), glm::vec4(toGlm(color), alpha))
      );
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicVisualizer::drawContactPoint(
      const btVector3& /*PointOnB*/,
      const btVector3& /*normalOnB*/,
      btScalar /*distance*/,
      int /*lifeTime*/,
      const btVector3& /*color*/)
    {
      LMB_ASSERT(false, "PHYSICS: DrawContactPoint not implemented");
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicVisualizer::reportErrorWarning(const char* warningString)
    {
      foundation::Error("Physics: " + String(warningString) + "\n");
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicVisualizer::draw3dText(
      const btVector3& /*location*/,
      const char* /*textString*/)
    {
      LMB_ASSERT(false, "PHYSICS: Draw3DText not implemented");
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicVisualizer::setDebugMode(int debug_mode) 
    { 
      debug_mode_ = debug_mode; 
    }

    ///////////////////////////////////////////////////////////////////////////
    int PhysicVisualizer::getDebugMode() const
    { 
      return debug_mode_; 
    }

    ///////////////////////////////////////////////////////////////////////////
    glm::vec3 PhysicVisualizer::toGlm(const btVector3& v) const
    {
      return glm::vec3(v.x(), v.y(), v.z()); 
    }
  }
}
