#include "transform_system.h"
#include "utils/decompose_matrix.h"
#include <utils/console.h>
#include <platform/scene.h>

namespace lambda
{
	namespace components
	{
		// The actual system.
		namespace TransformSystem
		{
			glm::quat lookRotation(const glm::vec3& forward, const glm::vec3& up)
			{
				if (forward == glm::vec3())
				{
					return glm::quat(1, 0, 0, 0);
				}

				const glm::vec3 new_forward = glm::normalize(forward);
				const glm::vec3 new_right = glm::normalize(glm::cross(up, new_forward));
				const glm::vec3 new_up = glm::cross(new_forward, new_right);

				const float m00 = new_right.x;
				const float m01 = new_right.y;
				const float m02 = new_right.z;
				const float m10 = new_up.x;
				const float m11 = new_up.y;
				const float m12 = new_up.z;
				const float m20 = new_forward.x;
				const float m21 = new_forward.y;
				const float m22 = new_forward.z;

				const float num8 = (m00 + m11) + m22;
				glm::quat quaternion;
				if (num8 > 0.0f)
				{
					float num = sqrtf(num8 + 1.0f);
					quaternion.w = num * 0.5f;
					num = 0.5f / num;
					quaternion.x = (m12 - m21) * num;
					quaternion.y = (m20 - m02) * num;
					quaternion.z = (m01 - m10) * num;
					return quaternion;
				}
				if ((m00 >= m11) && (m00 >= m22))
				{
					const float num7 = sqrtf(((1.0f + m00) - m11) - m22);
					const float num4 = 0.5f / num7;
					quaternion.x = 0.5f * num7;
					quaternion.y = (m01 + m10) * num4;
					quaternion.z = (m02 + m20) * num4;
					quaternion.w = (m12 - m21) * num4;
					return quaternion;
				}
				if (m11 > m22)
				{
					const float num6 = sqrtf(((1.0f + m11) - m00) - m22);
					const float num3 = 0.5f / num6;
					quaternion.x = (m10 + m01) * num3;
					quaternion.y = 0.5f * num6;
					quaternion.z = (m21 + m12) * num3;
					quaternion.w = (m20 - m02) * num3;
					return quaternion;
				}
				const float num5 = sqrtf(((1.0f + m22) - m00) - m11);
				const float num2 = 0.5f / num5;
				quaternion.x = (m20 + m02) * num2;
				quaternion.y = (m21 + m12) * num2;
				quaternion.z = 0.5f * num5;
				quaternion.w = (m01 - m10) * num2;
				return quaternion;
			}

			void cleanIfDirty(Data& data, scene::Scene& scene)
			{
				if (data.dirty)
				{
					data.dirty = false;

					// Local.
					data.local = glm::translate(glm::mat4(1.0f), data.translation);
					data.local *= glm::mat4_cast(data.rotation);
					data.local = glm::scale(data.local, data.scale);

					// World.
					if (data.getParent() == kRoot || data.getParent() == data.entity)
					{
						data.world = data.local;
					}
					else
					{
						data.world = getWorld(data.getParent(), scene) * data.local;
					}
				}
			}

			void makeDirtyRecursive(Data& data, scene::Scene& scene)
			{
				data.dirty = true;
				for (const entity::Entity& child : data.children)
				{
					makeDirtyRecursive(scene.transform.get(child), scene);
				}
			}

			bool isChildOf(const entity::Entity& parent, const entity::Entity& child, scene::Scene& scene)
			{
				for (const entity::Entity& ch : scene.transform.get(parent).children)
					if (ch == child || isChildOf(ch, child, scene))
						return true;

				return false;
			}

			TransformComponent addComponent(const entity::Entity& entity, scene::Scene& scene)
			{

				scene.transform.add(entity);
				return TransformComponent(entity, scene);
			}

			TransformComponent getComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return TransformComponent(entity, scene);
			}

			bool hasComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.transform.has(entity);
			}

