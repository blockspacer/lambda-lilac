#include "bullet_physics_world.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include "systems/transform_system.h"
#include "systems/rigid_body_system.h"
#include "systems/entity_system.h"
#include "systems/mono_behaviour_system.h"
#include "interfaces/iworld.h"
#include <containers/containers.h>

#include <btBulletDynamicsCommon.h>

namespace lambda
{
  namespace physics
  {
	  template<typename T>
	  bool is_infinite(const T &value)
	  {
		  T max_value = std::numeric_limits<T>::max();
		  T min_value = -max_value;

		  return !(min_value <= value && value <= max_value);
	  }

	  template<typename T>
	  bool is_nan(const T &value)
	  {
		  return value != value;
	  }

	  template<typename T>
	  bool is_valid(const T &value)
	  {
		  return !is_infinite(value) && !is_nan(value);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  btVector3 toBt(glm::vec3 v)
	  {
		  return btVector3(v.x, v.y, v.z);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  glm::vec3 toGlm(btVector3 v)
	  {
		  return glm::vec3(v.x(), v.y(), v.z());
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  btQuaternion toBt(glm::quat q)
	  {
		  return btQuaternion(q.x, q.y, q.z, q.w);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  glm::quat toGlm(btQuaternion q)
	  {
		  return glm::quat(q.w(), q.x(), q.y(), q.z());
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  inline static bool isValid(const btVector3& v)
	  {
		  return is_valid(v.x()) && is_valid(v.y()) && is_valid(v.z());
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  inline static bool isValid(const btQuaternion& v)
	  {
		  return is_valid(v.x()) && is_valid(v.y()) && is_valid(v.z()) && is_valid(v.w());
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  inline static bool isValid(const glm::vec3& v)
	  {
		  return is_valid(v.x) && is_valid(v.y) && is_valid(v.z);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  inline static bool isValid(const glm::quat& v)
	  {
		  return is_valid(v.x) && is_valid(v.y) && is_valid(v.z) && is_valid(v.w);
	  }





	  ///////////////////////////////////////////////////////////////////////////
	  BulletCollisionBody::BulletCollisionBody(
		  world::IWorld* world,
		  btDiscreteDynamicsWorld* dynamics_world,
		  BulletPhysicsWorld* physics_world,
		  entity::Entity entity)
		  : physics_world_(physics_world)
		  , dynamics_world_(dynamics_world)
		  , world_(world)
		  , type_(BulletCollisionBodyType::kNone)
		  , entity_(entity)
		  , velocity_constraints_(0)
		  , angular_constraints_(0)
		  , layers_(-1)
		  , body_(nullptr)
		  , motion_state_(nullptr)
		  , collision_shape_(nullptr)
		  , triangle_mesh_(nullptr)
		  , mass_(1.0f)
	  {
		  collision_shape_ = foundation::Memory::construct<btEmptyShape>();
		  createBody();
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  BulletCollisionBody::~BulletCollisionBody()
	  {
		  destroyBody();
		  foundation::Memory::destruct(collision_shape_);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  glm::vec3 BulletCollisionBody::getPosition() const
	  {
		  return toGlm(body_->getWorldTransform().getOrigin());
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::setPosition(glm::vec3 position)
	  {
		  auto transform = body_->getWorldTransform();
		  transform.setOrigin(toBt(position));
		  body_->setWorldTransform(transform);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  glm::quat BulletCollisionBody::getRotation() const
	  {
		  return toGlm(body_->getWorldTransform().getRotation());
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::setRotation(glm::quat rotation)
	  {
		  auto transform = body_->getWorldTransform();
		  transform.setRotation(toBt(rotation));
		  body_->setWorldTransform(transform);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  entity::Entity BulletCollisionBody::getEntity() const
	  {
		  return entity_;
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  float BulletCollisionBody::getFriction() const
	  {
		  return body_->getFriction();
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::setFriction(float friction)
	  {
		  body_->setFriction((btScalar)friction);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  float BulletCollisionBody::getMass() const
	  {
		  return mass_;
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::setMass(float mass)
	  {
		  mass_ = mass;

		  if (type_ == BulletCollisionBodyType::kCollider)
		  {
			  body_->setMassProps(0.0f, btVector3(0.0f, 0.0f, 0.0f));
		  }
		  else if (type_ == BulletCollisionBodyType::kRigidBody)
		  {
			  btVector3 inertia;
			  collision_shape_->calculateLocalInertia(mass_, inertia);
			  body_->setMassProps(mass_, inertia);
		  }
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  uint16_t BulletCollisionBody::getLayers() const
	  {
		  return layers_;
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::setLayers(uint16_t layers)
	  {
		  layers_ = layers;
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  uint8_t BulletCollisionBody::getVelocityConstraints() const
	  {
		  return velocity_constraints_;
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::setVelocityConstraints(uint8_t velocity_constraints)
	  {
		  velocity_constraints_ = velocity_constraints;

		  btVector3 v(
			  (velocity_constraints_ & (uint8_t)components::RigidBodyConstraints::kX) != 0 ? 0.0f : 1.0f,
			  (velocity_constraints_ & (uint8_t)components::RigidBodyConstraints::kY) != 0 ? 0.0f : 1.0f,
			  (velocity_constraints_ & (uint8_t)components::RigidBodyConstraints::kZ) != 0 ? 0.0f : 1.0f
		  );
		  body_->setLinearFactor(v);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  uint8_t BulletCollisionBody::getAngularConstraints() const
	  {
		  return angular_constraints_;
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::setAngularConstraints(uint8_t angular_constraints)
	  {
		  angular_constraints_ = angular_constraints;

		  btVector3 v(
			  (angular_constraints_ & (uint8_t)components::RigidBodyConstraints::kX) != 0 ? 0.0f : 1.0f,
			  (angular_constraints_ & (uint8_t)components::RigidBodyConstraints::kY) != 0 ? 0.0f : 1.0f,
			  (angular_constraints_ & (uint8_t)components::RigidBodyConstraints::kZ) != 0 ? 0.0f : 1.0f
		  );
		  body_->setAngularFactor(v);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  glm::vec3 BulletCollisionBody::getVelocity() const
	  {
		  return toGlm(body_->getLinearVelocity());
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::setVelocity(glm::vec3 velocity)
	  {
		  body_->setLinearVelocity(toBt(velocity));
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  glm::vec3 BulletCollisionBody::getAngularVelocity() const
	  {
		  return toGlm(body_->getAngularVelocity());
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::setAngularVelocity(glm::vec3 velocity)
	  {
		  body_->setAngularVelocity(toBt(velocity));
		  body_->activate();
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::applyImpulse(glm::vec3 impulse)
	  {
		  body_->applyCentralImpulse(toBt(impulse));
		  body_->activate();
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::applyImpulse(glm::vec3 impulse, glm::vec3 location)
	  {
		  body_->applyImpulse(toBt(impulse), toBt(location));
		  body_->activate();
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  bool BulletCollisionBody::isCollider() const
	  {
		  return type_ == BulletCollisionBodyType::kCollider;
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  bool BulletCollisionBody::isRigidBody() const
	  {
		  return type_ == BulletCollisionBodyType::kRigidBody;
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::makeBoxCollider()
	  {
		  glm::vec3 scale = world_->getScene().getSystem<components::TransformSystem>()->getWorldScale(entity_);
		  btVector3 half_extends(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f);
		  btCollisionShape* shape = foundation::Memory::construct<btBoxShape>(half_extends);
		  makeShape(shape);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::makeSphereCollider()
	  {
		  glm::vec3 scale = world_->getScene().getSystem<components::TransformSystem>()->getWorldScale(entity_);
		  btScalar radius = (scale.x + scale.z) / 4.0f;
		  btCollisionShape* shape = foundation::Memory::construct<btSphereShape>(radius);
		  makeShape(shape);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::makeCapsuleCollider()
	  {
		  glm::vec3 scale = world_->getScene().getSystem<components::TransformSystem>()->getWorldScale(entity_);
		  btScalar radius = (scale.x * 0.5f + scale.z * 0.5f) * 0.5f;
		  btScalar height = scale.y * 0.5f;
		  btCollisionShape* shape = foundation::Memory::construct<btCapsuleShape>(radius, height);
		  makeShape(shape);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::makeMeshCollider(asset::MeshHandle mesh, uint32_t sub_mesh_id)
	  {
		  // Get the indices.
		  glm::vec3 scale = world_->getScene().getSystem<components::TransformSystem>()->getWorldScale(entity_);
		  asset::SubMesh sub_mesh = mesh->getSubMeshes().at(sub_mesh_id);
		  auto index_offset = sub_mesh.offsets[asset::MeshElements::kIndices];
		  auto vertex_offset = sub_mesh.offsets[asset::MeshElements::kPositions];
		  int* indices = (int*)foundation::Memory::allocate(index_offset.count * sizeof(int));
		  glm::vec3* vertices = (glm::vec3*)foundation::Memory::allocate(vertex_offset.count * sizeof(glm::vec3));

		  auto mii = mesh->get(asset::MeshElements::kIndices);
		  auto mpi = mesh->get(asset::MeshElements::kPositions);

		  memcpy(vertices, (char*)mpi.data + vertex_offset.offset, vertex_offset.count * mpi.size);
		  for (uint32_t i = 0; i < vertex_offset.count; ++i)
			  vertices[i] *= scale;


		  if (sizeof(uint16_t) == mii.size)
		  {
			  Vector<uint16_t> idx(index_offset.count);
			  memcpy(idx.data(), (char*)mii.data + index_offset.offset, index_offset.count * mii.size);

			  for (size_t i = 0u; i < index_offset.count; ++i)
				  indices[i] = (int)idx.at(i);
		  }
		  else
		  {
			  Vector<uint32_t> idx(index_offset.count);
			  memcpy(idx.data(), (char*)mii.data + index_offset.offset, index_offset.count * mii.size);

			  for (size_t i = 0u; i < index_offset.count; ++i)
				  indices[i] = (int)idx.at(i);
		  }


		  auto* triangle_mesh = foundation::Memory::construct<btTriangleMesh>(false, false);

		  for (uint32_t i = 0; i < index_offset.count; i += 3)
		  {
			  auto v1 = vertices[indices[i + 0]];
			  auto v2 = vertices[indices[i + 1]];
			  auto v3 = vertices[indices[i + 2]];
			  triangle_mesh->addTriangle(
				  btVector3(v1.x, v1.y, v1.z),
				  btVector3(v2.x, v2.y, v2.z),
				  btVector3(v3.x, v3.y, v3.z),
				  false
			  );
		  }

		  btCollisionShape* shape = foundation::Memory::construct<btBvhTriangleMeshShape>(triangle_mesh, false);
		  makeShape(shape, triangle_mesh);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::makeShape(btCollisionShape* shape, btTriangleMesh* triangle_mesh)
	  {
		  if (collision_shape_)
			  foundation::Memory::destruct(collision_shape_);
		  if (triangle_mesh_)
			  foundation::Memory::destruct(triangle_mesh_);
		  triangle_mesh_ = triangle_mesh;

		  collision_shape_ = shape;
		  body_->setCollisionShape(collision_shape_);

		  setMass(mass_);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::makeCollider()
	  {
		  createBody();

		  type_ = BulletCollisionBodyType::kCollider;

		  body_->setCollisionFlags(btRigidBody::CF_STATIC_OBJECT);
		  btCollisionObject* collision_object = (btCollisionObject*)body_;
		  collision_object->setCollisionFlags(btRigidBody::CF_STATIC_OBJECT);
		  body_->setMassProps(0.0f, btVector3(0.0f, 0.0f, 0.0f));

		  dynamics_world_->addCollisionObject(body_);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::makeRigidBody()
	  {
		  createBody();

			type_ = BulletCollisionBodyType::kRigidBody;

			btVector3 inertia;
			collision_shape_->calculateLocalInertia(mass_, inertia);
			body_->setMassProps(mass_, inertia);
			body_->setCollisionFlags(0);
			btCollisionObject* collision_object = (btCollisionObject*)body_;
			collision_object->setCollisionFlags(0);
			setVelocityConstraints(velocity_constraints_);
			setAngularConstraints(angular_constraints_);

			dynamics_world_->addRigidBody(body_);
	  }

	  ///////////////////////////////////////////////////////////////////////////
	  void BulletCollisionBody::destroyBody()
	  {
		  if (body_)
		  {
			  if (type_ == BulletCollisionBodyType::kCollider)
				  dynamics_world_->removeCollisionObject(body_);
			  if (type_ == BulletCollisionBodyType::kRigidBody)
				  dynamics_world_->removeRigidBody(body_);
		  }
		  type_ = BulletCollisionBodyType::kNone;

		  if (motion_state_)
			  foundation::Memory::destruct(motion_state_);
		  motion_state_ = nullptr;
		  if (body_)
			  foundation::Memory::destruct(body_);
		  body_ = nullptr;
	  }

	  void BulletCollisionBody::createBody()
	  {
		  destroyBody();

		  glm::quat rotation = world_->getScene().getSystem<components::TransformSystem>()->getWorldRotation(entity_);
		  glm::vec3 translation = world_->getScene().getSystem<components::TransformSystem>()->getWorldTranslation(entity_);

		  motion_state_ = foundation::Memory::construct<btDefaultMotionState>(btTransform(toBt(rotation), toBt(translation)));
		  btRigidBody::btRigidBodyConstructionInfo rigid_body_ci(
			  /*mass*/         mass_,
			  /*motion_state*/ motion_state_,
			  /*shape*/        collision_shape_,
			  /*inertia*/      btVector3(0.0f, 0.0f, 0.0f)
		  );
		  body_ = foundation::Memory::construct<btRigidBody>(rigid_body_ci);
		  body_->setUserPointer(this);
	  }




	  ///////////////////////////////////////////////////////////////////////////
	  class MyCollisionDispatcher : public btCollisionDispatcher
	  {
	  public:
		  MyCollisionDispatcher(btCollisionConfiguration* collisionConfiguration)
			  : btCollisionDispatcher(collisionConfiguration)
		  {}

		  virtual bool needsCollision(const btCollisionObject* body0, const btCollisionObject* body1) override
		  {
			  auto up0 = body0->getUserPointer();
			  auto up1 = body1->getUserPointer();
			  BulletCollisionBody* bcb0 = (BulletCollisionBody*)body0->getUserPointer();
			  BulletCollisionBody* bcb1 = (BulletCollisionBody*)body1->getUserPointer();
			  int layers0 = bcb0->getLayers();
			  int layers1 = bcb1->getLayers();

			  if ((layers0 & layers1) == 0)
				  return false;

			  return btCollisionDispatcher::needsCollision(body0, body1);
		  }

		  virtual bool needsResponse(const btCollisionObject* body0, const btCollisionObject* body1) override
		  {
			  BulletCollisionBody* bcb0 = (BulletCollisionBody*)body0->getUserPointer();
			  BulletCollisionBody* bcb1 = (BulletCollisionBody*)body1->getUserPointer();
			  int layers0 = bcb0->getLayers();
			  int layers1 = bcb1->getLayers();

			  if ((layers0 & layers1) == 0)
				  return false;

			  return btCollisionDispatcher::needsResponse(body0, body1);
		  }
	  };

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

			BulletCollisionBody* lhs = (BulletCollisionBody*)body0->getUserPointer();
			BulletCollisionBody* rhs = (BulletCollisionBody*)body1->getUserPointer();

            bool is_trigger = 
                (body0->getCollisionFlags() & 
                    btCollisionObject::CF_NO_CONTACT_RESPONSE +
                body1->getCollisionFlags() & 
                    btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0 ? true : false;

            if (is_trigger)
                g_monoBehaviourSystem->onTriggerEnter(lhs->getEntity(), rhs->getEntity(), normal);
            else
                g_monoBehaviourSystem->onCollisionEnter(lhs->getEntity(), rhs->getEntity(), normal);
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

			BulletCollisionBody* lhs = (BulletCollisionBody*)body0->getUserPointer();
			BulletCollisionBody* rhs = (BulletCollisionBody*)body1->getUserPointer(); 
			
            bool is_trigger =
                (body0->getCollisionFlags() &
                    btCollisionObject::CF_NO_CONTACT_RESPONSE +
                    body1->getCollisionFlags() &
                    btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0 ? true : false;

            if (is_trigger)
                g_monoBehaviourSystem->onTriggerExit(lhs->getEntity(), rhs->getEntity(), normal);
            else
                g_monoBehaviourSystem->onCollisionExit(lhs->getEntity(), rhs->getEntity(), normal);
        }

        ///////////////////////////////////////////////////////////////////////////
        bool ContactProcessed(btManifoldPoint& cp, void* body0, void* body1)
        {
            const auto* _body0 = (btCollisionObject*)body0;
            const auto* _body1 = (btCollisionObject*)body1;

            const auto& bt_normal = cp.m_normalWorldOnB;
            const glm::vec3& normal =
                glm::vec3(bt_normal.x(), bt_normal.y(), bt_normal.z());

			BulletCollisionBody* lhs = (BulletCollisionBody*)_body0->getUserPointer();
			BulletCollisionBody* rhs = (BulletCollisionBody*)_body1->getUserPointer();

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
      Vector<entity::Entity> entities;

      virtual btScalar addSingleResult(
        btCollisionWorld::LocalRayResult& rayResult, 
        bool /*normalInWorldSpace*/
      ) override
      {
        if (rayResult.m_collisionObject != nullptr)
        {
          const btRigidBody* rb = btRigidBody::upcast(rayResult.m_collisionObject);
		  BulletCollisionBody* lhs = (BulletCollisionBody*)rb->getUserPointer();
		  entities.push_back(lhs->getEntity());
        }
        return btScalar();
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    BulletPhysicsWorld::BulletPhysicsWorld()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    BulletPhysicsWorld::~BulletPhysicsWorld()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicsWorld::initialize(
      platform::DebugRenderer* debug_renderer, 
	  world::IWorld* world)
    {
		physics_visualizer_.setDebugMode(
#if defined(_DEBUG) || defined(DEBUG)
			btIDebugDraw::DBG_DrawWireframe
#else
			btIDebugDraw::DBG_NoDebug
#endif
		);

	  world_ = world;
      physics_visualizer_.initialize(debug_renderer);
      
	  g_monoBehaviourSystem = world_->getScene().getSystem<components::MonoBehaviourSystem>().get();

	  collision_configuration_ =
		  foundation::Memory::construct<btDefaultCollisionConfiguration>();
	  dispatcher_ =
		  foundation::Memory::construct<btCollisionDispatcher>(
			  collision_configuration_
			  );
	  pair_cache_ = foundation::Memory::construct<btDbvtBroadphase>();
	  constraint_solver_ =
		  foundation::Memory::construct<btSequentialImpulseConstraintSolver>();
	  dynamics_world_ =
		  foundation::Memory::construct<btDiscreteDynamicsWorld>(
			  dispatcher_,
			  pair_cache_,
			  constraint_solver_,
			  collision_configuration_
			  );
	  dynamics_world_->setGravity(btVector3(0.0f, -9.81f, 0.0f));
	  dynamics_world_->setDebugDrawer(&physics_visualizer_);

	  gContactStartedCallback = ContactStarted;
	  gContactEndedCallback = ContactEnded;
	  gContactProcessedCallback = ContactProcessed;
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicsWorld::deinitialize()
    {
      g_monoBehaviourSystem = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicsWorld::render()
    {
      dynamics_world_->debugDrawWorld();
    }

    ///////////////////////////////////////////////////////////////////////////
    void BulletPhysicsWorld::update(const double& time_step)
    {
      for (int j = dynamics_world_->getNumCollisionObjects() - 1; j >= 0; --j)
      {
        btCollisionObject* object = 
          dynamics_world_->getCollisionObjectArray()[j];
        btRigidBody* rigid_body = btRigidBody::upcast(object);

        if (!rigid_body->isStaticObject())
        {
          btTransform transform;

		  BulletCollisionBody* rb = (BulletCollisionBody*)rigid_body->getUserPointer();
		  const entity::Entity entity = rb->getEntity();
		  bool activate = false;

		  {
			  glm::vec3 pos = world_->getScene().getSystem<components::TransformSystem>()->getWorldTranslation(entity);
			  glm::quat rot = world_->getScene().getSystem<components::TransformSystem>()->getWorldRotation(entity);

			  transform = rigid_body->getWorldTransform();

			  if (isValid(pos))
			  {
				  const btVector3 bt_pos(pos.x, pos.y, pos.z);
				  if (transform.getOrigin() != bt_pos)
				  {
					  transform.setOrigin(bt_pos);
					  activate = true;
				  }
			  }

			  if (isValid(rot))
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
		  BulletCollisionBody* rb = (BulletCollisionBody*)rigid_body->getUserPointer();
		  const entity::Entity entity = rb->getEntity();

          glm::quat rot(
            transform.getRotation().w(),
            transform.getRotation().x(),
            transform.getRotation().y(),
            transform.getRotation().z()
          );

          bool reset = false;
		  if (isValid(rot))
          {
            world_->getScene().getSystem<components::TransformSystem>()->setWorldRotation(entity, rot);
          }
          else
          {
            reset = true;
            rot = world_->getScene().getSystem<components::TransformSystem>()->getWorldRotation(entity);
            transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
          }

          // TODO (Hilze): Figure out why
          glm::vec3 pos(
            transform.getOrigin().x(),
            transform.getOrigin().y(),
            transform.getOrigin().z()
          );
          if (isValid(pos))
          {
            world_->getScene().getSystem<components::TransformSystem>()->setWorldTranslation(entity, pos);
          }
          else
          {
            reset = true;
            pos = world_->getScene().getSystem<components::TransformSystem>()->getWorldTranslation(entity);
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
    Vector<Manifold> BulletPhysicsWorld::raycast(
      const glm::vec3& start, 
      const glm::vec3& end)
    {
	  btVector3 f(start.x, start.y, start.z);
	  btVector3 t(end.x, end.y, end.z);
	  
	  btCollisionWorld::AllHitsRayResultCallback callback(f, t);
      dynamics_world_->rayTest(f, t, callback);


      Vector<Manifold> manifolds;
	  for (int i = 0; i < (int)callback.m_hitFractions.size(); ++i)
	  {
		  Manifold manifold;
		  manifold.num_contacts = 1;

		  BulletCollisionBody* rb = (BulletCollisionBody*)callback.m_collisionObjects[i]->getUserPointer();
		  manifold.rhs                = rb->getEntity();
		  manifold.contacts[0].point  = glm::vec3(callback.m_hitPointWorld[i].x(), callback.m_hitPointWorld[i].y(), callback.m_hitPointWorld[i].z());
		  manifold.contacts[0].normal = glm::vec3(callback.m_hitNormalWorld[i].x(), callback.m_hitNormalWorld[i].y(), callback.m_hitNormalWorld[i].z());

		  if (isValid(manifold.contacts[0].point) && isValid(manifold.contacts[0].normal))
			  manifolds.push_back(manifold);
	  }

      return manifolds;
    }

	///////////////////////////////////////////////////////////////////////////
	ICollisionBody* BulletPhysicsWorld::createCollisionBody(entity::Entity entity)
	{
		BulletCollisionBody* rb = foundation::Memory::construct<BulletCollisionBody>(
			world_,
			dynamics_world_,
			this,
			entity
			);
		rb->makeCollider();
		collision_bodies_.push_back(rb);

		return rb;
	}

	///////////////////////////////////////////////////////////////////////////
	void BulletPhysicsWorld::destroyCollisionBody(ICollisionBody* collision_body)
	{
		auto it = eastl::find(collision_bodies_.begin(), collision_bodies_.end(), collision_body);
		if (it != collision_bodies_.end())
			collision_bodies_.erase(it);

		foundation::Memory::destruct(collision_body);
	}

	///////////////////////////////////////////////////////////////////////////
	void physics::BulletPhysicsWorld::setDebugDrawEnabled(bool debug_draw_enabled)
	{
		physics_visualizer_.setDebugMode(
			debug_draw_enabled ?
			btIDebugDraw::DBG_DrawWireframe :
			btIDebugDraw::DBG_NoDebug
		);
	}

	///////////////////////////////////////////////////////////////////////////
	bool physics::BulletPhysicsWorld::getDebugDrawEnabled() const
	{
		return physics_visualizer_.getDebugMode() != btIDebugDraw::DBG_NoDebug;
	}

	///////////////////////////////////////////////////////////////////////////
	void physics::BulletPhysicsWorld::setGravity(glm::vec3 gravity)
	{
		dynamics_world_->setGravity(btVector3(
			gravity.x,
			gravity.y,
			gravity.z
		));
	}

	///////////////////////////////////////////////////////////////////////////
	glm::vec3 physics::BulletPhysicsWorld::getGravity() const
	{
		btVector3 gravity = dynamics_world_->getGravity();
		return glm::vec3(gravity.x(), gravity.y(), gravity.z());
	}
  }
}
