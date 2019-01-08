#include "transform_system.h"
#include "utils/decompose_matrix.h"

namespace lambda
{
  namespace components
  {
    entity::Entity TransformSystem::root_ = entity::Entity();
    
    glm::quat TransformSystem::lookRotation(const glm::vec3& forward, const glm::vec3& up)
    {
      if (forward == glm::vec3())
      {
        return glm::quat(1, 0, 0, 0);
      }

      const glm::vec3 new_forward = glm::normalize(forward);
      const glm::vec3 new_right   = glm::normalize(glm::cross(up, new_forward));
      const glm::vec3 new_up      = glm::cross(new_forward, new_right);

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
      if ((m00 >= m11)&& (m00 >= m22))
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

    const TransformData& TransformSystem::lookUpData(const entity::Entity& entity) const
    {
      assert(entity_to_data_.find(entity.id()) != entity_to_data_.end());
      return data_.at(entity_to_data_.at(entity.id()));
    }
    TransformData& TransformSystem::lookUpData(const entity::Entity& entity)
    {
      assert(entity_to_data_.find(entity.id()) != entity_to_data_.end());
      return data_.at(entity_to_data_.at(entity.id()));
    }

    void TransformSystem::cleanIfDirty(TransformData& data)
    {
      if (data.dirty)
      {
        data.dirty = false;

        // Local.
        data.local  = glm::translate(glm::mat4(1.0f), data.translation);
        data.local *= glm::mat4_cast(data.rotation);
        data.local  = glm::scale(data.local, data.scale);
        
        // World.
        if (data.getParent() == root_ || data.getParent().id() == data.entity.id())
        {
          data.world = data.local;
        }
        else
        {
          data.world = getWorld(data.getParent()) * data.local;
        }
      }
    }

    void TransformSystem::makeDirtyRecursive(TransformData& data)
    {
      data.dirty = true;
      for (const entity::Entity& child : data.children)
      {
        makeDirtyRecursive(lookUpData(child));
      }
    }
    
    void TransformSystem::initialize(world::IWorld& world)
    {
    }

    void TransformSystem::deinitialize()
    {
    }

    TransformSystem::~TransformSystem()
    {
    }

    TransformComponent TransformSystem::addComponent(const entity::Entity& entity)
    {
      data_.push_back(TransformData(entity));
      data_to_entity_[(uint32_t)data_.size() - 1u] = entity.id();
      entity_to_data_[entity.id()] = (uint32_t)data_.size() - 1u;

      return TransformComponent(entity, this);
    }

    TransformComponent TransformSystem::getComponent(const entity::Entity& entity)
    {
      return TransformComponent(entity, this);
    }

    bool TransformSystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity.id()) != entity_to_data_.end();
    }

    void TransformSystem::removeComponent(const entity::Entity& entity)
    {
      const auto& it = entity_to_data_.find(entity.id());
      if (it != entity_to_data_.end())
      {
        uint32_t id = it->second;

        for (auto i = data_to_entity_.find(id); i != data_to_entity_.end(); i++)
        {
          entity_to_data_.at(i->second)--;
        }

        data_.erase(data_.begin() + id);
        entity_to_data_.erase(it);
        data_to_entity_.erase(id);
      }
    }

    glm::mat4 TransformSystem::getLocal(const entity::Entity& entity)
    {
      TransformData& data = lookUpData(entity);
      cleanIfDirty(data);
      return data.local;
    }

    glm::mat4 TransformSystem::getWorld(const entity::Entity& entity)
    {
      TransformData& data = lookUpData(entity);
      cleanIfDirty(data);
      return data.world;
    }

    glm::mat4 TransformSystem::getInvWorld(const entity::Entity& entity)
    {
      return glm::inverse(getWorld(entity));
    }

    bool TransformSystem::hasParent(const entity::Entity& entity) const
    {
      const TransformData& data = lookUpData(entity);
      return true == data.getParent().isAlive()&& data.getParent() != entity;
    }

    entity::Entity TransformSystem::getParent(const entity::Entity& entity)
    {
      return lookUpData(entity).getParent();
    }

