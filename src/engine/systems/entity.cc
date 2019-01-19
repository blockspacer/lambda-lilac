#include "entity.h"
#include "entity_system.h"

namespace lambda
{
  namespace entity
  {
    //Entity::Entity() :
    //  id_(0), system_(nullptr)
    //{
    //}
    //Entity::Entity(size_t id, EntitySystem* system) :
    //  id_(id), system_(system)
    //{
    //}

    //Entity::Entity(const Entity& other) :
    //  id_(other.id_), system_(other.system_)
    //{
    //}

    //size_t Entity::id() const
    //{
    //  return id_;
    //}

    //Entity Entity::invalid()
    //{
    //  return Entity(0, nullptr);
    //}

    //bool Entity::isAlive() const
    //{
    //  //return id_ != 0u;

    //  if (system_ != nullptr)
    //  {
    //    return system_->isAlive(*this);
    //  }

    //  return false;
    //}

    //Entity::operator bool() const
    //{
    //  return isAlive();
    //}
    //bool Entity::operator==(const Entity& other) const
    //{
    //  return id_ == other.id_;
    //}
    //bool Entity::operator!=(const Entity& other) const
    //{
    //  return id_ != other.id_;
    //}
  }
}