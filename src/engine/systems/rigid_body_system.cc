#include "systems/rigid_body_system.h"
#include "systems/transform_system.h"
#include "systems/collider_system.h"
#include "systems/mono_behaviour_system.h"
#include "systems/entity_system.h"
#include <platform/scene.h>

#include <glm/gtx/functions.hpp>

#if VIOLET_PHYSICS_BULLET
#include "physics/bullet/bullet_physics_world.h"
#endif
#if VIOLET_PHYSICS_REACT
#include "physics/react/react_physics_world.h"
#endif
#if VIOLET_PHYSICS_NEWTON
#include "physics/newton/newton_physics_world.h"
#endif


namespace lambda
{
  namespace components
  {
	  namespace RigidBodySystem
	  {
		  RigidBodyComponent addComponent(const entity::Entity& entity, scene::Scene& scene)
		  {
				if (!TransformSystem::hasComponent(entity, scene))
				  TransformSystem::addComponent(entity, scene);

				scene.rigid_body.add(entity);

			  auto& data = scene.rigid_body.get(entity);
			  auto& collider = scene.collider.get(entity);
			  data.collision_body = collider.collision_body;
			  data.collision_body->makeRigidBody();

			  return RigidBodyComponent(entity, scene);
		  }
		  RigidBodyComponent getComponent(const entity::Entity& entity, scene::Scene& scene)
		  {
			  return RigidBodyComponent(entity, scene);
		  }
		  bool hasComponent(const entity::Entity& entity, scene::Scene& scene)
		  {
			  return scene.rigid_body.has(entity);
		  }
		  void RigidBodySystem::removeComponent(const entity::Entity& entity, scene::Scene& scene)
		  {
			  scene.rigid_body.remove(entity);
		  }

		  void collectGarbage(scene::Scene & scene)
		  {
			  if (!scene.rigid_body.marked_for_delete.empty())
			  {
				  for (entity::Entity entity : scene.rigid_body.marked_for_delete)
				  {
					  const auto& it = scene.rigid_body.entity_to_data.find(entity);
					  if (it != scene.rigid_body.entity_to_data.end())
					  {
						  auto& data = scene.rigid_body.data.at(it->second);
						  data.collision_body->makeCollider();
						  data.collision_body = nullptr;
						  uint32_t idx = it->second;
						  scene.rigid_body.unused_data_entries.push(idx);
						  scene.rigid_body.data_to_entity.erase(idx);
						  scene.rigid_body.entity_to_data.erase(entity);
						  scene.rigid_body.data[idx].valid = false;
					  }
				  }
				  scene.rigid_body.marked_for_delete.clear();
			  }
		  }

		  void initialize(scene::Scene & scene)
		  {
#if VIOLET_PHYSICS_BULLET
			  scene.rigid_body.physics_world = foundation::Memory::construct<physics::BulletPhysicsWorld>();
#endif
#if VIOLET_PHYSICS_REACT
			  scene.rigid_body.physics_world = foundation::Memory::construct<physics::ReactPhysicsWorld>();
#endif
#if VIOLET_PHYSICS_NEWTON
			  scene.rigid_body.physics_world = foundation::Memory::construct<physics::NewtonPhysicsWorld>();
#endif

			  scene.rigid_body.physics_world->initialize(scene);
		  }

		  void deinitialize(scene::Scene & scene)
		  {
			  Vector<entity::Entity> entities;
			  for (const auto& it : scene.rigid_body.entity_to_data)
				  entities.push_back(it.first);

			  for (const auto& entity : entities)
				  removeComponent(entity, scene);
			  collectGarbage(scene);

			  scene.rigid_body.physics_world->deinitialize();
			  foundation::Memory::destruct(scene.rigid_body.physics_world);
			  scene.rigid_body.physics_world = nullptr;
		  }

			void fixedUpdate(const float& delta_time, scene::Scene& scene)
			{
				scene.rigid_body.physics_world->update(delta_time);
			}


