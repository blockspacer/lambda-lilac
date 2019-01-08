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
  }

  namespace physics
  {
    ///////////////////////////////////////////////////////////////////////////
    class PhysicsWorld
    {
    public:
      PhysicsWorld();
      ~PhysicsWorld();
      
      void initialize(
       platform::DebugRenderer* debug_renderer,
       foundation::SharedPointer<entity::EntitySystem> entity_system,
       foundation::SharedPointer<components::TransformSystem> transform_system,
       foundation::SharedPointer<components::RigidBodySystem> rigid_body_system
      );
      void deinitialize();
      void render();
      void update(const double& time_step);

      void release();
      btDiscreteDynamicsWorld* dynamicsWorld();
      Vector<entity::Entity> raycast(
        const glm::vec3& start, 
        const glm::vec3& end
      );

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
      foundation::SharedPointer<entity::EntitySystem>    entity_system_;
      foundation::SharedPointer<components::TransformSystem> transform_system_;
      PhysicVisualizer physic_visualizer_;
    };
  }
}