    void TransformSystem::setParent(const entity::Entity& entity, const entity::Entity& parent)
    {
      TransformData& data = lookUpData(entity);
      //entity::Entity was  = data.getParent();
      if (data.getParent() != root_)
      {
        TransformData& p_data = lookUpData(data.getParent());
        eastl::remove(p_data.children.begin(), p_data.children.end(), entity);
      }
      
      data.setParent(parent);
      
      makeDirtyRecursive(data);
      
      lookUpData(parent).children.push_back(entity);
    }

    void TransformSystem::unsetParent(const entity::Entity& entity)
    {
      setParent(entity, root_);
    }

    Vector<entity::Entity> TransformSystem::getChildren(const entity::Entity& entity)
    {
      Vector<entity::Entity> children;
      for (const auto& child : lookUpData(entity).children)
      {
        children.push_back(child);
      }
      return children;
    }

    void TransformSystem::setLocalTranslation(const entity::Entity& entity, const glm::vec3& translation)
    {
      TransformData& data = lookUpData(entity);
      data.translation = translation;
      makeDirtyRecursive(data);
    }

    void TransformSystem::setLocalRotation(const entity::Entity& entity, const glm::quat& rotation)
    {
      TransformData& data = lookUpData(entity);
      data.rotation = rotation;
      makeDirtyRecursive(data);
    }

    void TransformSystem::setLocalRotation(const entity::Entity& entity, const glm::vec3& euler)
    {
      setLocalRotation(entity, glm::quat(euler));
    }

    void TransformSystem::setLocalScale(const entity::Entity& entity, const glm::vec3& scale)
    {
      TransformData& data = lookUpData(entity);
      data.scale = scale;
      makeDirtyRecursive(data);
    }

    glm::vec3 TransformSystem::getLocalTranslation(const entity::Entity& entity) const
    {
      return lookUpData(entity).translation;
    }

    glm::quat TransformSystem::getLocalRotation(const entity::Entity& entity) const
    {
      return lookUpData(entity).rotation;
    }

    glm::vec3 TransformSystem::getLocalScale(const entity::Entity& entity) const
    {
      return lookUpData(entity).scale;
    }

    void TransformSystem::moveLocal(const entity::Entity& entity, const glm::vec3& delta)
    {
      TransformData& data = lookUpData(entity);
      data.translation += delta;
      makeDirtyRecursive(data);
    }

    void TransformSystem::rotateLocal(const entity::Entity& entity, const glm::quat& delta)
    {
      TransformData& data = lookUpData(entity);
      data.rotation += delta; // TODO (Hilze): Validate this.
      makeDirtyRecursive(data);
    }

    void TransformSystem::scaleLocal(const entity::Entity& entity, const glm::vec3& delta)
    {
      TransformData& data = lookUpData(entity);
      data.scale *= delta;
      makeDirtyRecursive(data);
    }

    void TransformSystem::setWorldTranslation(const entity::Entity& entity, const glm::vec3& translation)
    {
      const TransformData& data = lookUpData(entity);
      if (false == data.getParent().isAlive() || data.getParent() == entity)
      {
        setLocalTranslation(entity, translation);
        return;
      }
      setLocalTranslation(
        entity,
        getInvWorld(getParent(entity)) * glm::vec4(translation, 1.0f)
      );
    }

    void TransformSystem::setWorldRotation(const entity::Entity& entity, const glm::quat& rotation)
    {
      const TransformData& data = lookUpData(entity);
      if (false == data.getParent().isAlive() || data.getParent() == entity)
      {
        setLocalRotation(entity, rotation);
        return;
      }

      const glm::quat parent_rotation = getWorldRotation(data.getParent());
      // q' represents the rotation from the rotation q1 to the rotation q2
      // q' = q1^-1 * q2
      
      setLocalRotation(entity, glm::inverse(parent_rotation) * rotation);
    }

    void TransformSystem::setWorldRotation(const entity::Entity& entity, const glm::vec3& euler)
    {
      setWorldRotation(entity, glm::quat(euler));
    }

