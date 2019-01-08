#pragma once
#include "interfaces/isystem.h"
#include "interfaces/icomponent.h"
#include <containers/containers.h>
#include <memory/memory.h>

namespace lambda
{
  namespace components
  {
    class MonoBehaviourSystem;
    
    class MonoBehaviourComponent : public IComponent
    {
    public:
      MonoBehaviourComponent(const entity::Entity& entity, MonoBehaviourSystem* system);
      MonoBehaviourComponent(const MonoBehaviourComponent& other);
      MonoBehaviourComponent();

    private:
      MonoBehaviourSystem* system_;
    };

    struct MonoBehaviourData
    {
      MonoBehaviourData(const entity::Entity& entity) : entity(entity) {};
      MonoBehaviourData(const MonoBehaviourData& other);
      MonoBehaviourData& operator=(const MonoBehaviourData& other);

      void* object = nullptr;
      
      void* initialize = nullptr;
      void* deinitialize = nullptr;
      
      void* update = nullptr;
      void* fixed_update = nullptr;

      void* on_collision_enter = nullptr;
      void* on_collision_exit = nullptr;
      
      void* on_trigger_enter = nullptr;
      void* on_trigger_exit = nullptr;

      entity::Entity entity;
    };

    class MonoBehaviourSystem : public ISystem
    {
    public:
      static size_t systemId() { return (size_t)SystemIds::kMonoBehaviourSystem; };
      MonoBehaviourComponent addComponent(const entity::Entity& entity);
      MonoBehaviourComponent getComponent(const entity::Entity& entity);
      bool hasComponent(const entity::Entity& entity);
      void removeComponent(const entity::Entity& entity);
      virtual void initialize(world::IWorld& world) override;
      virtual void deinitialize() override;
      virtual void update(const double& delta_time) override;
      virtual void fixedUpdate(const double& delta_time) override;
      virtual ~MonoBehaviourSystem() override {};
      
      void setObject(const entity::Entity& entity, void* ptr);
      void setInitialize(const entity::Entity& entity, void* ptr);
      void setDeinitialize(const entity::Entity& entity, void* ptr);
      void setUpdate(const entity::Entity& entity, void* ptr);
      void setFixedUpdate(const entity::Entity& entity, void* ptr);
      void setOnCollisionEnter(const entity::Entity& entity, void* ptr);
      void setOnCollisionExit(const entity::Entity& entity, void* ptr);
      void setOnTriggerEnter(const entity::Entity& entity, void* ptr);
      void setOnTriggerExit(const entity::Entity& entity, void* ptr);

      void* getObject(const entity::Entity& entity) const;
      void* getInitialize(const entity::Entity& entity) const;
      void* getDeinitialize(const entity::Entity& entity) const;
      void* getUpdate(const entity::Entity& entity) const;
      void* getFixedUpdate(const entity::Entity& entity) const;
      void* getOnCollisionEnter(const entity::Entity& entity) const;
      void* getOnCollisionExit(const entity::Entity& entity) const;
      void* getOnTriggerEnter(const entity::Entity& entity) const;
      void* getOnTriggerExit(const entity::Entity& entity) const;

    protected:
      MonoBehaviourData& lookUpData(const entity::Entity& entity);
      const MonoBehaviourData& lookUpData(const entity::Entity& entity) const;

    private:
      Vector<MonoBehaviourData> data_;
      Map<uint64_t, uint32_t> entity_to_data_;
      Map<uint32_t, uint64_t> data_to_entity_;

      world::IWorld* world_;
    };
  }
}