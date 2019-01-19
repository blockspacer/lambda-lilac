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
  namespace components
  {
    class TransformSystem;

    class TransformComponent : public IComponent
    {
    public:
      TransformComponent(const entity::Entity& entity, TransformSystem* system);
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

      void lookAt(const glm::vec3& target, glm::vec3 up);
      void lookAtLocal(const glm::vec3& target, glm::vec3 up);

      TransformSystem* getSystem() const { return system_; }
    private:
      TransformSystem* system_;
    };

    struct TransformData
    {
      TransformData(const entity::Entity& entity) : entity(entity) {};
      TransformData(const TransformData& other);
      TransformData& operator=(const TransformData& other);

      Vector<entity::Entity> children;
      entity::Entity entity;

      glm::vec3 translation = glm::vec3(0.0f);
      glm::quat rotation    = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
      glm::vec3 scale       = glm::vec3(1.0f);
      glm::mat4 local       = glm::mat4(1.0f);
      glm::mat4 world       = glm::mat4(1.0f);
      bool dirty            = true;

      entity::Entity getParent() const { return parent; }
      void setParent(entity::Entity p) { parent = p; }

    private:
      entity::Entity parent = entity::InvalidEntity;
    };


    class TransformSystem : public ISystem
    {
    public:
      ~TransformSystem();
      static size_t systemId() { return (size_t)SystemIds::kTransformSystem; };

      TransformComponent addComponent(const entity::Entity& entity);
      TransformComponent getComponent(const entity::Entity& entity);
      bool hasComponent(const entity::Entity& entity);
      void removeComponent(const entity::Entity& entity);

      glm::mat4 getLocal(const entity::Entity& entity);
      glm::mat4 getWorld(const entity::Entity& entity);
      glm::mat4 getInvWorld(const entity::Entity& entity);

      bool hasParent(const entity::Entity& entity) const;
      entity::Entity getParent(const entity::Entity& entity);
      void setParent(const entity::Entity& entity, const entity::Entity& parent);
      void unsetParent(const entity::Entity& entity);
      Vector<entity::Entity> getChildren(const entity::Entity& entity);

      void setLocalTranslation(const entity::Entity& entity, const glm::vec3& translation);
      void setLocalRotation(const entity::Entity& entity, const glm::quat& rotation);
      void setLocalRotation(const entity::Entity& entity, const glm::vec3& euler);
      void setLocalScale(const entity::Entity& entity, const glm::vec3& scale);
      glm::vec3 getLocalTranslation(const entity::Entity& entity) const;
      glm::quat getLocalRotation(const entity::Entity& entity) const;
      glm::vec3 getLocalScale(const entity::Entity& entity) const;
      void moveLocal(const entity::Entity& entity, const glm::vec3& delta);
      void rotateLocal(const entity::Entity& entity, const glm::quat& delta);
      void scaleLocal(const entity::Entity& entity, const glm::vec3& delta);

      void setWorldTranslation(const entity::Entity& entity, const glm::vec3& translation);
      void setWorldRotation(const entity::Entity& entity, const glm::quat& rotation);
      void setWorldRotation(const entity::Entity& entity, const glm::vec3& euler);
      void setWorldScale(const entity::Entity& entity, const glm::vec3& scale);
      glm::vec3 getWorldTranslation(const entity::Entity& entity);
      glm::quat getWorldRotation(const entity::Entity& entity);
      glm::vec3 getWorldScale(const entity::Entity& entity);

      static glm::quat lookRotation(const glm::vec3& forward, const glm::vec3& up);

    private:
      Vector<TransformData> data_;
      Map<uint64_t, uint32_t> entity_to_data_;
      Map<uint32_t, uint64_t> data_to_entity_;

      static entity::Entity root_;
      const TransformData& lookUpData(const entity::Entity& entity) const;
      TransformData& lookUpData(const entity::Entity& entity);
      void cleanIfDirty(TransformData& data);
      void makeDirtyRecursive(TransformData& data);
      
    private:
      virtual void initialize(world::IWorld& world) override;
      virtual void deinitialize() override;
    };
  }
}