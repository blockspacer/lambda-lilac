#pragma once
#include "systems/entity.h"

namespace lambda
{
  namespace components
  {
    ///////////////////////////////////////////////////////////////////////////
    class IComponent
    {
    public:
      IComponent(const entity::Entity& entity);

      bool operator==(const IComponent& other) const;
      bool operator!=(const IComponent& other) const;

      entity::Entity entity() const;

    protected:
      entity::Entity entity_;
    };
  }
}