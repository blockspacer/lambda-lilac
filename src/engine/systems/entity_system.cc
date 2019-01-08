#include "entity_system.h"

namespace lambda
{
  namespace entity
  {
    Entity EntitySystem::createEntity()
    {
      if (free_ids_.empty() == true)
      {
        for (size_t i = 0; i < kFreeIdIncrement; ++i)
        {
          free_ids_.push(free_id_count_ + i);
        }
        free_id_count_ += kFreeIdIncrement;
      }

      size_t id = free_ids_.front();
      free_ids_.pop();

      return Entity(id, this);
    }

    void EntitySystem::destroyEntity(const Entity& entity)
    {
      free_ids_.push(entity.id_);
    }

    bool EntitySystem::isAlive(const Entity& entity)
    {
      return entity.id_ > 0;
    }
  }
}
