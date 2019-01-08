#include "chai_script_transform.h"
#include "interfaces/iworld.h"
#include "platform/scene.h"
#include "systems/entity_system.h"

namespace lambda
{
  namespace scripting
  {
    ChaiTransform::ChaiTransform(world::IWorld* world, ChaiGameObject* game_object) : 
      ChaiComponent(game_object)
    {
      world::Scene& scene = world->getScene();
      transform_ = scene.getSystem<components::TransformSystem>()->addComponent(
        entity::Entity(
          game_object->GetId(),
          scene.getSystem<entity::EntitySystem>().get()
        )
      );
    }
    ChaiTransform::~ChaiTransform()
    {
      transform_.getSystem()->removeComponent(transform_.entity());
    }
    void ChaiTransform::setLocalTranslation(const ScriptVec3& translation)
    {
      transform_.setLocalTranslation(translation);
    }
    void ChaiTransform::setLocalRotation(const ScriptQuat& rotation)
    {
      transform_.setLocalRotation(rotation);
    }
    void ChaiTransform::setLocalRotation(const ScriptVec3& euler)
    {
      transform_.setLocalRotation(euler);
    }
    void ChaiTransform::setLocalScale(const ScriptVec3& scale)
    {
      transform_.setLocalScale(scale);
    }
    ScriptVec3 ChaiTransform::getLocalTranslation() const
    {
      return transform_.getLocalTranslation();
    }
    ScriptQuat ChaiTransform::getLocalRotation() const
    {
      return transform_.getLocalRotation();
    }
    ScriptVec3 ChaiTransform::getLocalScale() const
    {
      return transform_.getLocalScale();
    }
    void ChaiTransform::moveLocal(const ScriptVec3& delta)
    {
      transform_.moveLocal(delta);
    }
    void ChaiTransform::rotateLocal(const ScriptQuat& delta)
    {
      transform_.rotateLocal(delta);
    }
    void ChaiTransform::scaleLocal(const ScriptVec3& delta)
    {
      transform_.scaleLocal(delta);
    }
    void ChaiTransform::setWorldTranslation(const ScriptVec3& translation)
    {
      transform_.setWorldTranslation(translation);
    }
    void ChaiTransform::setWorldRotation(const ScriptQuat& rotation)
    {
      transform_.setWorldRotation(rotation);
    }
    void ChaiTransform::setWorldRotation(const ScriptVec3& euler)
    {
      transform_.setWorldRotation(euler);
    }
    void ChaiTransform::setWorldScale(const ScriptVec3& scale)
    {
      transform_.setWorldScale(scale);
    }
    ScriptVec3 ChaiTransform::getWorldTranslation() const
    {
      return transform_.getWorldTranslation();
    }
    ScriptQuat ChaiTransform::getWorldRotation() const
    {
      return transform_.getWorldRotation();
    }
    ScriptVec3 ChaiTransform::getWorldScale() const
    {
      return transform_.getWorldScale();
    }
    void ChaiTransform::moveWorld(const ScriptVec3& delta)
    {
      transform_.moveWorld(delta);
    }
    void ChaiTransform::rotateWorld(const ScriptQuat& delta)
    {
      transform_.rotateWorld(delta);
    }
    void ChaiTransform::scaleWorld(const ScriptVec3& delta)
    {
      transform_.scaleWorld(delta);
    }
    ScriptVec3 ChaiTransform::transformPoint(const ScriptVec3& point) const
    {
      return transform_.transformPoint(point);
    }
    ScriptVec3 ChaiTransform::transformVector(const ScriptVec3& vector) const
    {
      return transform_.transformVector(vector);
    }
    ScriptVec3 ChaiTransform::transformDirection(const ScriptVec3& vector) const
    {
      return transform_.transformDirection(vector);
    }
    ScriptVec3 ChaiTransform::transformLocalPoint(const ScriptVec3& point) const
    {
      return transform_.transformLocalPoint(point);
    }
    ScriptVec3 ChaiTransform::transformLocalVector(const ScriptVec3& vector) const
    {
      return transform_.transformLocalVector(vector);
    }
    ScriptVec3 ChaiTransform::transformLocalDirection(const ScriptVec3& vector) const
    {
      return transform_.transformLocalDirection(vector);
    }
    ScriptVec3 ChaiTransform::inverseTransformPoint(const ScriptVec3& point) const
    {
      return transform_.inverseTransformPoint(point);
    }
    ScriptVec3 ChaiTransform::inverseTransformVector(const ScriptVec3& vector) const
    {
      return transform_.inverseTransformVector(vector);
    }
    ScriptVec3 ChaiTransform::getWorldForward() const
    {
      return transform_.getWorldForward();
    }
    ScriptVec3 ChaiTransform::getWorldUp() const
    {
      return transform_.getWorldUp();
    }
    ScriptVec3 ChaiTransform::getWorldRight() const
    {
      return transform_.getWorldRight();
    }
    void ChaiTransform::lookAt(const ScriptVec3& target, ScriptVec3 up)
    {
      transform_.lookAt(target, up);
    }
    void ChaiTransform::lookAtLocal(const ScriptVec3& target, ScriptVec3 up)
    {
      transform_.lookAtLocal(target, up);
    }
  }
}