			/////////////////////////////////////////////////////////////////////////////
			void serialize(scene::Scene& scene, scene::Serializer& serializer)
			{
				for (auto v : scene.rigid_body.unused_data_entries.get_container())
					serializer.serialize("rigid_body/unused_data_entries/", toString(v));
				for (auto v : scene.rigid_body.marked_for_delete)
					serializer.serialize("rigid_body/marked_for_delete/", toString(v));
				for (auto v : scene.rigid_body.data_to_entity)
					serializer.serialize("rigid_body/data_to_entity/", toString(v.first) + "|" + toString(v.second));
				for (auto v : scene.rigid_body.entity_to_data)
					serializer.serialize("rigid_body/entity_to_data/", toString(v.first) + "|" + toString(v.second));

				for (auto v : scene.rigid_body.data)
				{
					serializer.serialize("rigid_body/data/entity/", toString(v.entity));
					serializer.serialize("rigid_body/data/valid/", toString(v.valid));
					serializer.serialize("rigid_body/data/angular_constraints/", toString((uint32_t)v.collision_body->getAngularConstraints()));
					serializer.serialize("rigid_body/data/angular_velocity/", toString(v.collision_body->getAngularVelocity().x) + "|" + toString(v.collision_body->getAngularVelocity().y) + "|" + toString(v.collision_body->getAngularVelocity().z));
					serializer.serialize("rigid_body/data/friction/", toString(v.collision_body->getFriction()));
					serializer.serialize("rigid_body/data/layers/", toString(v.collision_body->getLayers()));
					serializer.serialize("rigid_body/data/mass/", toString(v.collision_body->getMass()));
					serializer.serialize("rigid_body/data/position/", toString(v.collision_body->getPosition().x) + "|" + toString(v.collision_body->getPosition().y) + "|" + toString(v.collision_body->getPosition().z));
					serializer.serialize("rigid_body/data/rotation/", toString(v.collision_body->getRotation().x) + "|" + toString(v.collision_body->getRotation().y) + "|" + toString(v.collision_body->getRotation().z) + "|" + toString(v.collision_body->getRotation().w));
					serializer.serialize("rigid_body/data/velocity/", toString(v.collision_body->getVelocity().x) + "|" + toString(v.collision_body->getVelocity().y) + "|" + toString(v.collision_body->getVelocity().z));
					serializer.serialize("rigid_body/data/velocity_constraints/", toString((uint32_t)v.collision_body->getVelocityConstraints()));
				}
			}

			/////////////////////////////////////////////////////////////////////////////
			void deserialize(scene::Scene& scene, scene::Serializer& serializer)
			{
				scene.name.unused_data_entries.get_container().clear();
				scene.name.marked_for_delete.clear();
				scene.name.data_to_entity.clear();
				scene.name.entity_to_data.clear();
				//scene.name.data.clear();

				for (const String& str : serializer.deserializeNamespace("rigid_body/unused_data_entries/"))
					scene.name.unused_data_entries.push(std::stoul(stlString(str)));
				for (const String& str : serializer.deserializeNamespace("rigid_body/marked_for_delete/"))
					scene.name.marked_for_delete.insert(std::stoul(stlString(str)));
				for (const String& str : serializer.deserializeNamespace("rigid_body/entity_to_data/"))
					scene.name.entity_to_data.insert({ std::stoul(stlString(split(str, '|')[0])), std::stoul(stlString(split(str, '|')[1])) });
				for (const String& str : serializer.deserializeNamespace("rigid_body/data_to_entity/"))
					scene.name.data_to_entity.insert({ std::stoul(stlString(split(str, '|')[0])), std::stoul(stlString(split(str, '|')[1])) });

				Vector<String> entities             = serializer.deserializeNamespace("rigid_body/data/entity/");
				Vector<String> validity             = serializer.deserializeNamespace("rigid_body/data/valid/");
				Vector<String> angular_constraints  = serializer.deserializeNamespace("rigid_body/data/angular_constraints/");
				Vector<String> angular_velocity     = serializer.deserializeNamespace("rigid_body/data/angular_velocity/");
				Vector<String> friction             = serializer.deserializeNamespace("rigid_body/data/friction/");
				Vector<String> layers               = serializer.deserializeNamespace("rigid_body/data/layers/");
				Vector<String> mass                 = serializer.deserializeNamespace("rigid_body/data/mass/");
				Vector<String> position             = serializer.deserializeNamespace("rigid_body/data/position/");
				Vector<String> rotation             = serializer.deserializeNamespace("rigid_body/data/rotation/");
				Vector<String> velocity             = serializer.deserializeNamespace("rigid_body/data/velocity/");
				Vector<String> velocity_constraints = serializer.deserializeNamespace("rigid_body/data/velocity_constraints/");

				for (uint32_t i = 0; i < entities.size(); ++i)
				{
					entity::Entity entity = (entity::Entity)std::stoul(stlString(entities[i]));
					//Data data((entity::Entity)std::stoul(stlString(entities[i])));
					Data& data = scene.rigid_body.get(entity);
					data.valid = std::stoul(stlString(validity[i])) > 0ul ? true : false;
					data.entity = entity;
					data.collision_body->setAngularConstraints((uint8_t)std::stoul(stlString(angular_constraints[i])));
					data.collision_body->setAngularVelocity(glm::vec3(
						std::stof(stlString(split(angular_velocity[i], '|')[0])),
						std::stof(stlString(split(angular_velocity[i], '|')[1])),
						std::stof(stlString(split(angular_velocity[i], '|')[2]))
					));
					data.collision_body->setFriction(std::stof(stlString(friction[i])));
					data.collision_body->setLayers((uint16_t)std::stoul(stlString(layers[i])));
					data.collision_body->setMass(std::stof(stlString(mass[i])));
					data.collision_body->setPosition(glm::vec3(
						std::stof(stlString(split(position[i], '|')[0])),
						std::stof(stlString(split(position[i], '|')[1])),
						std::stof(stlString(split(position[i], '|')[2]))
					));
					data.collision_body->setRotation(glm::quat(
						std::stof(stlString(split(rotation[i], '|')[3])),
						std::stof(stlString(split(rotation[i], '|')[0])),
						std::stof(stlString(split(rotation[i], '|')[1])),
						std::stof(stlString(split(rotation[i], '|')[2]))
					));
					data.collision_body->setVelocity(glm::vec3(
						std::stof(stlString(split(velocity[i], '|')[0])),
						std::stof(stlString(split(velocity[i], '|')[1])),
						std::stof(stlString(split(velocity[i], '|')[2]))
					));
					data.collision_body->setVelocityConstraints((uint8_t)std::stoul(stlString(velocity_constraints[i])));

					//scene.rigid_body.data.push_back(data);
				}
			}

