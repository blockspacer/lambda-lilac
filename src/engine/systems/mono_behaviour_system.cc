#include "mono_behaviour_system.h"
#include "interfaces/iworld.h"
#include "interfaces/iscript_context.h"

namespace lambda
{
  namespace components
  {
    MonoBehaviourComponent MonoBehaviourSystem::addComponent(const entity::Entity& entity)
    {
      data_.push_back(MonoBehaviourData(entity));
      data_to_entity_[(uint32_t)data_.size() - 1u] = entity;
      entity_to_data_[entity] = (uint32_t)data_.size() - 1u;

      //MonoBehaviourData& data = lookUpData(entity);
      return MonoBehaviourComponent(entity, this);
    }
    MonoBehaviourComponent MonoBehaviourSystem::getComponent(const entity::Entity& entity)
    {
      return MonoBehaviourComponent(entity, this);
    }
    bool MonoBehaviourSystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity) != entity_to_data_.end();
    }
    void MonoBehaviourSystem::removeComponent(const entity::Entity& entity)
    {
      const auto& it = entity_to_data_.find(entity);
      if (it != entity_to_data_.end())
      {
        {
          MonoBehaviourData& data = data_.at(it->second);
#define FREE(x) if (x) world_->getScripting()->freeHandle(x)
          FREE(data.object);
          FREE(data.initialize);
          FREE(data.deinitialize);
          FREE(data.update);
          FREE(data.fixed_update);
          FREE(data.on_collision_enter);
          FREE(data.on_collision_exit);
          FREE(data.on_trigger_enter);
          FREE(data.on_trigger_exit);
        }
        
        uint32_t id = it->second;

        for (auto i = data_to_entity_.find(id); i != data_to_entity_.end(); i++)
        {
          entity_to_data_.at(i->second)--;
        }

        data_.erase(data_.begin() + id);
        entity_to_data_.erase(it);
        data_to_entity_.erase(id);
      }
    }
    void MonoBehaviourSystem::initialize(world::IWorld& world)
    {
      world_ = &world;
    }
    void MonoBehaviourSystem::deinitialize()
    {
      world_ = nullptr;
    }
    void MonoBehaviourSystem::update(const double& delta_time)
    {
      for (const auto& data : data_)
        if (data.object && data.update)
          world_->getScripting()->executeFunction(data.object, data.update, {});
    }
    void MonoBehaviourSystem::fixedUpdate(const double& delta_time)
    {
      for (const auto& data : data_)
        if (data.object && data.fixed_update)
          world_->getScripting()->executeFunction(data.object, data.fixed_update, {});
    }
    void MonoBehaviourSystem::setObject(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).object = ptr;
    }
    void MonoBehaviourSystem::setInitialize(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).initialize = ptr;
    }
    void MonoBehaviourSystem::setDeinitialize(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).deinitialize = ptr;
    }
    void MonoBehaviourSystem::setUpdate(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).update = ptr;
    }
    void MonoBehaviourSystem::setFixedUpdate(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).fixed_update = ptr;
    }
    void MonoBehaviourSystem::setOnCollisionEnter(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).on_collision_enter = ptr;
    }
    void MonoBehaviourSystem::setOnCollisionExit(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).on_collision_exit = ptr;
    }
    void MonoBehaviourSystem::setOnTriggerEnter(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).on_trigger_enter = ptr;
    }
    void MonoBehaviourSystem::setOnTriggerExit(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).on_trigger_exit = ptr;
    }
    void* MonoBehaviourSystem::getObject(const entity::Entity& entity) const
    {
      return lookUpData(entity).object;
    }
    void* MonoBehaviourSystem::getInitialize(const entity::Entity& entity) const
    {
      return lookUpData(entity).initialize;
    }
    void* MonoBehaviourSystem::getDeinitialize(const entity::Entity& entity) const
    {
      return lookUpData(entity).deinitialize;
    }
    void* MonoBehaviourSystem::getUpdate(const entity::Entity& entity) const
    {
      return lookUpData(entity).update;
    }
    void* MonoBehaviourSystem::getFixedUpdate(const entity::Entity& entity) const
    {
      return lookUpData(entity).fixed_update;
    }
    void* MonoBehaviourSystem::getOnCollisionEnter(const entity::Entity& entity) const
    {
      return lookUpData(entity).on_collision_enter;
    }
    void* MonoBehaviourSystem::getOnCollisionExit(const entity::Entity& entity) const
    {
      return lookUpData(entity).on_collision_exit;
    }
    void* MonoBehaviourSystem::getOnTriggerEnter(const entity::Entity& entity) const
    {
      return lookUpData(entity).on_trigger_enter;
    }
    void* MonoBehaviourSystem::getOnTriggerExit(const entity::Entity& entity) const
    {
      return lookUpData(entity).on_trigger_exit;
    }
    MonoBehaviourData& MonoBehaviourSystem::lookUpData(const entity::Entity& entity)
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
      return data_.at(entity_to_data_.at(entity));
    }
    const MonoBehaviourData& MonoBehaviourSystem::lookUpData(const entity::Entity& entity) const
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
      return data_.at(entity_to_data_.at(entity));
    }
    MonoBehaviourComponent::MonoBehaviourComponent(const entity::Entity& entity, MonoBehaviourSystem* system) :
      IComponent(entity), system_(system)
    {
    }
    MonoBehaviourComponent::MonoBehaviourComponent(const MonoBehaviourComponent& other) :
      IComponent(other.entity_), system_(other.system_)
    {
    }
    MonoBehaviourComponent::MonoBehaviourComponent() :
      IComponent(entity::Entity()), system_(nullptr)
    {
    }
    MonoBehaviourData::MonoBehaviourData(const MonoBehaviourData & other)
    {
      object             = other.object;
      initialize         = other.initialize;
      deinitialize       = other.deinitialize;
      update             = other.update;
      fixed_update       = other.fixed_update;
      on_collision_enter = other.on_collision_enter;
      on_collision_exit  = other.on_collision_exit;
      on_trigger_enter   = other.on_trigger_enter;
      on_trigger_exit    = other.on_trigger_exit;
      entity             = other.entity;
    }
    MonoBehaviourData & MonoBehaviourData::operator=(const MonoBehaviourData & other)
    {
      object             = other.object;
      initialize         = other.initialize;
      deinitialize       = other.deinitialize;
      update             = other.update;
      fixed_update       = other.fixed_update;
      on_collision_enter = other.on_collision_enter;
      on_collision_exit  = other.on_collision_exit;
      on_trigger_enter   = other.on_trigger_enter;
      on_trigger_exit    = other.on_trigger_exit;
      entity             = other.entity;

      return *this;
    }
}
}
