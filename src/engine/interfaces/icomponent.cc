#include "icomponent.h"
#include "systems/entity.h"

namespace lambda
{
  namespace components
  {
    ///////////////////////////////////////////////////////////////////////////
    IComponent::IComponent(const entity::Entity& entity) 
      : entity_(entity)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    bool IComponent::operator==(const IComponent& other) const
    {
      return entity_ == other.entity_;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool IComponent::operator!=(const IComponent& other) const
    {
      return entity_ != other.entity_;
    }

    ///////////////////////////////////////////////////////////////////////////
    entity::Entity IComponent::entity() const
    {
      return entity_;
    }
  }
}