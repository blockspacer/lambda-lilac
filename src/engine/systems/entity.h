#pragma once
#include <containers/containers.h>

namespace lambda
{
  namespace entity
  {
    class EntitySystem;
		typedef uint32_t Entity;
		constexpr Entity InvalidEntity = 0u;

		/*class Entity
    {
    public:
      friend class EntitySystem;

      Entity();
      Entity(size_t id, EntitySystem* system);
      Entity(const Entity& other);

      size_t id() const;

      static Entity invalid();

      bool isAlive() const;
      operator bool() const;
      bool operator==(const Entity& other) const;
      bool operator!=(const Entity& other) const;

    protected:
      size_t id_;
      EntitySystem* system_;
    };*/
  }
}

//namespace eastl
//{
//  template <>
//  struct hash<lambda::entity::Entity>
//  {
//    size_t operator()(const lambda::entity::Entity& k) const
//    {
//      return k.id();
//    }
//  };
//}