			void removeComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				/*for (entity::Entity child : getChildren(entity))
					unsetParent(child);*/
				unsetParent(entity, scene);
				scene.transform.remove(entity);
			}

			void collectGarbage(scene::Scene & scene)
			{
				if (!scene.transform.marked_for_delete.empty())
				{
					for (entity::Entity entity : scene.transform.marked_for_delete)
					{
						const auto& it = scene.transform.entity_to_data.find(entity);
						if (it != scene.transform.entity_to_data.end())
						{
							uint32_t idx = it->second;
							scene.transform.unused_data_entries.push(idx);
							scene.transform.data_to_entity.erase(idx);
							scene.transform.entity_to_data.erase(entity);
							scene.transform.data[idx].valid = false;
						}
					}
					scene.transform.marked_for_delete.clear();
				}
			}

			void deinitialize(scene::Scene & scene)
			{
				Vector<entity::Entity> entities;
				for (const auto& it : scene.transform.entity_to_data)
					entities.push_back(it.first);

				for (const auto& entity : entities)
					scene.transform.remove(entity);
				collectGarbage(scene);
			}

			glm::mat4 getLocal(const entity::Entity& entity, scene::Scene& scene)
			{
				Data& data = scene.transform.get(entity);
				cleanIfDirty(data, scene);
				return data.local;
			}

			glm::mat4 getWorld(const entity::Entity& entity, scene::Scene& scene)
			{
				Data& data = scene.transform.get(entity);
				cleanIfDirty(data, scene);
				return data.world;
			}

			glm::mat4 getInvWorld(const entity::Entity& entity, scene::Scene& scene)
			{
				return glm::inverse(getWorld(entity, scene));
			}

			bool hasParent(const entity::Entity& entity, scene::Scene& scene)
			{
				const Data& data = scene.transform.get(entity);
				return data.getParent() != 0u && data.getParent() != entity;
			}

			entity::Entity getParent(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.transform.get(entity).getParent();
			}

			void setParent(const entity::Entity& entity, const entity::Entity& parent, scene::Scene& scene)
			{
				// You cannot parent yourself to yourself
				// You cannot parent yourself to something that is your child.
				if (entity == parent || isChildOf(entity, parent, scene))
					return;

				// You cannot parent yourself to something else recursively.
				if (parent != kRoot && entity == scene.transform.get(parent).getParent())
					return;

				Data& data = scene.transform.get(entity);
				//entity::Entity was  = data.getParent();
				if (data.getParent() != kRoot)
				{
					Data& p_data = scene.transform.get(data.getParent());
					eastl::remove(p_data.children.begin(), p_data.children.end(), entity);
				}

				data.setParent(parent);

				makeDirtyRecursive(data, scene);

				if (parent != kRoot)
				{
					auto& parent_data = scene.transform.get(parent);
					if (eastl::find(parent_data.children.begin(), parent_data.children.end(), entity) == parent_data.children.end())
						parent_data.children.push_back(entity);
				}
			}

			void unsetParent(const entity::Entity& entity, scene::Scene& scene)
			{
				setParent(entity, kRoot, scene);
			}

			Vector<entity::Entity> getChildren(const entity::Entity& entity, scene::Scene& scene)
			{
				Vector<entity::Entity> children;
				for (const auto& child : scene.transform.get(entity).children)
					children.push_back(child);
				return children;
			}

			void setLocalTranslation(const entity::Entity& entity, const glm::vec3& translation, scene::Scene& scene)
			{
				Data& data = scene.transform.get(entity);
				data.translation = translation;
				makeDirtyRecursive(data, scene);
			}

			void setLocalRotation(const entity::Entity& entity, const glm::quat& rotation, scene::Scene& scene)
			{
				Data& data = scene.transform.get(entity);
				data.rotation = rotation;
				makeDirtyRecursive(data, scene);
			}

			void setLocalRotation(const entity::Entity& entity, const glm::vec3& euler, scene::Scene& scene)
			{
				setLocalRotation(entity, glm::quat(euler), scene);
			}

			void setLocalScale(const entity::Entity& entity, const glm::vec3& scale, scene::Scene& scene)
			{
				Data& data = scene.transform.get(entity);
				data.scale = scale;
				makeDirtyRecursive(data, scene);
			}

			glm::vec3 getLocalTranslation(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.transform.get(entity).translation;
			}

			glm::quat getLocalRotation(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.transform.get(entity).rotation;
			}

			glm::vec3 getLocalScale(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.transform.get(entity).scale;
			}

			void moveLocal(const entity::Entity& entity, const glm::vec3& delta, scene::Scene& scene)
			{
				Data& data = scene.transform.get(entity);
				data.translation += delta;
				makeDirtyRecursive(data, scene);
			}

			void rotateLocal(const entity::Entity& entity, const glm::quat& delta, scene::Scene& scene)
			{
				Data& data = scene.transform.get(entity);
				data.rotation += delta; // TODO (Hilze): Validate this.
				makeDirtyRecursive(data, scene);
			}

			void scaleLocal(const entity::Entity& entity, const glm::vec3& delta, scene::Scene& scene)
			{
				Data& data = scene.transform.get(entity);
				data.scale *= delta;
				makeDirtyRecursive(data, scene);
			}

			void setWorldTranslation(const entity::Entity& entity, const glm::vec3& translation, scene::Scene& scene)
			{
				const Data& data = scene.transform.get(entity);
				if (0u == data.getParent() || data.getParent() == entity)
				{
					setLocalTranslation(entity, translation, scene);
					return;
				}
				setLocalTranslation(
					entity,
					getInvWorld(getParent(entity, scene), scene) * glm::vec4(translation, 1.0f),
					scene
				);
			}

			void setWorldRotation(const entity::Entity& entity, const glm::quat& rotation, scene::Scene& scene)
			{
				const Data& data = scene.transform.get(entity);
				if (0u == data.getParent() || data.getParent() == entity)
				{
					setLocalRotation(entity, rotation, scene);
					return;
				}

				const glm::quat parent_rotation = getWorldRotation(data.getParent(), scene);
				// q' represents the rotation from the rotation q1 to the rotation q2
				// q' = q1^-1 * q2

				setLocalRotation(entity, glm::inverse(parent_rotation) * rotation, scene);
			}

			void setWorldRotation(const entity::Entity& entity, const glm::vec3& euler, scene::Scene& scene)
			{
				setWorldRotation(entity, glm::quat(euler), scene);
			}

			void setWorldScale(const entity::Entity& entity, const glm::vec3& scale, scene::Scene& scene)
			{
				const Data& data = scene.transform.get(entity);
				if (0u == data.getParent() || data.getParent() == entity)
				{
					setLocalScale(entity, scale, scene);
					return;
				}
				setLocalScale(
					entity,
					scale / getWorldScale(getParent(entity, scene), scene),
					scene
				);
			}

			glm::vec3 getWorldTranslation(const entity::Entity& entity, scene::Scene& scene)
			{
				glm::vec3 scale;
				glm::vec3 translation;
				glm::quat orientation;
				utilities::decomposeMatrix(getWorld(entity, scene), scale, orientation, translation);

				return translation;
			}

			glm::quat getWorldRotation(const entity::Entity& entity, scene::Scene& scene)
			{
				const Data& data = scene.transform.get(entity);
				if (data.getParent() != 0u && entity != data.getParent())
					return data.rotation * getWorldRotation(data.getParent(), scene);
				else
					return data.rotation;
			}

			glm::vec3 getWorldScale(const entity::Entity& entity, scene::Scene& scene)
			{
				const Data& data = scene.transform.get(entity);
				if (data.getParent() != 0u && entity != data.getParent())
					return data.scale * getWorldScale(data.getParent(), scene);
				else
					return data.scale;
			}

			void moveWorld(const entity::Entity& entity, const glm::vec3& delta, scene::Scene& scene)
			{
				setWorldTranslation(entity, getWorldTranslation(entity, scene) + delta, scene);
			}

			void rotateWorld(const entity::Entity& entity, const glm::quat& delta, scene::Scene& scene)
			{
				setWorldRotation(entity, getWorldRotation(entity, scene) * delta, scene);
			}

			void scaleWorld(const entity::Entity& entity, const glm::vec3& delta, scene::Scene& scene)
			{
				setWorldScale(entity, getWorldScale(entity, scene) * delta, scene);
			}

			glm::vec3 transformPoint(const entity::Entity& entity, const glm::vec3& point, scene::Scene& scene)
			{
				return getWorld(entity, scene) * glm::vec4(point, 1.0f);
			}

			glm::vec3 transformVector(const entity::Entity& entity, const glm::vec3& vector, scene::Scene& scene)
			{
				return getWorld(entity, scene) * glm::vec4(vector, 0.0f);
			}

			glm::vec3 transformDirection(const entity::Entity& entity, const glm::vec3& vector, scene::Scene& scene)
			{
				return getWorldRotation(entity, scene) * vector;
			}

			glm::vec3 transformLocalPoint(const entity::Entity& entity, const glm::vec3& point, scene::Scene& scene)
			{
				return getLocal(entity, scene) * glm::vec4(point, 0.0f);
			}

			glm::vec3 transformLocalVector(const entity::Entity& entity, const glm::vec3& vector, scene::Scene& scene)
			{
				return getLocal(entity, scene) * glm::vec4(vector, 0.0f);
			}

			glm::vec3 transformLocalDirection(const entity::Entity& entity, const glm::vec3& vector, scene::Scene& scene)
			{
				return getLocalRotation(entity, scene) * vector;
			}

			glm::vec3 inverseTransformPoint(const entity::Entity& entity, const glm::vec3& point, scene::Scene& scene)
			{
				return getInvWorld(entity, scene) * glm::vec4(point, 1.0f);
			}

			glm::vec3 inverseTransformVector(const entity::Entity& entity, const glm::vec3& vector, scene::Scene& scene)
			{
				return getInvWorld(entity, scene) * glm::vec4(vector, 0.0f);
			}

			glm::vec3 getWorldForward(const entity::Entity& entity, scene::Scene& scene)
			{
				return ((glm::mat3x3)getWorld(entity, scene)) * glm::vec3(0.0f, 0.0f, -1.0f);
			}

			glm::vec3 getWorldUp(const entity::Entity& entity, scene::Scene& scene)
			{
				return ((glm::mat3x3)getWorld(entity, scene)) * glm::vec3(0.0f, 1.0f, 0.0f);
			}

			glm::vec3 getWorldRight(const entity::Entity& entity, scene::Scene& scene)
			{
				return ((glm::mat3x3)getWorld(entity, scene)) * glm::vec3(1.0f, 0.0f, 0.0f);
			}

			glm::vec3 getLocalForward(const entity::Entity& entity, scene::Scene& scene)
			{
				return ((glm::mat3x3)getLocal(entity, scene)) * glm::vec3(0.0f, 0.0f, -1.0f);
			}

			glm::vec3 getLocalUp(const entity::Entity& entity, scene::Scene& scene)
			{
				return ((glm::mat3x3)getLocal(entity, scene)) * glm::vec3(0.0f, 1.0f, 0.0f);
			}

			glm::vec3 getLocalRight(const entity::Entity& entity, scene::Scene& scene)
			{
				return ((glm::mat3x3)getLocal(entity, scene)) * glm::vec3(0.0f, 0.0f, -1.0f);
			}

			void lookAt(const entity::Entity& entity, const glm::vec3& target, glm::vec3 up, scene::Scene& scene)
			{
				const glm::vec3 direction = glm::normalize(target - getWorldTranslation(entity, scene));
				setLocalRotation(entity, TransformSystem::lookRotation(direction, up), scene);
			}

			void lookAtLocal(const entity::Entity& entity, const glm::vec3& target, glm::vec3 up, scene::Scene& scene)
			{
				setLocalRotation(entity, glm::quat_cast(glm::lookAt(target, getLocalTranslation(entity, scene), up)), scene);
			}
		}

		// The system data.
		namespace TransformSystem
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
				LMB_ASSERT(it != entity_to_data.end(), "TRANSFORM: %llu does not have a component", entity);
				LMB_ASSERT(data[it->second].valid, "TRANSFORM: %llu's data was not valid", entity);
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

		// The transform data.
		namespace TransformSystem
		{
			Data::Data(const Data& other)
			{
				children = other.children;
				parent = other.parent;
				entity = other.entity;
				translation = other.translation;
				rotation = other.rotation;
				scale = other.scale;
				local = other.local;
				world = other.world;
				dirty = other.dirty;
				valid = other.valid;
			}

			Data& Data::operator=(const Data& other)
			{
				children = other.children;
				parent = other.parent;
				entity = other.entity;
				translation = other.translation;
				rotation = other.rotation;
				scale = other.scale;
				local = other.local;
				world = other.world;
				dirty = other.dirty;
				valid = other.valid;
				return *this;
			}
		}

















		TransformComponent::TransformComponent(const entity::Entity& entity, scene::Scene& scene) :
			IComponent(entity), scene_(&scene)
		{
		}
		TransformComponent::TransformComponent(const TransformComponent& other) :
			IComponent(other.entity_), scene_(other.scene_)
		{
		}
		TransformComponent::TransformComponent() :
			IComponent(entity::Entity()), scene_(nullptr)
		{
		}
		glm::mat4 TransformComponent::getLocal() const
		{
			return TransformSystem::getLocal(entity_, *scene_);
		}
		glm::mat4 TransformComponent::getWorld() const
		{
			return TransformSystem::getWorld(entity_, *scene_);
		}
		glm::mat4 TransformComponent::getInvWorld() const
		{
			return TransformSystem::getInvWorld(entity_, *scene_);
		}
		bool TransformComponent::hasParent() const
		{
			return TransformSystem::hasParent(entity_, *scene_);
		}
		TransformComponent TransformComponent::getParent() const
		{
			return TransformSystem::getComponent(TransformSystem::getParent(entity_, *scene_), *scene_);
		}
		void TransformComponent::setParent(const TransformComponent& parent)
		{
			TransformSystem::setParent(entity_, parent.entity(), *scene_);
		}
		void TransformComponent::unsetParent()
		{
			TransformSystem::unsetParent(entity_, *scene_);
		}
		Vector<TransformComponent> TransformComponent::getChildren() const
		{
			Vector<TransformComponent> children;
			for (const entity::Entity& entity : TransformSystem::getChildren(entity_, *scene_))
				children.push_back(TransformSystem::getComponent(entity, *scene_));
			return children;
		}
		void TransformComponent::setLocalTranslation(const glm::vec3& translation)
		{
			TransformSystem::setLocalTranslation(entity_, translation, *scene_);
		}
		void TransformComponent::setLocalRotation(const glm::quat& rotation)
		{
			TransformSystem::setLocalRotation(entity_, rotation, *scene_);
		}
		void TransformComponent::setLocalRotation(const glm::vec3& euler)
		{
			TransformSystem::setLocalRotation(entity_, euler, *scene_);
		}
		void TransformComponent::setLocalScale(const glm::vec3& scale)
		{
			TransformSystem::setLocalScale(entity_, scale, *scene_);
		}
		glm::vec3 TransformComponent::getLocalTranslation() const
		{
			return TransformSystem::getLocalTranslation(entity_, *scene_);
		}
		glm::quat TransformComponent::getLocalRotation() const
		{
			return TransformSystem::getLocalRotation(entity_, *scene_);
		}
		glm::vec3 TransformComponent::getLocalScale() const
		{
			return TransformSystem::getLocalScale(entity_, *scene_);
		}
		void TransformComponent::moveLocal(const glm::vec3& delta)
		{
			TransformSystem::moveLocal(entity_, delta, *scene_);
		}
		void TransformComponent::rotateLocal(const glm::quat& delta)
		{
			TransformSystem::rotateLocal(entity_, delta, *scene_);
		}
		void TransformComponent::scaleLocal(const glm::vec3& delta)
		{
			TransformSystem::scaleLocal(entity_, delta, *scene_);
		}
		void TransformComponent::setWorldTranslation(const glm::vec3& translation)
		{
			return TransformSystem::setWorldTranslation(entity_, translation, *scene_);
		}
		void TransformComponent::setWorldRotation(const glm::quat& rotation)
		{
			return TransformSystem::setWorldRotation(entity_, rotation, *scene_);
		}
		void TransformComponent::setWorldRotation(const glm::vec3& euler)
		{
			setWorldRotation(glm::quat(euler));
		}
		void TransformComponent::setWorldScale(const glm::vec3& scale)
		{
			TransformSystem::setWorldScale(entity_, scale, *scene_);
		}
		glm::vec3 TransformComponent::getWorldTranslation() const
		{
			return TransformSystem::getWorldTranslation(entity_, *scene_);
		}
		glm::quat TransformComponent::getWorldRotation() const
		{
			return TransformSystem::getWorldRotation(entity_, *scene_);
		}
		glm::vec3 TransformComponent::getWorldScale() const
		{
			return TransformSystem::getWorldScale(entity_, *scene_);
		}
		void TransformComponent::moveWorld(const glm::vec3& delta)
		{
			TransformSystem::moveWorld(entity_, delta, *scene_);
		}
		void TransformComponent::rotateWorld(const glm::quat& delta)
		{
			TransformSystem::rotateWorld(entity_, delta, *scene_);
		}
		void TransformComponent::scaleWorld(const glm::vec3& delta)
		{
			TransformSystem::scaleWorld(entity_, delta, *scene_);
		}
		glm::vec3 TransformComponent::transformPoint(const glm::vec3& point) const
		{
			return TransformSystem::transformPoint(entity_, point, *scene_);
		}
		glm::vec3 TransformComponent::transformVector(const glm::vec3& vector) const
		{
			return TransformSystem::transformVector(entity_, vector, *scene_);
		}
		glm::vec3 TransformComponent::transformDirection(const glm::vec3& vector) const
		{
			return TransformSystem::transformDirection(entity_, vector, *scene_);
		}
		glm::vec3 TransformComponent::transformLocalPoint(const glm::vec3& point) const
		{
			return TransformSystem::transformLocalPoint(entity_, point, *scene_);
		}
		glm::vec3 TransformComponent::transformLocalVector(const glm::vec3& vector) const
		{
			return TransformSystem::transformLocalVector(entity_, vector, *scene_);
		}
		glm::vec3 TransformComponent::transformLocalDirection(const glm::vec3& vector) const
		{
			return TransformSystem::transformLocalDirection(entity_, vector, *scene_);
		}
		glm::vec3 TransformComponent::inverseTransformPoint(const glm::vec3& point) const
		{
			return TransformSystem::inverseTransformPoint(entity_, point, *scene_);
		}
		glm::vec3 TransformComponent::inverseTransformVector(const glm::vec3& vector) const
		{
			return TransformSystem::inverseTransformVector(entity_, vector, *scene_);
		}
		glm::vec3 TransformComponent::getWorldForward() const
		{
			return TransformSystem::getWorldForward(entity_, *scene_);
		}
		glm::vec3 TransformComponent::getWorldUp() const
		{
			return TransformSystem::getWorldUp(entity_, *scene_);
		}
		glm::vec3 TransformComponent::getWorldRight() const
		{
			return TransformSystem::getWorldRight(entity_, *scene_);
		}
		glm::vec3 TransformComponent::getLocalForward() const
		{
			return TransformSystem::getLocalForward(entity_, *scene_);
		}
		glm::vec3 TransformComponent::getLocalUp() const
		{
			return TransformSystem::getLocalUp(entity_, *scene_);
		}
		glm::vec3 TransformComponent::getLocalRight() const
		{
			return TransformSystem::getLocalRight(entity_, *scene_);
		}
		void TransformComponent::lookAt(const glm::vec3& target, glm::vec3 up)
		{
			TransformSystem::lookAt(entity_, target, up, *scene_);
		}
		void TransformComponent::lookAtLocal(const glm::vec3& target, glm::vec3 up)
		{
			TransformSystem::lookAtLocal(entity_, target, up, *scene_);
		}
	}
}