#include "physics_world.h"
#include <btBulletDynamicsCommon.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include "systems/transform_system.h"
#include "systems/rigid_body_system.h"
#include "systems/entity_system.h"

namespace lambda
{
  namespace physics
  {
    ///////////////////////////////////////////////////////////////////////////
    struct PersonalRayCallback : public btCollisionWorld::RayResultCallback
    {
      // Inherited via RayResultCallback
      Vector<int> entities;

      virtual btScalar addSingleResult(
        btCollisionWorld::LocalRayResult& rayResult, 
        bool /*normalInWorldSpace*/
      ) override
      {
        if (rayResult.m_collisionObject != nullptr)
        {
          const btRigidBody* rb = btRigidBody::upcast(rayResult.m_collisionObject);
          entities.push_back(rb->getUserIndex());
        }
        return btScalar();
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    PhysicsWorld::PhysicsWorld()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    PhysicsWorld::~PhysicsWorld()
    {
      destroy();
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::initialize(
      platform::DebugRenderer* debug_renderer, 
      foundation::SharedPointer<entity::EntitySystem> entity_system, 
      foundation::SharedPointer<components::TransformSystem> transform_system,
      foundation::SharedPointer<components::RigidBodySystem> rigid_body_system)
    {
      physic_visualizer_.initialize(debug_renderer);
      entity_system_ = entity_system;
      transform_system_ = transform_system;
      rigid_body_system_ = rigid_body_system;
      initialize();
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::deinitialize()
    {
      destroy();
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::render()
    {
      dynamics_world_->debugDrawWorld();
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::update(const double& time_step)
    {
      for (int j = dynamics_world_->getNumCollisionObjects() - 1; j >= 0; --j)
      {
        btCollisionObject* object = 
          dynamics_world_->getCollisionObjectArray()[j];
        btRigidBody* rigid_body = btRigidBody::upcast(object);

        if (false == rigid_body->isStaticObject() && 
            true == rigid_body->isActive())
        {
          btTransform transform;

          // TODO (Hilze): Find out why this doesn't work
          /*if (rigid_body&& rigid_body->getMotionState())
          {
            rigid_body->getMotionState()->getWorldTransform(transform);

            const entity::Entity entity((uint64_t)rigid_body->getUserIndex(), 
              0u, entity_system_.get());
            glm::vec3 pos = transform_system_->getWorldTranslation(entity);
            glm::quat rot = transform_system_->getWorldRotation(entity);
            if (false == (std::isnan(pos.x) || 
              std::isnan(pos.y) || 
              std::isnan(pos.z)))
            {
              transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
            }
            if (false == (std::isnan(rot.x) || 
              std::isnan(rot.y) || 
              std::isnan(rot.z) || 
              std::isnan(rot.w)))
            {
              transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
            }

            rigid_body->getMotionState()->setWorldTransform(transform);
          }*/

          transform = object->getWorldTransform();
          const entity::Entity entity = 
            (uint32_t)rigid_body->getUserIndex();
          glm::vec3 pos = transform_system_->getWorldTranslation(entity);
          glm::quat rot = transform_system_->getWorldRotation(entity);
          if (false == (std::isnan(pos.x) || 
            std::isnan(pos.y) || 
            std::isnan(pos.z)))
          {
            transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
          }
          if (false == (std::isnan(rot.x) || 
            std::isnan(rot.y) || 
            std::isnan(rot.z) || 
            std::isnan(rot.w)))
          {
            transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
          }
          object->setWorldTransform(transform);

        }
      }

      dynamics_world_->stepSimulation((float)time_step, 1, (float)time_step);

      for (int j = dynamics_world_->getNumCollisionObjects() - 1; j >= 0; --j)
      {
        btCollisionObject* object =
          dynamics_world_->getCollisionObjectArray()[j];
        btRigidBody* rigid_body = btRigidBody::upcast(object);
        btTransform transform;

        if(rigid_body&& false == rigid_body->isStaticObject())
        {
          if (rigid_body->getMotionState())
          {
            rigid_body->getMotionState()->getWorldTransform(transform);
          }
          else
          {
            transform = object->getWorldTransform();
          }

          const entity::Entity entity =
            (uint32_t)rigid_body->getUserIndex();

          glm::quat rot(
            transform.getRotation().w(),
            transform.getRotation().x(),
            transform.getRotation().y(),
            transform.getRotation().z()
          );
          bool reset = false;
          if (false == (std::isnan(rot.x) || 
            std::isnan(rot.y) || 
            std::isnan(rot.z) || 
            std::isnan(rot.w)))
          {
            transform_system_->setWorldRotation(entity, rot);
          }
          else
          {
            reset = true;
            rot = transform_system_->getWorldRotation(entity);
            transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
          }

          // TODO (Hilze): Figure out why
          btTransform transform2 = object->getWorldTransform();
          glm::vec3 pos(
            transform2.getOrigin().x(),
            transform2.getOrigin().y(),
            transform2.getOrigin().z()
          );
          if (false == (std::isnan(pos.x) || 
            std::isnan(pos.y) || 
            std::isnan(pos.z)))
          {
            transform_system_->setWorldTranslation(entity, pos);
          }
          else
          {
            reset = true;
            pos = transform_system_->getWorldTranslation(entity);
            transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
          }

          if (reset == true)
          {
            rigid_body->forceActivationState(0);
            rigid_body->clearForces();
            rigid_body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
            rigid_body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
            rigid_body->updateInertiaTensor();
            rigid_body->setWorldTransform(transform);
          }

          glm::vec3 vel(
            rigid_body->getLinearVelocity().x(),
            rigid_body->getLinearVelocity().y(),
            rigid_body->getLinearVelocity().z()
          );

          glm::vec3 ang(
            rigid_body->getAngularVelocity().x(),
            rigid_body->getAngularVelocity().y(),
            rigid_body->getAngularVelocity().z()
          );
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::release()
    {
      for (int i = dynamics_world_->getNumCollisionObjects() - 1; i >= 0; --i)
      {
        btCollisionObject* object =
          dynamics_world_->getCollisionObjectArray()[i];
        btRigidBody* rigid_body = btRigidBody::upcast(object);

        if (rigid_body&& rigid_body->getMotionState())
        {
          foundation::Memory::deallocate(rigid_body->getMotionState());
        }
        dynamics_world_->removeCollisionObject(object);
        foundation::Memory::deallocate(object);
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    btDiscreteDynamicsWorld * PhysicsWorld::dynamicsWorld()
    {
      return dynamics_world_.get();
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<entity::Entity> PhysicsWorld::raycast(
      const glm::vec3& start, 
      const glm::vec3& end)
    {
      return Vector<entity::Entity>();
      PersonalRayCallback callback;
      dynamics_world_->rayTest(
        btVector3(start.x, start.y, start.z), 
        btVector3(end.x, end.y, end.z),
        callback
      );

      Vector<entity::Entity> entities(callback.entities.size());
      for (uint64_t i = 0u; i < entities.size(); ++i)
      {
				entities.at(i) = 
					(uint32_t)callback.entities.at(i);
      }
      
      return entities;
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::initialize()
    {
      collision_configuration_ = 
        foundation::Memory::constructShared<btDefaultCollisionConfiguration>();
      dispatcher_ = 
        foundation::Memory::constructShared<btCollisionDispatcher>(
          collision_configuration_.get()
          );
      pair_cache_ = foundation::Memory::constructShared<btDbvtBroadphase>();
      constraint_solver_ = 
        foundation::Memory::constructShared<
        btSequentialImpulseConstraintSolver
      >();
      dynamics_world_ = 
        foundation::Memory::constructShared<btDiscreteDynamicsWorld>(
          dispatcher_.get(), 
          pair_cache_.get(),
          constraint_solver_.get(),
          collision_configuration_.get()
          );
      dynamics_world_->setGravity(btVector3(0.0f, -9.81f, 0.0f));
      dynamics_world_->setDebugDrawer(&physic_visualizer_);

//#define RWSDEBUG // TODO (Hilze): Remove this!
#if defined(DEBUG) || defined(RWSDEBUG)
      physic_visualizer_.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
#endif

      // TODO (Hilze): Add Physics callbacks. 
      // Might require a rewrite of the scripting ecs.
      //gContactStartedCallback   = ContactStarted;
      //gContactEndedCallback     = ContactEnded;
      //gContactProcessedCallback = ContactProcessed;
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::destroy()
    {
      transform_system_.reset();
      rigid_body_system_.reset();
      entity_system_.reset();
      release();
    }
  }
}
