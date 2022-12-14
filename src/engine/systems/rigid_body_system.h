#pragma once
#include "interfaces/isystem.h"
#include "interfaces/iphysics.h"
#include "interfaces/icomponent.h"

#include <containers/containers.h>
#include <memory/memory.h>

#include <glm/vec3.hpp>

namespace lambda
{
	namespace scene
	{
		struct Scene;
	}

	namespace components
	{
		enum class RigidBodyConstraints : uint8_t
		{
			kNone = 0,
			kX = 1 << 0,
			kY = 1 << 1,
			kZ = 1 << 2,
		};

		class RigidBodyComponent : public IComponent
		{
		public:
			RigidBodyComponent(const entity::Entity& entity, scene::Scene& scene);
			RigidBodyComponent(const RigidBodyComponent& other);
			RigidBodyComponent();

			float getMass() const;
			void setMass(const float& mass);
			glm::vec3 getVelocity() const;
			void setVelocity(const glm::vec3& velocity);
			glm::vec3 getAngularVelocity() const;
			void setAngularVelocity(const glm::vec3& velocity);
			uint8_t getAngularConstraints() const;
			void setAngularConstraints(const uint8_t& constraints);
			uint8_t getVelocityConstraints() const;
			void setVelocityConstraints(const uint8_t& constraints);
			void applyImpulse(const glm::vec3& impulse);
			void setFriction(float friction);
			float getFriction() const;

		private:
			scene::Scene* scene_;
		};

		namespace RigidBodySystem
		{
			struct Data
			{
				Data() {}
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				bool valid = true;
				entity::Entity entity;
			};

			struct SystemData
			{
				Vector<Data>                  data;
				Map<entity::Entity, uint32_t> entity_to_data;
				Map<uint32_t, entity::Entity> data_to_entity;
				Set<entity::Entity>           marked_for_delete;
				Queue<uint32_t>               unused_data_entries;

				Data& add(const entity::Entity& entity);
				Data& get(const entity::Entity& entity);
				void  remove(const entity::Entity& entity);
				bool  has(const entity::Entity& entity);

				physics::IPhysicsWorld* physics_world;
			};

			RigidBodyComponent addComponent(const entity::Entity& entity, scene::Scene& scene);
			RigidBodyComponent getComponent(const entity::Entity& entity, scene::Scene& scene);
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene);
			void removeComponent(const entity::Entity& entity, scene::Scene& scene);
			physics::IPhysicsWorld* getPhysicsWorld(scene::Scene& scene);

			void collectGarbage(scene::Scene& scene);
			void initialize(scene::Scene& scene);
			void deinitialize(scene::Scene& scene);
			void fixedUpdate(const float& delta_time, scene::Scene& scene);

			float getMass(const entity::Entity& entity, scene::Scene& scene);
			void setMass(const entity::Entity& entity, const float& mass, scene::Scene& scene);
			glm::vec3 getVelocity(const entity::Entity& entity, scene::Scene& scene);
			void setVelocity(const entity::Entity& entity, const glm::vec3& velocity, scene::Scene& scene);
			glm::vec3 getAngularVelocity(const entity::Entity& entity, scene::Scene& scene);
			void setAngularVelocity(const entity::Entity& entity, const glm::vec3& velocity, scene::Scene& scene);
			uint8_t getAngularConstraints(const entity::Entity& entity, scene::Scene& scene);
			void setAngularConstraints(const entity::Entity& entity, const uint8_t& constraints, scene::Scene& scene);
			uint8_t getVelocityConstraints(const entity::Entity& entity, scene::Scene& scene);
			void setVelocityConstraints(const entity::Entity& entity, const uint8_t& constraints, scene::Scene& scene);
			void applyImpulse(const entity::Entity& entity, const glm::vec3& impulse, scene::Scene& scene);
			void setFriction(const entity::Entity& entity, float friction, scene::Scene& scene);
			float getFriction(const entity::Entity& entity, scene::Scene& scene);
		}
	}
}
