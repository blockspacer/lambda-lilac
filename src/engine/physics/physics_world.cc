#include "physics_world.h"
#include <btBulletDynamicsCommon.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include "systems/transform_system.h"
#include "systems/rigid_body_system.h"
#include "systems/entity_system.h"
#include "systems/mono_behaviour_system.h"
#include "interfaces/iworld.h"

namespace lambda
{
  namespace physics
  {
        components::MonoBehaviourSystem* g_monoBehaviourSystem;

        ///////////////////////////////////////////////////////////////////////////
        void ContactStarted(btPersistentManifold*const& manifold)
        {
            const auto* body0 = manifold->getBody0();
            const auto* body1 = manifold->getBody1();

            const auto& bt_normal = 
                (manifold->getNumContacts() <= 0) ? btVector3(0.0f, 0.0f, 0.0f) : 
                manifold->getContactPoint(0).m_normalWorldOnB;
            const glm::vec3& normal =
                glm::vec3(bt_normal.x(), bt_normal.y(), bt_normal.z());

            const entity::Entity lhs = body0->getUserIndex();
            const entity::Entity rhs = body1->getUserIndex();

            bool is_trigger = 
                (body0->getCollisionFlags() & 
                    btCollisionObject::CF_NO_CONTACT_RESPONSE +
                body1->getCollisionFlags() & 
                    btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0 ? true : false;

            if (is_trigger)
                g_monoBehaviourSystem->onTriggerEnter(lhs, rhs, normal);
            else
                g_monoBehaviourSystem->onCollisionEnter(lhs, rhs, normal);
        }

        ///////////////////////////////////////////////////////////////////////////
        void ContactEnded(btPersistentManifold*const& manifold)
        {
            const auto* body0 = manifold->getBody0();
            const auto* body1 = manifold->getBody1();

            const auto& bt_normal = 
                (manifold->getNumContacts() <= 0) ? btVector3(0.0f, 0.0f, 0.0f) : 
                manifold->getContactPoint(0).m_normalWorldOnB;
            const glm::vec3& normal = 
                glm::vec3(bt_normal.x(), bt_normal.y(), bt_normal.z());

            const entity::Entity lhs = body0->getUserIndex();
            const entity::Entity rhs = body1->getUserIndex();

            bool is_trigger =
                (body0->getCollisionFlags() &
                    btCollisionObject::CF_NO_CONTACT_RESPONSE +
                    body1->getCollisionFlags() &
                    btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0 ? true : false;

            if (is_trigger)
                g_monoBehaviourSystem->onTriggerExit(lhs, rhs, normal);
            else
                g_monoBehaviourSystem->onCollisionExit(lhs, rhs, normal);
        }

        ///////////////////////////////////////////////////////////////////////////
        bool ContactProcessed(btManifoldPoint& cp, void* body0, void* body1)
        {
            const auto* _body0 = (btCollisionObject*)body0;
            const auto* _body1 = (btCollisionObject*)body1;

            const auto& bt_normal = cp.m_normalWorldOnB;
            const glm::vec3& normal =
                glm::vec3(bt_normal.x(), bt_normal.y(), bt_normal.z());

            const entity::Entity lhs = _body0->getUserIndex();
            const entity::Entity rhs = _body1->getUserIndex();

            bool is_trigger =
                (_body0->getCollisionFlags() &
                    btCollisionObject::CF_NO_CONTACT_RESPONSE +
                    _body1->getCollisionFlags() &
                    btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0 ? true : false;

            //Scripting::ScriptBinding::get()->ContactProcessed(lhs, rhs, normal, is_trigger);
            return false;
        }


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
#if defined(_DEBUG) || defined(DEBUG)
		: debug_draw_enabled_(true)
#else
		: debug_draw_enabled_(false)
#endif
    {
		physic_visualizer_.setDebugMode(
			debug_draw_enabled_ ? 
			btIDebugDraw::DBG_DrawWireframe : 
			btIDebugDraw::DBG_NoDebug
		);
    }

    ///////////////////////////////////////////////////////////////////////////
    PhysicsWorld::~PhysicsWorld()
    {
      destroy();
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::initialize(
      platform::DebugRenderer* debug_renderer, 
	  world::IWorld* world)
    {
	  world_ = world;
      physic_visualizer_.initialize(debug_renderer);
      g_monoBehaviourSystem = world_->getScene().getSystem<components::MonoBehaviourSystem>().get();
      initialize();
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::deinitialize()
    {
      g_monoBehaviourSystem = nullptr;
      destroy();
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::render()
    {
      dynamics_world_->debugDrawWorld();
    }

	///////////////////////////////////////////////////////////////////////////
	inline bool isNaN(const glm::vec3& v)
	{
		return (std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z));
	}

	///////////////////////////////////////////////////////////////////////////
	inline bool isNotNaN(const glm::vec3& v)
	{
		return (!std::isnan(v.x) && !std::isnan(v.y) && !std::isnan(v.z));
	}

	///////////////////////////////////////////////////////////////////////////
	inline bool isNaN(const glm::quat& v)
	{
		return (std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z) || std::isnan(v.w));
	}

	///////////////////////////////////////////////////////////////////////////
	inline bool isNotNaN(const glm::quat& v)
	{
		return (!std::isnan(v.x) && !std::isnan(v.y) && !std::isnan(v.z) && !std::isnan(v.w));
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

		  const entity::Entity entity = rigid_body->getUserIndex();
		  bool activate = false;

		  {
			  transform = rigid_body->getWorldTransform();

			  glm::vec3 pos = getTransformSystem()->getWorldTranslation(entity);
			  glm::quat rot = getTransformSystem()->getWorldRotation(entity);

			  if (isNotNaN(pos))
			  {
				  const btVector3 bt_pos(pos.x, pos.y, pos.z);
				  if (transform.getOrigin() != bt_pos)
				  {
					  transform.setOrigin(bt_pos);
					  activate = true;
				  }
			  }

			  if (isNotNaN(rot))
			  {
				  const btQuaternion bt_rot(rot.x, rot.y, rot.z, rot.w);
				  if (transform.getRotation() != bt_rot)
				  {
					  transform.setRotation(bt_rot);
					  activate = true;
				  }
			  }

			  rigid_body->setWorldTransform(transform);
			  rigid_body->getMotionState()->setWorldTransform(transform);
		  }

		  if (activate)
			  object->activate();

		  //*/
        }
      }

      dynamics_world_->stepSimulation((float)time_step, 1, (float)time_step);

      for (int j = dynamics_world_->getNumCollisionObjects() - 1; j >= 0; --j)
      {
        btCollisionObject* object =
          dynamics_world_->getCollisionObjectArray()[j];
        btRigidBody* rigid_body = btRigidBody::upcast(object);
        btTransform transform;

        if(rigid_body && false == rigid_body->isStaticObject())
        {
          transform = rigid_body->getCenterOfMassTransform();
          const entity::Entity entity = rigid_body->getUserIndex();

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
            getTransformSystem()->setWorldRotation(entity, rot);
          }
          else
          {
            reset = true;
            rot = getTransformSystem()->getWorldRotation(entity);
            transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
          }

          // TODO (Hilze): Figure out why
          glm::vec3 pos(
            transform.getOrigin().x(),
            transform.getOrigin().y(),
            transform.getOrigin().z()
          );
          if (false == (std::isnan(pos.x) || 
            std::isnan(pos.y) || 
            std::isnan(pos.z)))
          {
            getTransformSystem()->setWorldTranslation(entity, pos);
          }
          else
          {
            reset = true;
            pos = getTransformSystem()->getWorldTranslation(entity);
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
			rigid_body->getMotionState()->setWorldTransform(transform);
		  }
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
    Vector<Manifold> PhysicsWorld::raycast(
      const glm::vec3& start, 
      const glm::vec3& end)
    {
	  btVector3 f(start.x, start.y, start.z);
	  btVector3 t(end.x, end.y, end.z);
	  
	  btCollisionWorld::AllHitsRayResultCallback callback(f, t);
      dynamics_world_->rayTest(f, t, callback);


      Vector<Manifold> manifolds(callback.m_hitFractions.size());
	  for (int i = 0; i < (int)manifolds.size(); ++i)
	  {
		  manifolds[i].entity = static_cast<entity::Entity>(callback.m_collisionObjects[i]->getUserIndex());
		  manifolds[i].point  = glm::vec3(callback.m_hitPointWorld[i].x(), callback.m_hitPointWorld[i].y(), callback.m_hitPointWorld[i].z());
		  manifolds[i].normal = glm::vec3(callback.m_hitNormalWorld[i].x(), callback.m_hitNormalWorld[i].y(), callback.m_hitNormalWorld[i].z());
	  }

      return manifolds;
    }

	struct MyContactCallBack : public btCollisionWorld::ContactResultCallback
	{
		virtual btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, 
			int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override
		{
			btVector3 ptA = cp.getPositionWorldOnA();
			btVector3 ptB = cp.getPositionWorldOnB();
			return 0;
		}
	};

	///////////////////////////////////////////////////////////////////////////
	void physics::PhysicsWorld::setDebugDrawEnabled(bool debug_draw_enabled)
	{
		debug_draw_enabled_ = debug_draw_enabled;

		physic_visualizer_.setDebugMode(
			debug_draw_enabled_ ?
			btIDebugDraw::DBG_DrawWireframe :
			btIDebugDraw::DBG_NoDebug
		);
	}

	///////////////////////////////////////////////////////////////////////////
	bool physics::PhysicsWorld::getDebugDrawEnabled() const
	{
		return debug_draw_enabled_;
	}

	///////////////////////////////////////////////////////////////////////////
	void physics::PhysicsWorld::setGravity(glm::vec3 gravity)
	{
		dynamics_world_->setGravity(btVector3(
			gravity.x,
			gravity.y,
			gravity.z
		));
	}

	///////////////////////////////////////////////////////////////////////////
	glm::vec3 physics::PhysicsWorld::getGravity() const
	{
		btVector3 gravity = dynamics_world_->getGravity();
		return glm::vec3(gravity.x(), gravity.y(), gravity.z());
	}

	///////////////////////////////////////////////////////////////////////////
	components::TransformSystem* physics::PhysicsWorld::getTransformSystem() const
	{
		return world_->getScene().getSystem<components::TransformSystem>().get();
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

	  physic_visualizer_.setDebugMode(
		  debug_draw_enabled_ ?
		  btIDebugDraw::DBG_DrawWireframe :
		  btIDebugDraw::DBG_NoDebug
	  );

      // TODO (Hilze): Add Physics callbacks. 
      // Might require a rewrite of the scripting ecs.
      gContactStartedCallback   = ContactStarted;
      gContactEndedCallback     = ContactEnded;
      gContactProcessedCallback = ContactProcessed;
    }

    ///////////////////////////////////////////////////////////////////////////
    void PhysicsWorld::destroy()
    {
	  world_ = nullptr;
      release();
    }
  }
}
