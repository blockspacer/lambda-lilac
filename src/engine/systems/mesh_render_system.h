#pragma once
#include "interfaces/isystem.h"
#include "assets/mesh.h"
#include "interfaces/iworld.h"
#include "interfaces/iwindow.h"
#include "assets/mesh_io.h"
#include "utils/zone_manager.h"

namespace lambda
{
  namespace utilities
  {
    class Frustum;
    class Culler;
    struct Renderable;
    struct LinkedNode;
  }

  namespace components
  {
    class MeshRenderSystem;
    class TransformSystem;

    class MeshRenderComponent : public IComponent
    {
    public:
      MeshRenderComponent(const entity::Entity& entity, MeshRenderSystem* system);
      MeshRenderComponent(const MeshRenderComponent& other);
      MeshRenderComponent();

      void setMesh(asset::MeshHandle mesh);
      asset::MeshHandle getMesh() const;
      void setSubMesh(const uint32_t& sub_mesh);
      uint32_t getSubMesh() const;
      void setMetallicness(const float& metallicness);
      float getMetallicness() const;
      void setRoughness(const float& roughness);
      float getRoughness() const;
      void setAlbedoTexture(asset::VioletTextureHandle texture);
      asset::VioletTextureHandle getAlbedoTexture() const;
      void setNormalTexture(asset::VioletTextureHandle texture);
      asset::VioletTextureHandle getNormalTexture() const;
      void setMetallicRoughnessTexture(asset::VioletTextureHandle texture);
      asset::VioletTextureHandle getMetallicRoughnessTexture() const;
      void attachMesh(asset::MeshHandle mesh);
      bool getVisible() const;
      void setVisible(const bool& visible);
      bool getCastShadows() const;
      void setCastShadows(const bool& cast_shadows);

    private:
      MeshRenderSystem* system_;
    };

    struct MeshRenderData
    {
      MeshRenderData(const entity::Entity& entity) : entity(entity) {};
      MeshRenderData(const MeshRenderData& other);
      MeshRenderData& operator=(const MeshRenderData& other);

      asset::MeshHandle mesh;
      uint32_t sub_mesh = 0u;
      asset::VioletTextureHandle albedo_texture;
      asset::VioletTextureHandle normal_texture;
      asset::VioletTextureHandle metallic_roughness_texture;
      float metallicness = 1.0f;
      float roughness = 1.0f;
      bool visible = true;
      bool cast_shadows = true;
			bool valid = true;

      entity::Entity entity;
    };

    class MeshRenderSystem : public ISystem
    {
    public:
      ~MeshRenderSystem();
      void setMesh(const entity::Entity& entity, asset::MeshHandle mesh);
      void setSubMesh(const entity::Entity& entity, const uint32_t& sub_mesh);
      void setAlbedoTexture(const entity::Entity& entity, asset::VioletTextureHandle texture);
      void setNormalTexture(const entity::Entity& entity, asset::VioletTextureHandle texture);
      void setMetallicRoughnessTexture(const entity::Entity& entity, asset::VioletTextureHandle texture);
      void setMetallicness(const entity::Entity& entity, const float& metallicness);
      void setRoughness(const entity::Entity& entity, const float& roughness);
      asset::MeshHandle getMesh(const entity::Entity& entity);
      uint32_t getSubMesh(const entity::Entity& entity);
      asset::VioletTextureHandle getAlbedoTexture(const entity::Entity& entity);
      asset::VioletTextureHandle getNormalTexture(const entity::Entity& entity);
      asset::VioletTextureHandle getMetallicRoughnessTexture(const entity::Entity& entity);
      float getMetallicness(const entity::Entity& entity);
      float getRoughness(const entity::Entity& entity);
      void attachMesh(const entity::Entity& entity, asset::MeshHandle mesh);
      bool getVisible(const entity::Entity& entity) const;
      void setVisible(const entity::Entity& entity, const bool& visible);
      bool getCastShadows(const entity::Entity& entity) const;
      void setCastShadows(const entity::Entity& entity, const bool& cast_shadows);
      void makeStatic(const entity::Entity& entity);
      void makeDynamic(const entity::Entity& entity);

      virtual void initialize(world::IWorld& world) override;
      virtual void deinitialize() override;
      virtual void onRender() override;
			virtual void collectGarbage() override;
			void createRenderList(utilities::Culler& culler, const utilities::Frustum& frustum);
      void createSortedRenderList(utilities::LinkedNode* statics, utilities::LinkedNode* dynamics, Vector<utilities::Renderable*>& opaque, Vector<utilities::Renderable*>& alpha);
      void renderAll(utilities::Culler& culler, const utilities::Frustum& frustum, bool is_rh = true);
      void renderAll(utilities::LinkedNode* statics, utilities::LinkedNode* dynamics, bool is_rh = true);
      void renderAll(const Vector<utilities::Renderable*>& opaque, const Vector<utilities::Renderable*>& alpha, bool is_rh = true);

      static size_t systemId() { return (size_t)SystemIds::kMeshRenderSystem; };
      MeshRenderComponent addComponent(const entity::Entity& entity);
      MeshRenderComponent getComponent(const entity::Entity& entity);
      bool hasComponent(const entity::Entity& entity);
      void removeComponent(const entity::Entity& entity);
			virtual String profilerInfo() const override;

    private:
      MeshRenderData& lookUpData(const entity::Entity& entity);
      const MeshRenderData& lookUpData(const entity::Entity& entity) const;

      Vector<MeshRenderData> data_;
			Map<entity::Entity, uint32_t> entity_to_data_;
			Map<uint32_t, entity::Entity> data_to_entity_;
			Set<entity::Entity> marked_for_delete_;
			Queue<uint32_t> unused_data_entries_;

    private:
      foundation::SharedPointer<entity::EntitySystem> entity_system_;
      foundation::SharedPointer<TransformSystem> transform_system_;
      world::IWorld* world_;

      Vector<entity::Entity>         dynamic_renderables_;
      Vector<utilities::Renderable*> static_renderables_;
      utilities::ZoneManager         static_zone_manager_;

      asset::VioletTextureHandle default_albedo_;
      asset::VioletTextureHandle default_normal_;
      asset::VioletTextureHandle default_metallic_roughness_;
    };
  }
}