    void TransformSystem::setWorldScale(const entity::Entity& entity, const glm::vec3& scale)
    {
      const TransformData& data = lookUpData(entity);
      if (false == data.getParent().isAlive() || data.getParent() == entity)
      {
        setLocalScale(entity, scale);
        return;
      }
      setLocalScale(
        entity,
        scale / getWorldScale(getParent(entity))
      );
    }

    glm::vec3 TransformSystem::getWorldTranslation(const entity::Entity& entity)
    {
      glm::vec3 scale;
      glm::vec3 translation;
      glm::quat orientation;
      utilities::decomposeMatrix(getWorld(entity), scale, orientation, translation);

      return translation;
    }

    glm::quat TransformSystem::getWorldRotation(const entity::Entity& entity)
    {
      const TransformData& data = lookUpData(entity);
      if (data.getParent().isAlive()&& entity != data.getParent())
      {
        return data.rotation * getWorldRotation(data.getParent());
      }
      else
      {
        return data.rotation;
      }
    }

    glm::vec3 TransformSystem::getWorldScale(const entity::Entity& entity)
    {
      const TransformData& data = lookUpData(entity);
      if (data.getParent().isAlive()&& entity != data.getParent())
      {
        return data.scale * getWorldScale(data.getParent());
      }
      else
      {
        return data.scale;
      }
    }



































