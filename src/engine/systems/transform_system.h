#pragma once
#include "interfaces/isystem.h"
#include "interfaces/icomponent.h"
#include <containers/containers.h>

#include <glm/mat4x4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace lambda
{
	namespace scene
	{
		struct Scene;
	}

	namespace components
	{
		class TransformComponent : public IComponent
		{
		public:
			TransformComponent(const entity::Entity& entity, scene::Scene& scene);
			TransformComponent(const TransformComponent& other);
			TransformComponent();

		public:
			glm::mat4 getLocal() const;
			glm::mat4 getWorld() const;
			glm::mat4 getInvWorld() const;

			bool hasParent() const;
			TransformComponent getParent() const;
			void setParent(const TransformComponent& parent);
			void unsetParent();
			Vector<TransformComponent> getChildren() const;

			void setLocalTranslation(const glm::vec3& translation);
			void setLocalRotation(const glm::quat& rotation);
			void setLocalRotation(const glm::vec3& euler);
			void setLocalScale(const glm::vec3& scale);
			glm::vec3 getLocalTranslation() const;
			glm::quat getLocalRotation() const;
			glm::vec3 getLocalScale() const;
			void moveLocal(const glm::vec3& delta);
			void rotateLocal(const glm::quat& delta);
			void scaleLocal(const glm::vec3& delta);

			void setWorldTranslation(const glm::vec3& translation);
			void setWorldRotation(const glm::quat& rotation);
			void setWorldRotation(const glm::vec3& euler);
			void setWorldScale(const glm::vec3& scale);
			glm::vec3 getWorldTranslation() const;
			glm::quat getWorldRotation() const;
			glm::vec3 getWorldScale() const;
			void moveWorld(const glm::vec3& delta);
			void rotateWorld(const glm::quat& delta);
			void scaleWorld(const glm::vec3& delta);

			glm::vec3 transformPoint(const glm::vec3& point) const;
			glm::vec3 transformVector(const glm::vec3& vector) const;
			glm::vec3 transformDirection(const glm::vec3& vector) const;
			glm::vec3 transformLocalPoint(const glm::vec3& point) const;
			glm::vec3 transformLocalVector(const glm::vec3& vector) const;
			glm::vec3 transformLocalDirection(const glm::vec3& vector) const;
			glm::vec3 inverseTransformPoint(const glm::vec3& point) const;
			glm::vec3 inverseTransformVector(const glm::vec3& vector) const;

			glm::vec3 getWorldForward() const;
			glm::vec3 getWorldUp() const;
			glm::vec3 getWorldRight() const;

			glm::vec3 getLocalForward() const;
			glm::vec3 getLocalUp() const;
			glm::vec3 getLocalRight() const;

			void lookAt(const glm::vec3& target, glm::vec3 up);
			void lookAtLocal(const glm::vec3& target, glm::vec3 up);

		private:
			scene::Scene* scene_;
		};

		namespace TransformSystem
		{
			struct Data
			{
				Data() {};
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				Vector<entity::Entity> children;
				entity::Entity entity;

				glm::vec3 translation = glm::vec3(0.0f);
				glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
				glm::vec3 scale = glm::vec3(1.0f);
				glm::mat4 local = glm::mat4(1.0f);
				glm::mat4 world = glm::mat4(1.0f);
				bool dirty = true;
				bool valid = true;
				entity::Entity parent = entity::InvalidEntity;

				entity::Entity getParent() const { return parent; }
				void setParent(entity::Entity p) { parent = p; }
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
			};

			TransformComponent addComponent(const entity::Entity& entity, scene::Scene& scene);
			TransformComponent getComponent(const entity::Entity& entity, scene::Scene& scene);
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene);
			void removeComponent(const entity::Entity& entity, scene::Scene& scene);

			void collectGarbage(scene::Scene& scene);
			void deinitialize(scene::Scene& scene);

			glm::mat4 getLocal(const entity::Entity& entity, scene::Scene& scene);
			glm::mat4 getWorld(const entity::Entity& entity, scene::Scene& scene);
			glm::mat4 getInvWorld(const entity::Entity& entity, scene::Scene& scene);

			bool hasParent(const entity::Entity& entity, scene::Scene& scene);
			entity::Entity getParent(const entity::Entity& entity, scene::Scene& scene);
			void setParent(const entity::Entity& entity, const entity::Entity& parent, scene::Scene& scene);
			void unsetParent(const entity::Entity& entity, scene::Scene& scene);
			Vector<entity::Entity> getChildren(const entity::Entity& entity, scene::Scene& scene);

			void setLocalTranslation(const entity::Entity& entity, const glm::vec3& translation, scene::Scene& scene);
			void setLocalRotation(const entity::Entity& entity, const glm::quat& rotation, scene::Scene& scene);
			void setLocalRotation(const entity::Entity& entity, const glm::vec3& euler, scene::Scene& scene);
			void setLocalScale(const entity::Entity& entity, const glm::vec3& scale, scene::Scene& scene);
			glm::vec3 getLocalTranslation(const entity::Entity& entity, scene::Scene& scene);
			glm::quat getLocalRotation(const entity::Entity& entity, scene::Scene& scene);
			glm::vec3 getLocalScale(const entity::Entity& entity, scene::Scene& scene);
			void moveLocal(const entity::Entity& entity, const glm::vec3& delta, scene::Scene& scene);
			void rotateLocal(const entity::Entity& entity, const glm::quat& delta, scene::Scene& scene);
			void scaleLocal(const entity::Entity& entity, const glm::vec3& delta, scene::Scene& scene);

			void setWorldTranslation(const entity::Entity& entity, const glm::vec3& translation, scene::Scene& scene);
			void setWorldRotation(const entity::Entity& entity, const glm::quat& rotation, scene::Scene& scene);
			void setWorldRotation(const entity::Entity& entity, const glm::vec3& euler, scene::Scene& scene);
			void setWorldScale(const entity::Entity& entity, const glm::vec3& scale, scene::Scene& scene);
			glm::vec3 getWorldTranslation(const entity::Entity& entity, scene::Scene& scene);
			glm::quat getWorldRotation(const entity::Entity& entity, scene::Scene& scene);
			glm::vec3 getWorldScale(const entity::Entity& entity, scene::Scene& scene);

			void moveWorld(const entity::Entity& entity, const glm::vec3& delta, scene::Scene& scene);
			void rotateWorld(const entity::Entity& entity, const glm::quat& delta, scene::Scene& scene);
			void scaleWorld(const entity::Entity& entity, const glm::vec3& delta, scene::Scene& scene);

			glm::vec3 transformPoint(const entity::Entity& entity, const glm::vec3& point, scene::Scene& scene);
			glm::vec3 transformVector(const entity::Entity& entity, const glm::vec3& vector, scene::Scene& scene);
			glm::vec3 transformDirection(const entity::Entity& entity, const glm::vec3& vector, scene::Scene& scene);
			glm::vec3 transformLocalPoint(const entity::Entity& entity, const glm::vec3& point, scene::Scene& scene);
			glm::vec3 transformLocalVector(const entity::Entity& entity, const glm::vec3& vector, scene::Scene& scene);
			glm::vec3 transformLocalDirection(const entity::Entity& entity, const glm::vec3& vector, scene::Scene& scene);
			glm::vec3 inverseTransformPoint(const entity::Entity& entity, const glm::vec3& point, scene::Scene& scene);
			glm::vec3 inverseTransformVector(const entity::Entity& entity, const glm::vec3& vector, scene::Scene& scene);

			glm::vec3 getWorldForward(const entity::Entity& entity, scene::Scene& scene);
			glm::vec3 getWorldUp(const entity::Entity& entity, scene::Scene& scene);
			glm::vec3 getWorldRight(const entity::Entity& entity, scene::Scene& scene);

			glm::vec3 getLocalForward(const entity::Entity& entity, scene::Scene& scene);
			glm::vec3 getLocalUp(const entity::Entity& entity, scene::Scene& scene);
			glm::vec3 getLocalRight(const entity::Entity& entity, scene::Scene& scene);

			void lookAt(const entity::Entity& entity, const glm::vec3& target, glm::vec3 up, scene::Scene& scene);
			void lookAtLocal(const entity::Entity& entity, const glm::vec3& target, glm::vec3 up, scene::Scene& scene);

			void cleanIfDirty(Data& data, scene::Scene& scene);
			void makeDirtyRecursive(Data& data, scene::Scene& scene);
			bool isChildOf(const entity::Entity& parent, const entity::Entity& child, scene::Scene& scene);

			glm::quat lookRotation(const glm::vec3& forward, const glm::vec3& up);

			static constexpr entity::Entity kRoot = entity::Entity();
		}
	}
}
