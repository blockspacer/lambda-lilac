#include "chai_script_entity.h"
#include "systems/entity_system.h"

namespace lambda
{
  namespace scripting
  {
    ChaiComponent::ChaiComponent(ChaiGameObject* game_object) :
      game_object_(game_object)
    {
    }
    ChaiComponent::~ChaiComponent()
    {
    }
    ChaiGameObject* ChaiComponent::GetGameObject() const
    {
      return game_object_;
    }
    void ChaiGameObject::destroy()
    {
      // Remove components.
      for (const auto& component : data_->components)
      {
        component.second->End();
        foundation::Memory::destruct(component.second);
      }
      data_->components.clear();

      // Delete the data.
      foundation::Memory::destruct(data_);
      data_ = nullptr;
    }
    ChaiGameObject::ChaiGameObject(size_t id) :
      data_(foundation::Memory::construct<Data>())
    {
      data_->id = id;
    }
    ChaiGameObject::ChaiGameObject(const ChaiGameObject& other) :
      data_(other.data_)
    {
    }
    void ChaiGameObject::operator=(const ChaiGameObject& other)
    {
      data_ = other.data_;
    }
    ChaiGameObject::~ChaiGameObject()
    {
    }
    size_t ChaiGameObject::GetId() const
    {
      return data_->id;
    }
  }
}