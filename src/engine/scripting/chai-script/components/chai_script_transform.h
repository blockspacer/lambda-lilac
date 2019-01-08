#pragma once
#include "../chai_script_entity.h"
#include "../../script_vector.h"
#include "systems/transform_system.h"

namespace lambda
{
  namespace scripting
  {
    class ChaiTransform : public ChaiComponent
    {
    public:
      ChaiTransform(world::IWorld* world, ChaiGameObject* game_object);
      virtual ~ChaiTransform();
      
      virtual void Begin() final override {};
      virtual void End() final override {};

      // TODO (Hilze): Implement.
      //bool hasParent() const;
      //TransformComponent getParent() const;
      //void setParent(const TransformComponent& parent);
      //void unsetParent();
      //Vector<TransformComponent> getChildren() const;

      void setLocalTranslation(const ScriptVec3& translation);
      void setLocalRotation(const ScriptQuat& rotation);
      void setLocalRotation(const ScriptVec3& euler);
      void setLocalScale(const ScriptVec3& scale);
      ScriptVec3 getLocalTranslation() const;
      ScriptQuat getLocalRotation() const;
      ScriptVec3 getLocalScale() const;
      void moveLocal(const ScriptVec3& delta);
      void rotateLocal(const ScriptQuat& delta);
      void scaleLocal(const ScriptVec3& delta);

      void setWorldTranslation(const ScriptVec3& translation);
      void setWorldRotation(const ScriptQuat& rotation);
      void setWorldRotation(const ScriptVec3& euler);
      void setWorldScale(const ScriptVec3& scale);
      ScriptVec3 getWorldTranslation() const;
      ScriptQuat getWorldRotation() const;
      ScriptVec3 getWorldScale() const;
      void moveWorld(const ScriptVec3& delta);
      void rotateWorld(const ScriptQuat& delta);
      void scaleWorld(const ScriptVec3& delta);

      ScriptVec3 transformPoint(const ScriptVec3& point) const;
      ScriptVec3 transformVector(const ScriptVec3& vector) const;
      ScriptVec3 transformDirection(const ScriptVec3& vector) const;
      ScriptVec3 transformLocalPoint(const ScriptVec3& point) const;
      ScriptVec3 transformLocalVector(const ScriptVec3& vector) const;
      ScriptVec3 transformLocalDirection(const ScriptVec3& vector) const;
      ScriptVec3 inverseTransformPoint(const ScriptVec3& point) const;
      ScriptVec3 inverseTransformVector(const ScriptVec3& vector) const;

      ScriptVec3 getWorldForward() const;
      ScriptVec3 getWorldUp() const;
      ScriptVec3 getWorldRight() const;

      void lookAt(const ScriptVec3& target, ScriptVec3 up);
      void lookAtLocal(const ScriptVec3& target, ScriptVec3 up);

    private:
      components::TransformComponent transform_;
    };
  }
}