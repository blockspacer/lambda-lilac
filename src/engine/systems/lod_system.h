#pragma once
#include "interfaces/icomponent.h"
#include "interfaces/isystem.h"
#include "assets/mesh.h"
#include "systems/mesh_render_system.h"
#include "systems/transform_system.h"
#include "systems/camera_system.h"

namespace lambda
{
  namespace components
  {
    class LODSystem;

    class LOD
    {
    public:
      void setMesh(asset::MeshHandle mesh);
      void setDistance(const float& distance);
      asset::MeshHandle getMesh() const;
      float getDistance() const;

      bool operator<(const LOD& other) const
      {
        return distance_ < other.distance_;
      }
      bool operator>(const LOD& other) const
      {
        return distance_ > other.distance_;
      }

    private:
      asset::MeshHandle mesh_;
      float distance_; // Distance AFTER which this LOD should be used.
    };

    class LODComponent : public IComponent
    {
    public:
      LODComponent(const entity::Entity& entity, LODSystem* system);
      LODComponent(const LODComponent& other);
      LODComponent();

      void setBaseLOD(const LOD& lod);
      LOD getBaseLOD() const;
      void addLOD(const LOD& lod);
      Vector<LOD> getLODs() const;

    private:
      LODSystem* system_;
    };

    struct LODData
    {
      LODData(const entity::Entity& entity) : entity(entity) {};
      LODData(const LODData& other);
      LODData& operator=(const LODData& other);

      Vector<LOD> lods;
      LOD base_lod;
      entity::Entity entity;
    };

    class LODSystem : public ISystem
    {
    public:
      static size_t systemId() { return (size_t)SystemIds::kLODSystem; };
      LODComponent addComponent(const entity::Entity& entity);
      LODComponent getComponent(const entity::Entity& entity);
      bool hasComponent(const entity::Entity& entity);
      void removeComponent(const entity::Entity& entity);
      virtual void initialize(world::IWorld& world) override;
      virtual void deinitialize() override;
      virtual void update(const double& delta_time) override;
      virtual ~LODSystem() override {};

      void setBaseLOD(const entity::Entity& entity, const LOD& lod);
      void addLOD(const entity::Entity& entity, const LOD& lod);
      LOD getBaseLOD(const entity::Entity& entity) const;
      Vector<LOD> getLODs(const entity::Entity& entity) const;

    protected:
      LODData& lookUpData(const entity::Entity& entity);
      const LODData& lookUpData(const entity::Entity& entity) const;

    private:
      Vector<LODData> data_;
      Map<uint64_t, uint32_t> entity_to_data_;
      Map<uint32_t, uint64_t> data_to_entity_;
      foundation::SharedPointer<TransformSystem> transform_system_;
      foundation::SharedPointer<MeshRenderSystem> mesh_render_system_;
      foundation::SharedPointer<CameraSystem> camera_system_;
      double time_ = 0.0;
      double update_frequency_ = 1.0 / 30.0;
    };
  }
}