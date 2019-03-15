#include "bullet_physics_visualizer.h"
#include "platform/debug_renderer.h"
#include <utils/console.h>
#include <interfaces/iphysics.h>

namespace lambda
{
  namespace physics
  {
    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicVisualizer::initialize(platform::DebugRenderer* debug_renderer)
    {
      debug_renderer_ = debug_renderer;
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicVisualizer::drawLine(
      const btVector3& from, 
      const btVector3& to, 
      const btVector3& fromColor, 
      const btVector3& toColor)
    {
      debug_renderer_->DrawLine(platform::DebugLine(
        toGlm(from) * VIOLET_INV_PHYSICS_SCALE,
        toGlm(to) * VIOLET_INV_PHYSICS_SCALE,
        glm::vec4(toGlm(fromColor), 1.0f),
        glm::vec4(toGlm(toColor), 1.0f)
      )); 
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicVisualizer::drawLine(
      const btVector3& from, 
      const btVector3& to, 
      const btVector3& color)
    {
      debug_renderer_->DrawLine(platform::DebugLine(
        toGlm(from) * VIOLET_INV_PHYSICS_SCALE,
        toGlm(to) * VIOLET_INV_PHYSICS_SCALE,
        glm::vec4(toGlm(color), 1.0f)
      ));
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicVisualizer::drawSphere(
      const btVector3& /*p*/,
      btScalar /*radius*/,
      const btVector3& /*color*/)
    {
		LMB_ASSERT(false, "PHYSICS: drawSphere not implemented");
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicVisualizer::drawTriangle(
      const btVector3& a,
      const btVector3& b, 
      const btVector3& c, 
      const btVector3& color, 
      btScalar alpha)
    {
      debug_renderer_->DrawLine(
        platform::DebugLine(toGlm(a) * VIOLET_INV_PHYSICS_SCALE, toGlm(b) * VIOLET_INV_PHYSICS_SCALE, glm::vec4(toGlm(color), alpha))
      );
      
      debug_renderer_->DrawLine(
        platform::DebugLine(toGlm(b) * VIOLET_INV_PHYSICS_SCALE, toGlm(c) * VIOLET_INV_PHYSICS_SCALE, glm::vec4(toGlm(color), alpha))
      );
      
      debug_renderer_->DrawLine(
        platform::DebugLine(toGlm(c) * VIOLET_INV_PHYSICS_SCALE, toGlm(a) * VIOLET_INV_PHYSICS_SCALE, glm::vec4(toGlm(color), alpha))
      );
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicVisualizer::drawContactPoint(
      const btVector3& /*PointOnB*/,
      const btVector3& /*normalOnB*/,
      btScalar /*distance*/,
      int /*lifeTime*/,
      const btVector3& /*color*/)
    {
      LMB_ASSERT(false, "PHYSICS: DrawContactPoint not implemented");
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicVisualizer::reportErrorWarning(const char* warningString)
    {
      foundation::Error("Physics: " + String(warningString) + "\n");
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicVisualizer::draw3dText(
      const btVector3& /*location*/,
      const char* /*textString*/)
    {
      LMB_ASSERT(false, "PHYSICS: Draw3DText not implemented");
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicVisualizer::setDebugMode(int debug_mode) 
    { 
      debug_mode_ = debug_mode; 
    }

    ///////////////////////////////////////////////////////////////////////////
    int BulletPhysicVisualizer::getDebugMode() const
    { 
      return debug_mode_; 
    }

    ///////////////////////////////////////////////////////////////////////////
    glm::vec3 BulletPhysicVisualizer::toGlm(const btVector3& v) const
    {
      return glm::vec3(v.x(), v.y(), v.z()); 
    }
  }
}