		  physics::IPhysicsWorld* RigidBodySystem::getPhysicsWorld(scene::Scene& scene)
		  {
			  return scene.rigid_body.physics_world;
		  }
		  float RigidBodySystem::getMass(const entity::Entity& entity, scene::Scene& scene)
		  {
			  return scene.rigid_body.get(entity).collision_body->getMass();
		  }
		  void RigidBodySystem::setMass(const entity::Entity& entity, const float& mass, scene::Scene& scene)
		  {
			  scene.rigid_body.get(entity).collision_body->setMass(mass);
		  }
		  glm::vec3 RigidBodySystem::getVelocity(const entity::Entity& entity, scene::Scene& scene)
		  {
			  return scene.rigid_body.get(entity).collision_body->getVelocity();
		  }
		  void RigidBodySystem::setVelocity(const entity::Entity& entity, const glm::vec3& velocity, scene::Scene& scene)
		  {
			  if (std::isnan(velocity.x) || std::isnan(velocity.y) || std::isnan(velocity.z))
				  return;

			  scene.rigid_body.get(entity).collision_body->setVelocity(velocity);
		  }
		  glm::vec3 RigidBodySystem::getAngularVelocity(const entity::Entity& entity, scene::Scene& scene)
		  {
			  return scene.rigid_body.get(entity).collision_body->getAngularVelocity();
		  }
		  void RigidBodySystem::setAngularVelocity(const entity::Entity& entity, const glm::vec3& velocity, scene::Scene& scene)
		  {
			  if (std::isnan(velocity.x) || std::isnan(velocity.y) || std::isnan(velocity.z))
				  return;

			  scene.rigid_body.get(entity).collision_body->setAngularVelocity(velocity);
		  }
		  uint8_t RigidBodySystem::getAngularConstraints(const entity::Entity& entity, scene::Scene& scene)
		  {
			  return scene.rigid_body.get(entity).collision_body->getAngularConstraints();
		  }
		  void RigidBodySystem::setAngularConstraints(const entity::Entity& entity, const uint8_t& constraints, scene::Scene& scene)
		  {
			  scene.rigid_body.get(entity).collision_body->setAngularConstraints(constraints);
		  }
		  uint8_t RigidBodySystem::getVelocityConstraints(const entity::Entity& entity, scene::Scene& scene)
		  {
			  return scene.rigid_body.get(entity).collision_body->getVelocityConstraints();
		  }
		  void RigidBodySystem::setVelocityConstraints(const entity::Entity& entity, const uint8_t& constraints, scene::Scene& scene)
		  {
			  scene.rigid_body.get(entity).collision_body->setVelocityConstraints(constraints);
		  }
		  void RigidBodySystem::applyImpulse(const entity::Entity& entity, const glm::vec3& impulse, scene::Scene& scene)
		  {
			  scene.rigid_body.get(entity).collision_body->applyImpulse(impulse);
		  }
		  void RigidBodySystem::setFriction(const entity::Entity & entity, float friction, scene::Scene& scene)
		  {
			  scene.rigid_body.get(entity).collision_body->setFriction(friction);
		  }
		  float RigidBodySystem::getFriction(const entity::Entity & entity, scene::Scene& scene)
		  {
			  return scene.rigid_body.get(entity).collision_body->getFriction();
		  }
	  }



