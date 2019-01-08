#include "lod_system.h"
#include <algorithm>

namespace lambda
{
  namespace components
  {
    void LOD::setMesh(asset::MeshHandle mesh)
    {
      mesh_ = mesh;
    }
    void LOD::setDistance(const float& distance)
    {
      distance_ = distance;
    }
    asset::MeshHandle LOD::getMesh() const
    {
      return mesh_;
    }
    float LOD::getDistance() const
    {
      return distance_;
    }
    LODComponent::LODComponent(const entity::Entity& entity, LODSystem* system) :
      IComponent(entity), system_(system)
    {
    }
    LODComponent::LODComponent(const LODComponent& other) :
      IComponent(other.entity_), system_(other.system_)
    {
    }
    LODComponent::LODComponent() :
      IComponent(entity::Entity()), system_(nullptr)
    {
    }
    void LODComponent::setBaseLOD(const LOD& lod)
    {
      system_->setBaseLOD(entity_, lod);
    }
    LOD LODComponent::getBaseLOD() const
    {
      return system_->getBaseLOD(entity_);
    }
    void LODComponent::addLOD(const LOD& lod)
    {
      return system_->addLOD(entity_, lod);
    }
    Vector<LOD> LODComponent::getLODs() const
    {
      return system_->getLODs(entity_);
    }
    LODComponent LODSystem::addComponent(const entity::Entity& entity)
    {
      require(mesh_render_system_.get(), entity);
      require(transform_system_.get(), entity);

      data_.push_back(LODData(entity));
      data_to_entity_[(uint32_t)data_.size() - 1u] = entity.id();
      entity_to_data_[entity.id()] = (uint32_t)data_.size() - 1u;

      auto& data =lookUpData(entity);
      data.base_lod.setDistance(0.0f);
      data.base_lod.setMesh(mesh_render_system_->getMesh(entity));

      return LODComponent(entity, this);
    }
    LODComponent LODSystem::getComponent(const entity::Entity& entity)
    {
      return LODComponent(entity, this);
    }
    bool LODSystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity.id()) != entity_to_data_.end();
    }
    void LODSystem::removeComponent(const entity::Entity& entity)
    {
      const auto& it = entity_to_data_.find(entity.id());
      if (it != entity_to_data_.end())
      {
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
    void LODSystem::initialize(world::IWorld& world)
    {
      transform_system_   = world.getScene().getSystem<TransformSystem>();
      mesh_render_system_ = world.getScene().getSystem<MeshRenderSystem>();
      camera_system_      = world.getScene().getSystem<CameraSystem>();
    }
    void LODSystem::deinitialize()
    {
    }
    void LODSystem::update(const double& delta_time)
    {
      // Do not update the LODs every frame. Just not worth it.
      time_ += delta_time;
      if (time_ < update_frequency_)
      {
        return;
      }
      time_ -= update_frequency_;


      // Update LODs.
      glm::vec3 camera_position = transform_system_->getWorldTranslation(camera_system_->getMainCamera());

      for (LODData& data : data_)
      {
        LOD* chosen_lod =&data.base_lod;
        float distance = glm::length(transform_system_->getWorldTranslation(data.entity) - camera_position);
        
        for (LOD& lod : data.lods)
        {
          if (distance > lod.getDistance())
          {
            chosen_lod =&lod;
            break;
          }
        }

        mesh_render_system_->setMesh(data.entity, chosen_lod->getMesh());
      }
    }
    void LODSystem::setBaseLOD(const entity::Entity& entity, const LOD& lod)
    {
      lookUpData(entity).base_lod = lod;
    }
    void LODSystem::addLOD(const entity::Entity& entity, const LOD& lod)
    {
      auto& data = lookUpData(entity);
      data.lods.push_back(lod);

      std::sort(data.lods.begin(), data.lods.end(), std::greater<LOD>());
    }
    LOD LODSystem::getBaseLOD(const entity::Entity& entity) const
    {
      return lookUpData(entity).base_lod;
    }
    Vector<LOD> LODSystem::getLODs(const entity::Entity& entity) const
    {
      return lookUpData(entity).lods;
    }
    LODData& LODSystem::lookUpData(const entity::Entity& entity)
    {
      LMB_ASSERT(entity_to_data_.find(entity.id()) != entity_to_data_.end(), "LOD: could not find component");
      return data_.at(entity_to_data_.at(entity.id()));
    }
    const LODData& LODSystem::lookUpData(const entity::Entity& entity) const
    {
      LMB_ASSERT(entity_to_data_.find(entity.id()) != entity_to_data_.end(), "LOD: could not find component");
      return data_.at(entity_to_data_.at(entity.id()));
    }
    LODData::LODData(const LODData & other)
    {
      lods     = other.lods;
      base_lod = other.base_lod;
      entity   = other.entity;
    }
    LODData & LODData::operator=(const LODData & other)
    {
      lods     = other.lods;
      base_lod = other.base_lod;
      entity   = other.entity;
      
      return *this;
    }
}
}