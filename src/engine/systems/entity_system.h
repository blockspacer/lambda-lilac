#pragma once
#include "entity.h"
#include "interfaces/isystem.h"
#include <containers/containers.h>

namespace lambda
{
  namespace entity
  {
    class EntitySystem : public components::ISystem
    {
    public:
      static size_t systemId() { return (size_t)components::SystemIds::kEntitySystem; };

      Entity createEntity();
      void destroyEntity(const Entity& entity);
      bool isAlive(const Entity& entity);

    private:
      virtual void initialize(world::IWorld&) override {};
      virtual void deinitialize() override {};

    private:
      static const uint32_t kFreeIdIncrement = 64u;
			uint32_t free_id_count_ = 1;
      Queue<uint32_t> free_ids_;
    };
  }
}