	  // The system data.
	  namespace RigidBodySystem
	  {
		  Data& SystemData::add(const entity::Entity& entity)
		  {
			  uint32_t idx = 0ul;
			  if (!unused_data_entries.empty())
			  {
				  idx = unused_data_entries.front();
				  unused_data_entries.pop();
				  data[idx] = Data(entity);
			  }
			  else
			  {
				  idx = (uint32_t)data.size();
				  data.push_back(Data(entity));
				  data_to_entity[idx] = entity;
			  }

			  data_to_entity[idx] = entity;
			  entity_to_data[entity] = idx;

			  return data[idx];
		  }

		  Data& SystemData::get(const entity::Entity& entity)
		  {
			  auto it = entity_to_data.find(entity);
			  LMB_ASSERT(it != entity_to_data.end(), "RIGIDBODY: %llu does not have a component", entity);
			  LMB_ASSERT(data[it->second].valid, "RIGIDBODY: %llu's data was not valid", entity);
			  return data[it->second];
		  }

		  void SystemData::remove(const entity::Entity& entity)
		  {
			  marked_for_delete.insert(entity);
		  }

		  bool SystemData::has(const entity::Entity& entity)
		  {
			  return entity_to_data.find(entity) != entity_to_data.end();
		  }
	  }





	namespace RigidBodySystem
	{
		Data::Data(const Data& other)
		{
			collision_body = other.collision_body;
			entity = other.entity;
			valid = other.valid;
		}
		Data& Data::operator=(const Data& other)
		{
			collision_body = other.collision_body;
			entity = other.entity;
			valid = other.valid;
			return *this;
		}
	}












	RigidBodyComponent::RigidBodyComponent(const entity::Entity& entity, scene::Scene& scene) :
		IComponent(entity), scene_(&scene)
	{
	}
	RigidBodyComponent::RigidBodyComponent(const RigidBodyComponent& other) :
		IComponent(other.entity_), scene_(other.scene_)
	{
	}
	RigidBodyComponent::RigidBodyComponent() :
		IComponent(entity::Entity()), scene_(nullptr)
	{
	}
	float RigidBodyComponent::getMass() const
	{
		return RigidBodySystem::getMass(entity_, *scene_);
	}
	void RigidBodyComponent::setMass(const float& mass)
	{
		RigidBodySystem::setMass(entity_, mass, *scene_);
	}
	glm::vec3 RigidBodyComponent::getVelocity() const
	{
		return RigidBodySystem::getVelocity(entity_, *scene_);
	}
	void RigidBodyComponent::setVelocity(const glm::vec3& velocity)
	{
		RigidBodySystem::setVelocity(entity_, velocity, *scene_);
	}
	glm::vec3 RigidBodyComponent::getAngularVelocity() const
	{
		return RigidBodySystem::getAngularVelocity(entity_, *scene_);
	}
	void RigidBodyComponent::setAngularVelocity(const glm::vec3& velocity)
	{
		RigidBodySystem::setAngularVelocity(entity_, velocity, *scene_);
	}
	uint8_t RigidBodyComponent::getAngularConstraints() const
	{
		return RigidBodySystem::getAngularConstraints(entity_, *scene_);
	}
	void RigidBodyComponent::setAngularConstraints(const uint8_t& constraints)
	{
		RigidBodySystem::setAngularConstraints(entity_, constraints, *scene_);
	}
	uint8_t RigidBodyComponent::getVelocityConstraints() const
	{
		return RigidBodySystem::getVelocityConstraints(entity_, *scene_);
	}
	void RigidBodyComponent::setVelocityConstraints(const uint8_t& constraints)
	{
		RigidBodySystem::setVelocityConstraints(entity_, constraints, *scene_);
	}
	void RigidBodyComponent::applyImpulse(const glm::vec3& impulse)
	{
		RigidBodySystem::applyImpulse(entity_, impulse, *scene_);
	}
	void RigidBodyComponent::setFriction(float friction)
	{
		RigidBodySystem::setFriction(entity_, friction, *scene_);
	}
	float RigidBodyComponent::getFriction() const
	{
		return RigidBodySystem::getFriction(entity_, *scene_);
	}
  }
}
