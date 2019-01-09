/** @file rigid_body.as */
#include "enums.as"
#include "icomponent.as"

/**
* @addtogroup Components
* @{
**/

/**
* @class RigidBody
* @brief A rigid body component which you can attach to an entity
* @author Hilze Vonck
**/
class RigidBody : IComponent
{
  void Initialize() final
  {
    Violet_Components_RigidBody::Create(GetId());
  }
  void Destroy() final
  {
    Violet_Components_RigidBody::Destroy(GetId());
  }

  void ApplyImpulse(const Vec3&in impulse) const
  {
    Violet_Components_RigidBody::ApplyImpulse(GetId(), impulse);
  }

  void SetVelocity(const Vec3&in velocity) const
  {
    Violet_Components_RigidBody::SetVelocity(GetId(), velocity);
  }

  Vec3 GetVelocity()
  {
    return Violet_Components_RigidBody::GetVelocity(GetId());
  }

  void SetAngularVelocity(const Vec3&in velocity) const
  {
    Violet_Components_RigidBody::SetAngularVelocity(GetId(), velocity);
  }

  Vec3 GetAngularVelocity()
  {
    return Violet_Components_RigidBody::GetAngularVelocity(GetId());
  }

  uint8 GetVelocityConstraints()
  {
    return Violet_Components_RigidBody::GetVelocityConstraints(GetId());
  }

  void SetVelocityConstraints(const uint8&in constraints)
  {
    Violet_Components_RigidBody::SetVelocityConstraints(GetId(), constraints);
  }

  uint8 GetAngularConstraints()
  {
    return Violet_Components_RigidBody::GetAngularConstraints(GetId());
  }

  void SetAngularConstraints(const uint8&in constraints)
  {
    Violet_Components_RigidBody::SetAngularConstraints(GetId(), constraints);
  }
}

/**
* @}
**/
