#pragma once
#include "systems/entity.h"
#include <glm/glm.hpp>
#include <containers/containers.h>
#include "physics_visualizer.h"
#include <memory/memory.h>

#if VIOLET_PHYSICS_BULLET
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
#endif

namespace lambda
{
  namespace components
  {
    class TransformSystem;
    class RigidBodySystem;
		class MonoBehaviourSystem;
  }

  namespace world
  {
	  class IWorld;
  }

  namespace platform
  {
	  class DebugRenderer;
  }

  namespace physics
  {
    ///////////////////////////////////////////////////////////////////////////
	struct Manifold
	{
	  entity::Entity entity;
	  glm::vec3 normal;
	  glm::vec3 point;
	};

    ///////////////////////////////////////////////////////////////////////////
    class PhysicsWorld
    {
    public:
      PhysicsWorld();
      ~PhysicsWorld();
      
      void initialize(
				platform::DebugRenderer* debug_renderer,
				world::IWorld* world
      );
      void deinitialize();
      void render();
      void update(const double& time_step);

      void release();
#if VIOLET_PHYSICS_BULLET
	  btDiscreteDynamicsWorld* dynamicsWorld();
#endif
	  Vector<Manifold> raycast(
		  const glm::vec3& start,
		  const glm::vec3& end
	  );

	  void setDebugDrawEnabled(bool debug_draw_enabled);
	  bool getDebugDrawEnabled() const;

	  void setGravity(glm::vec3 gravity);
	  glm::vec3 getGravity() const;

	  components::TransformSystem* getTransformSystem() const;

    private:
      void initialize();
      void destroy();

    private:
#if VIOLET_PHYSICS_BULLET
		foundation::SharedPointer<btDefaultCollisionConfiguration>
        collision_configuration_;
      
      foundation::SharedPointer<btSequentialImpulseConstraintSolver> 
        constraint_solver_;
#endif
      
      foundation::SharedPointer<components::RigidBodySystem> 
        rigid_body_system_;
      
#if VIOLET_PHYSICS_BULLET
	  foundation::SharedPointer<btCollisionDispatcher> dispatcher_;
      foundation::SharedPointer<btBroadphaseInterface> pair_cache_;
	  foundation::SharedPointer<btDiscreteDynamicsWorld> dynamics_world_;
#endif
	  world::IWorld* world_;
#if VIOLET_PHYSICS_BULLET
      PhysicVisualizer physic_visualizer_;
#endif
	  bool debug_draw_enabled_;
    };
  }
}