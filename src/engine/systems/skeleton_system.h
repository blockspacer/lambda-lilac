#pragma once
#include "interfaces/icomponent.h"
#include "interfaces/isystem.h"
#include "assets/mesh.h"

namespace lambda
{
  namespace components
  {
    class SkeletonSystem;

    class SkeletonComponent : public IComponent
    {
    public:
      SkeletonComponent(const entity::Entity& entity, SkeletonSystem* system);
      SkeletonComponent(const SkeletonComponent& other);
      SkeletonComponent();

    private:
      SkeletonSystem* system_;
    };

    struct SkeletonData
    {
      SkeletonData(const entity::Entity& entity) : entity(entity) {};
      SkeletonData(const SkeletonData& other);
      SkeletonData& operator=(const SkeletonData& other);

      asset::MeshHandle mesh;
      entity::Entity entity;
    };

    class SkeletonSystem : public ISystem
    {
    public:
      static size_t systemId() { return (size_t)SystemIds::kSkeletonSystem; };
      SkeletonComponent addComponent(const entity::Entity& entity);
      SkeletonComponent getComponent(const entity::Entity& entity);
      bool hasComponent(const entity::Entity& entity);
      void removeComponent(const entity::Entity& entity);
      virtual void initialize(world::IWorld& world) override;
      virtual void deinitialize() override;
      virtual void update(const double& delta_time) override;
      virtual void fixedUpdate(const double& time_step) override;
      virtual ~SkeletonSystem() override {};

    protected:
      SkeletonData& lookUpData(const entity::Entity& entity);
      const SkeletonData& lookUpData(const entity::Entity& entity) const;

    private:
      Vector<SkeletonData> data_;
      Map<uint64_t, uint32_t> entity_to_data_;
      Map<uint32_t, uint64_t> data_to_entity_;
    };
  }
}