    TransformComponent::TransformComponent(const entity::Entity& entity, TransformSystem* system) :
      IComponent(entity), system_(system)
    {
    }
    TransformComponent::TransformComponent(const TransformComponent& other) :
      IComponent(other.entity_), system_(other.system_)
    {
    }
    TransformComponent::TransformComponent() :
      IComponent(entity::Entity()), system_(nullptr)
    {
    }
    glm::mat4 TransformComponent::getLocal() const
    {
      return system_->getLocal(entity_);
    }
    glm::mat4 TransformComponent::getWorld() const
    {
      return system_->getWorld(entity_);
    }
    glm::mat4 TransformComponent::getInvWorld() const
    {
      return system_->getInvWorld(entity_);
    }
    bool TransformComponent::hasParent() const
    {
      return system_->hasParent(entity_);
    }
    TransformComponent TransformComponent::getParent() const
    {
      return system_->getComponent(system_->getParent(entity_));
    }
    void TransformComponent::setParent(const TransformComponent& parent)
    {
      system_->setParent(entity_, parent.entity());
    }
    void TransformComponent::unsetParent()
    {
      system_->unsetParent(entity_);
    }
    Vector<TransformComponent> TransformComponent::getChildren() const
    {
      Vector<TransformComponent> children;
      for (const entity::Entity& entity : system_->getChildren(entity_))
      {
        children.push_back(system_->getComponent(entity));
      }
      return children;
    }
    void TransformComponent::setLocalTranslation(const glm::vec3& translation)
    {
      system_->setLocalTranslation(entity_, translation);
    }
    void TransformComponent::setLocalRotation(const glm::quat& rotation)
    {
      system_->setLocalRotation(entity_, rotation);
    }
    void TransformComponent::setLocalRotation(const glm::vec3& euler)
    {
      system_->setLocalRotation(entity_, euler);
    }
    void TransformComponent::setLocalScale(const glm::vec3& scale)
    {
      system_->setLocalScale(entity_, scale);
    }
    glm::vec3 TransformComponent::getLocalTranslation() const
    {
      return system_->getLocalTranslation(entity_);
    }
    glm::quat TransformComponent::getLocalRotation() const
    {
      return system_->getLocalRotation(entity_);
    }
    glm::vec3 TransformComponent::getLocalScale() const
    {
      return system_->getLocalScale(entity_);
    }
    void TransformComponent::moveLocal(const glm::vec3& delta)
    {
      system_->moveLocal(entity_, delta);
    }
    void TransformComponent::rotateLocal(const glm::quat& delta)
    {
      system_->rotateLocal(entity_, delta);
    }
    void TransformComponent::scaleLocal(const glm::vec3& delta)
    {
      system_->scaleLocal(entity_, delta);
    }
    void TransformComponent::setWorldTranslation(const glm::vec3& translation)
    {
      return system_->setWorldTranslation(entity_, translation);
    }
    void TransformComponent::setWorldRotation(const glm::quat& rotation)
    {
      return system_->setWorldRotation(entity_, rotation);
    }
    void TransformComponent::setWorldRotation(const glm::vec3& euler)
    {
      setWorldRotation(glm::quat(euler));
    }
    void TransformComponent::setWorldScale(const glm::vec3& scale)
    {
      return system_->setWorldScale(entity_, scale);
    }
    glm::vec3 TransformComponent::getWorldTranslation() const
    {
      return system_->getWorldTranslation(entity_);
    }
    glm::quat TransformComponent::getWorldRotation() const
    {
      return system_->getWorldRotation(entity_);
    }
    glm::vec3 TransformComponent::getWorldScale() const
    {
      return system_->getWorldScale(entity_);
    }
    void TransformComponent::moveWorld(const glm::vec3& delta)
    {
      setWorldTranslation(getWorldTranslation() + delta);
    }
    void TransformComponent::rotateWorld(const glm::quat& delta)
    {
      setWorldRotation(getWorldRotation() * delta);
    }
    void TransformComponent::scaleWorld(const glm::vec3& delta)
    {
      setWorldScale(getWorldScale() * delta);
    }
    glm::vec3 TransformComponent::transformPoint(const glm::vec3& point) const
    {
      return getWorld() * glm::vec4(point, 1.0f);
    }
    glm::vec3 TransformComponent::transformVector(const glm::vec3& vector) const
    {
      return getWorld() * glm::vec4(vector, 0.0f);
    }
    glm::vec3 TransformComponent::transformDirection(const glm::vec3& vector) const
    {
      return getWorldRotation() * vector;
    }
    glm::vec3 TransformComponent::transformLocalPoint(const glm::vec3& point) const
    {
      return getLocal() * glm::vec4(point, 0.0f);
    }
    glm::vec3 TransformComponent::transformLocalVector(const glm::vec3& vector) const
    {
      return getLocal() * glm::vec4(vector, 0.0f);
    }
    glm::vec3 TransformComponent::transformLocalDirection(const glm::vec3& vector) const
    {
      return getLocalRotation() * vector;
    }
    glm::vec3 TransformComponent::inverseTransformPoint(const glm::vec3& point) const
    {
      return getInvWorld() * glm::vec4(point, 1.0f);
    }
    glm::vec3 TransformComponent::inverseTransformVector(const glm::vec3& vector) const
    {
      return getInvWorld() * glm::vec4(vector, 0.0f);
    }
    glm::vec3 TransformComponent::getWorldForward() const
    {
      return ((glm::mat3x3)getWorld()) * glm::vec3(0.0f, 0.0f, -1.0f);
    }
    glm::vec3 TransformComponent::getWorldUp() const
    {
      return ((glm::mat3x3)getWorld()) * glm::vec3(0.0f, 1.0f, 0.0f);
    }
    glm::vec3 TransformComponent::getWorldRight() const
    {
      return ((glm::mat3x3)getWorld()) * glm::vec3(1.0f, 0.0f, 0.0f);
    }
    void TransformComponent::lookAt(const glm::vec3& target, glm::vec3 up)
    {
      const glm::vec3 direction = glm::normalize(target - getWorldTranslation());
      setLocalRotation(system_->lookRotation(direction, up));
    }
    void TransformComponent::lookAtLocal(const glm::vec3& target, glm::vec3 up)
    {
      setLocalRotation(glm::quat_cast(glm::lookAt(target, getLocalTranslation(), up)));
    }
    TransformData::TransformData(const TransformData& other)
    {
      children    = other.children;
      parent      = other.parent;
      entity      = other.entity;
      translation = other.translation;
      rotation    = other.rotation;
      scale       = other.scale;
      local       = other.local;
      world       = other.world;
      dirty       = other.dirty;
    }
    TransformData& TransformData::operator=(const TransformData& other)
    {
      children    = other.children;
      parent      = other.parent;
      entity      = other.entity;
      translation = other.translation;
      rotation    = other.rotation;
      scale       = other.scale;
      local       = other.local;
      world       = other.world;
      dirty       = other.dirty;
      return *this;
    }
}
}
