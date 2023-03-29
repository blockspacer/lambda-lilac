/** @file transform.as */
#include "icomponent.as"

/**
* @addtogroup Components
* @{
**/

/**
* @class Transform
* @brief A transform component which can be attached to an entity
* @author Hilze Vonck
**/
class Transform : IComponent
{
    void Initialize() final
    {
        Violet_Components_Transform::Create(GetId());
    }
    void Destroy() final
    {
        Violet_Components_Transform::Destroy(GetId());
    }
    /**
    * @brief Moves the transform locally
    * @param m (const Vec3&) The local offset
    * @public
    **/
    void MoveLocal(const Vec3&in m)
    {
        SetLocalPosition(GetLocalPosition() + m);
    }
    /**
    * @brief Scales the transform locally
    * @param s (const Vec3&) The local scale
    * @public
    **/
    void ScaleLocal(const Vec3&in s)
    {
        SetLocalScale(GetLocalScale() * s);
    }
    /**
    * @brief Moves the transform in the world
    * @param m (const Vec3&) The world offset
    * @public
    **/
    void MoveWorld(const Vec3&in m)
    {
        SetWorldPosition(GetWorldPosition() + m);
    }
    /**
    * @brief Scales the transform in the world
    * @param s (const Vec3&) The world scale
    * @public
    **/
    void ScaleWorld(const Vec3&in s)
    {
        SetWorldScale(GetWorldScale() * s);
    }
    /**
    * @brief Returns the world position
    * @return (Vec3) The world position
    * @public
    **/
    Vec3 GetWorldPosition() const
    {
        return Violet_Components_Transform::GetWorldPosition(GetId());
    }
    /**
    * @brief Sets the world position
    * @param p (const Vec3&) The new world position
    * @public
    **/
    void SetWorldPosition(const Vec3&in p) const
    {
        Violet_Components_Transform::SetWorldPosition(GetId(), p);
    }
    /**
    * @brief Returns the local position
    * @return (Vec3) The local position
    * @public
    **/
    Vec3 GetLocalPosition() const
    {
        return Violet_Components_Transform::GetLocalPosition(GetId());
    }
    /**
    * @brief Sets the local position
    * @param p (const Vec3&) The new local position
    * @public
    **/
    void SetLocalPosition(const Vec3&in p) const
    {
        Violet_Components_Transform::SetLocalPosition(GetId(), p);
    }
    /**
    * @brief Returns the world  rotation
    * @return (Quat) The world  rotation
    * @public
    **/
    Quat GetWorldRotation() const
    {
        return Violet_Components_Transform::GetWorldRotation(GetId());
    }
    /**
    * @brief Sets the world  rotation
    * @param r (const Quat&) The new world  rotation
    * @public
    **/
    void SetWorldRotation(const Quat&in r) const
    {
        Violet_Components_Transform::SetWorldRotation(GetId(), r);
    }
    /**
    * @brief Returns the local  rotation
    * @return (Quat) The local  rotation
    * @public
    **/
    Quat GetLocalRotation() const
    {
        return Violet_Components_Transform::GetLocalRotation(GetId());
    }
    /**
    * @brief Sets the world local rotation
    * @param r (const Quat&) The new local  rotation
    * @public
    **/
    void SetLocalRotation(const Quat&in r) const
    {
        Violet_Components_Transform::SetLocalRotation(GetId(), r);
    }
    /**
    * @brief Returns the world  rotation
    * @return (Vec3) The world  rotation
    * @public
    **/
    Vec3 GetWorldRotationEuler() const
    {
        return Violet_Components_Transform::GetWorldRotationEuler(GetId());
    }
    /**
    * @brief Sets the world euler rotation
    * @param r (const Vec3&) The new world euler rotation
    * @public
    **/
    void SetWorldRotationEuler(const Vec3&in r) const
    {
        Violet_Components_Transform::SetWorldRotationEuler(GetId(), r);
    }
    /**
    * @brief Returns the world scale
    * @return (Vec3) The world scales
    * @public
    **/
    Vec3 GetWorldScale() const
    {
        return Violet_Components_Transform::GetWorldScale(GetId());
    }
    /**
    * @brief Sets the world scale
    * @param s (const Vec3&) The new world scales
    * @public
    **/
    void SetWorldScale(const Vec3&in s) const
    {
        Violet_Components_Transform::SetWorldScale(GetId(), s);
    }
    /**
    * @brief Returns the local euler rotation
    * @return (Vec3) The local euler rotation
    * @public
    **/
    Vec3 GetLocalRotationEuler() const
    {
        return Violet_Components_Transform::GetLocalRotationEuler(GetId());
    }
    /**
    * @brief Sets the world local rotation
    * @param r (const Vec3&) The new local euler rotation
    * @public
    **/
    void SetLocalRotationEuler(const Vec3&in r) const
    {
        Violet_Components_Transform::SetLocalRotationEuler(GetId(), r);
    }
    /**
    * @brief Returns the local scale
    * @return (Vec3) The local scales
    * @public
    **/
    Vec3 GetLocalScale() const
    {
        return Violet_Components_Transform::GetLocalScale(GetId());
    }
    /**
    * @brief Sets the local scale
    * @param s (const Vec3&) The local scales
    * @public
    **/
    void SetLocalScale(const Vec3&in s) const
    {
        Violet_Components_Transform::SetLocalScale(GetId(), s);
    }
    /**
    * @brief Sets the parent of this transform
    * @param parent (const Entity&) The new parent
    * @public
    **/
    void SetParent(const Transform@ parent) const
    {
      Violet_Components_Transform::SetParent(GetId(), parent.GetId());
    }
}

/**
* @}
**/
