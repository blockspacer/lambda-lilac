#pragma once
#include "systems/entity.h"
#include <glm/glm.hpp>
#include <containers/containers.h>
#include "physics_visualizer.h"
#include <memory/memory.h>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

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
      btDiscreteDynamicsWorld* dynamicsWorld();
	  Vector<Manifold> raycast(
		  const glm::vec3& start,
		  const glm::vec3& end
	  );
	  Vector<Manifold> collisionTest(
		  const entity::Entity& entity
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
      foundation::SharedPointer<btDefaultCollisionConfiguration> 
        collision_configuration_;
      
      foundation::SharedPointer<btSequentialImpulseConstraintSolver> 
        constraint_solver_;
      
      foundation::SharedPointer<components::RigidBodySystem> 
        rigid_body_system_;
      
      foundation::SharedPointer<btCollisionDispatcher> dispatcher_;
      foundation::SharedPointer<btBroadphaseInterface> pair_cache_;
	  foundation::SharedPointer<btDiscreteDynamicsWorld> dynamics_world_;
	  world::IWorld* world_;
      PhysicVisualizer physic_visualizer_;
	  bool debug_draw_enabled_;
    };
  }
}