#include "mesh_render_system.h"
#include "transform_system.h"
#include "entity_system.h"
#include <glm/glm.hpp>
#include "assets/asset_manager.h"
#include <containers/containers.h>
#include "assets/texture.h"
#include "platform/blend_state.h"
#include "platform/rasterizer_state.h"
#include "platform/debug_renderer.h"
#include <glm/gtx/norm.hpp>
#include "assets/mesh_io.h"
#include "assets/mesh.h"
#include <algorithm>
#include <cmath>
#include "platform/culling.h"
#include "utils/renderable.h"
#include "platform/shader_variable_manager.h"

namespace lambda
{
  namespace components
  {
    MeshRenderSystem::~MeshRenderSystem()
    {
      for (auto& it : data_)
      {
        it.mesh.reset();
        it.sub_mesh = 0u;
        // TODO (Hilze): Implement.
        //it.albedo_texture.reset();
      }

      transform_system_.reset();
      entity_system_.reset();
    }
    void MeshRenderSystem::setMesh(const entity::Entity& entity, asset::MeshHandle mesh)
    {
      lookUpData(entity).mesh = mesh;
    }
    void MeshRenderSystem::setSubMesh(const entity::Entity& entity, const uint32_t& sub_mesh)
    {
      lookUpData(entity).sub_mesh = sub_mesh;
    }
    void MeshRenderSystem::setAlbedoTexture(const entity::Entity& entity, asset::VioletTextureHandle texture)
    {
      lookUpData(entity).albedo_texture = texture;
    }
    void MeshRenderSystem::setNormalTexture(const entity::Entity& entity, asset::VioletTextureHandle texture)
    {
      lookUpData(entity).normal_texture = texture;
    }
    void MeshRenderSystem::setMetallicness(const entity::Entity& entity, const float& metallicness)
    {
      lookUpData(entity).metallicness = metallicness;
    }
    void MeshRenderSystem::setRoughness(const entity::Entity& entity, const float& roughness)
    {
      lookUpData(entity).roughness = roughness;
    }
    void MeshRenderSystem::setMetallicRoughnessTexture(const entity::Entity& entity, asset::VioletTextureHandle texture)
    {
      lookUpData(entity).metallic_roughness_texture = texture;
    }
    asset::MeshHandle MeshRenderSystem::getMesh(const entity::Entity& entity)
    {
      return lookUpData(entity).mesh;
    }
    uint32_t MeshRenderSystem::getSubMesh(const entity::Entity& entity)
    {
      return lookUpData(entity).sub_mesh;
    }
    asset::VioletTextureHandle MeshRenderSystem::getAlbedoTexture(const entity::Entity& entity)
    {
      return lookUpData(entity).albedo_texture;
    }
    asset::VioletTextureHandle MeshRenderSystem::getNormalTexture(const entity::Entity& entity)
    {
      return lookUpData(entity).normal_texture;
    }
    asset::VioletTextureHandle MeshRenderSystem::getMetallicRoughnessTexture(const entity::Entity& entity)
    {
      return lookUpData(entity).metallic_roughness_texture;
    }
    float MeshRenderSystem::getMetallicness(const entity::Entity& entity)
    {
      return lookUpData(entity).metallicness;
    }
    float MeshRenderSystem::getRoughness(const entity::Entity& entity)
    {
      return lookUpData(entity).roughness;
    }
    void MeshRenderSystem::attachMesh(const entity::Entity& entity, asset::MeshHandle mesh)
    {
      // Get all textures.
      Vector<asset::VioletTextureHandle> textures = mesh->getAttachedTextures();
      const glm::uvec3 texture_count = mesh->getAttachedTextureCount();
      size_t alb_offset = 0u;
      size_t nor_offset = texture_count.x;
      size_t mrt_offset = nor_offset + texture_count.y;

      // Prepare all of the entities.
      Vector<entity::Entity> entities(mesh->getSubMeshes().size());
      Vector<TransformComponent> transforms(mesh->getSubMeshes().size());
      for (size_t i = 0; i < mesh->getSubMeshes().size(); ++i)
      {
        // Create an entity.
        entity::Entity& e = entities.at(i);
        e = entity_system_->createEntity();

        // Create a transform.
        TransformComponent& transform = transforms.at(i);
        transform = transform_system_->addComponent(e);
        // Set default parent to the mesh renderer component.
        transform.setParent(transform_system_->getComponent(entity));
      }

      // Attach all of the mesh renderers.
      for (size_t i = 0; i < mesh->getSubMeshes().size(); ++i)
      {
        const asset::SubMesh& sub_mesh = mesh->getSubMeshes().at(i);
        TransformComponent transform = transforms.at(i);
        
        transform.setLocalTranslation(sub_mesh.io.translation);
        transform.setLocalRotation(sub_mesh.io.rotation);
        transform.setLocalScale(sub_mesh.io.scale);

        if (i != sub_mesh.io.parent)
        {
          transform.setParent(transforms.at(sub_mesh.io.parent));
        }

        // Mesh
        if (sub_mesh.offset.at(asset::MeshElements::kPositions).count > 0u)
        {
          MeshRenderComponent mesh_render = addComponent(entities.at(i));
          mesh_render.setMesh(mesh);
          mesh_render.setSubMesh((uint32_t)i);
          mesh_render.setMetallicness(sub_mesh.io.metallic);
          mesh_render.setRoughness(sub_mesh.io.roughness);

          // Textures
          if (sub_mesh.io.tex_alb != -1)
          {
            assert(sub_mesh.offset.at(asset::MeshElements::kPositions).count > 0);
            mesh_render.setAlbedoTexture(textures.at(alb_offset + sub_mesh.io.tex_alb));
          }
          if (sub_mesh.io.tex_nor != -1)
          {
            assert(sub_mesh.offset.at(asset::MeshElements::kPositions).count > 0);
            mesh_render.setNormalTexture(textures.at(nor_offset + sub_mesh.io.tex_nor));
          }
          if (sub_mesh.io.tex_mrt != -1)
          {
            assert(sub_mesh.offset.at(asset::MeshElements::kPositions).count > 0);
            mesh_render.setMetallicRoughnessTexture(textures.at(mrt_offset + sub_mesh.io.tex_mrt));
          }
        }
      }
    }
    bool MeshRenderSystem::getVisible(const entity::Entity& entity) const
    {
      return lookUpData(entity).visible;
    }
    void MeshRenderSystem::setVisible(const entity::Entity& entity, const bool& visible)
    {
      lookUpData(entity).visible = visible;
    }
    bool MeshRenderSystem::getCastShadows(const entity::Entity& entity) const
    {
      return lookUpData(entity).cast_shadows;
    }
    void MeshRenderSystem::setCastShadows(const entity::Entity& entity, const bool& cast_shadows)
    {
      lookUpData(entity).cast_shadows = cast_shadows;
    }
    void MeshRenderSystem::makeStatic(const entity::Entity& entity)
    {
			for (const auto& data : static_renderables_)
				if (data->entity == entity)
					return;

      auto it = eastl::find(dynamic_renderables_.begin(), dynamic_renderables_.end(), entity);
      if (it != dynamic_renderables_.end())
        dynamic_renderables_.erase(it);

      const MeshRenderData& data = lookUpData(entity);
      if (!data.mesh)
        return;


      utilities::Renderable* renderable = foundation::Memory::construct<utilities::Renderable>();
      renderable->entity                     = entity;
      renderable->model_matrix               = transform_system_->getWorld(entity);
      renderable->mesh                       = data.mesh;
      renderable->sub_mesh                   = data.sub_mesh;
      renderable->albedo_texture             = data.albedo_texture;
      renderable->normal_texture             = data.normal_texture;
      renderable->metallic_roughness_texture = data.metallic_roughness_texture;
      renderable->metallicness               = data.metallicness;

      const asset::SubMesh& sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);

      const glm::vec3 min = renderable->model_matrix * glm::vec4(sub_mesh.min, 1.0f);
      const glm::vec3 max = renderable->model_matrix * glm::vec4(sub_mesh.max, 1.0f);
      renderable->min = glm::vec3(
        std::fminf(min.x, max.x),
        std::fminf(min.y, max.y),
        std::fminf(min.z, max.z)
      );
      renderable->max = glm::vec3(
        std::fmaxf(min.x, max.x),
        std::fmaxf(min.y, max.y),
        std::fmaxf(min.z, max.z)
      );
      renderable->center = (renderable->min + renderable->max) * 0.5f;
      renderable->radius = glm::length(renderable->center - renderable->max);

      static_zone_manager_.addToken(
        glm::vec2(renderable->min.x, renderable->min.z), 
        glm::vec2(renderable->max.x, renderable->max.z), 
        utilities::Token(entity, renderable)
      );
      static_renderables_.push_back(renderable);
    }
    void MeshRenderSystem::makeDynamic(const entity::Entity& entity)
    {
			for (const auto& data : dynamic_renderables_)
				if (data == entity)
					return;

      for (uint32_t i = 0u; i < static_renderables_.size(); ++i)
      {
        if (static_renderables_.at(i)->entity == entity)
        {
          foundation::Memory::destruct(static_renderables_.at(i));
          static_renderables_.erase(static_renderables_.begin() + i);
          break;
        }
      }

      static_zone_manager_.removeToken(utilities::Token(entity, nullptr));

      dynamic_renderables_.push_back(entity);
    }
    void MeshRenderSystem::initialize(world::IWorld& world)
    {
      transform_system_ = world.getScene().getSystem<TransformSystem>();
      entity_system_    = world.getScene().getSystem<entity::EntitySystem>();
      world_ =&world;
      
      default_albedo_ = asset::TextureManager::getInstance()->create(
        Name("__default_albedo__"),
        1u, 1u, 1u, 
        TextureFormat::kR8G8B8A8,
        0u,
        Vector<unsigned char>{ 255u, 255u, 255u, 255u }
      );
      default_normal_ = asset::TextureManager::getInstance()->create(
        Name("__default_normal__"),
        1u, 1u, 1u,
        TextureFormat::kR8G8B8A8,
        0u,
        Vector<unsigned char>{ 128u, 128u, 255u, 255u }
      );
      default_metallic_roughness_ = asset::TextureManager::getInstance()->create(
        Name("__default_metallic_roughness__"),
        1u, 1u, 1u,
        TextureFormat::kR8G8B8A8,
        0u,
        Vector<unsigned char>{ 0u, 255u, 0u, 0u }
      );
    }
    void MeshRenderSystem::deinitialize()
    {
      entity_system_.reset();
      transform_system_.reset();

      default_albedo_ = nullptr;
      default_normal_ = nullptr;
      default_metallic_roughness_ = nullptr;
    }
    void MeshRenderSystem::onRender()
    {
			//while (!dynamic_renderables_.empty())
			//	makeStatic(dynamic_renderables_.back());
    }

    void MeshRenderSystem::createRenderList(utilities::Culler& culler, const utilities::Frustum & frustum)
    {
      culler.cullStatics(static_zone_manager_, frustum);
      culler.cullDynamics(dynamic_renderables_, entity_to_data_, data_, transform_system_, frustum);
    }

    void MeshRenderSystem::createSortedRenderList(utilities::LinkedNode* statics, utilities::LinkedNode* dynamics, Vector<utilities::Renderable*>& opaque, Vector<utilities::Renderable*>& alpha)
    {
      for (utilities::LinkedNode* node = statics->next; node != nullptr; node = node->next)
      {
        utilities::Renderable* renderable = (utilities::Renderable*)node->data;
        // TODO (Hilze): Implement.
        if (renderable->albedo_texture && renderable->albedo_texture->getLayer(0u).containsAlpha())
        {
          alpha.push_back(renderable);
        }
        else
        {
          opaque.push_back(renderable);
        }
      }

      for (utilities::LinkedNode* node = dynamics->next; node != nullptr; node = node->next)
      {
        utilities::Renderable* renderable = (utilities::Renderable*)node->data;
        // TODO (Hilze): Implement.
        if (renderable->albedo_texture && renderable->albedo_texture->getLayer(0u).containsAlpha())
        {
          alpha.push_back(renderable);
        }
        else
        {
          opaque.push_back(renderable);
        }
      }
    }
    
    struct DepthSort
    {
      bool larger = false;
      Map<size_t, float> entity_to_depth;
      inline bool operator() (const MeshRenderData* mesh1, const MeshRenderData* mesh2)
      {
        if (larger)
          return entity_to_depth.at(mesh1->entity) > entity_to_depth.at(mesh2->entity);
        else
          return entity_to_depth.at(mesh1->entity) < entity_to_depth.at(mesh2->entity);
      }
    };

    void MeshRenderSystem::renderAll(utilities::Culler& culler, const utilities::Frustum& frustum)
    {
      createRenderList(culler, frustum);

      utilities::LinkedNode statics = culler.getStatics();
      utilities::LinkedNode dynamics = culler.getDynamics();

      renderAll(&statics, &dynamics);
    }
    void MeshRenderSystem::renderAll(utilities::LinkedNode* statics, utilities::LinkedNode* dynamics)
    {
      Vector<utilities::Renderable*> opaque;
      Vector<utilities::Renderable*> alpha;
      createSortedRenderList(statics, dynamics, opaque, alpha);
      renderAll(opaque, alpha);
    }
    void MeshRenderSystem::renderAll(const Vector<utilities::Renderable*>& opaque, const Vector<utilities::Renderable*>& alpha)
    {
      foundation::SharedPointer<platform::IRenderer> renderer = world_->getRenderer();

      // -------------------------RENDER-STATIC-OPAQUE-------------------------
      renderer->setBlendState(platform::BlendState::Default());
      for (const utilities::Renderable* renderable : opaque)
      {
        renderer->setMesh(renderable->mesh);
        renderer->setSubMesh(renderable->sub_mesh);

        renderer->setTexture((renderable->albedo_texture) ? renderable->albedo_texture : default_albedo_, 0u);
        renderer->setTexture((renderable->normal_texture) ? renderable->normal_texture : default_normal_, 1u);
        renderer->setTexture((renderable->metallic_roughness_texture) ? renderable->metallic_roughness_texture : default_metallic_roughness_, 2u);

        world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("metallic_roughness"), glm::vec2(renderable->metallicness, renderable->roughness)));
        world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("model_matrix"), renderable->model_matrix));

        auto sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);
        // TODO (Hilze): Implement.
        if (sub_mesh.io.double_sided == true || (sub_mesh.io.tex_alb >= 0 && renderable->mesh->getAttachedTextures().at(sub_mesh.io.tex_alb)->getLayer(0u).containsAlpha()))
        {
          renderer->setRasterizerState(platform::RasterizerState::SolidNone());
        }
        else
        {
          renderer->setRasterizerState(platform::RasterizerState::SolidFront());
        }

        renderer->draw();
      }
      // -------------------------RENDER-STATIC-ALPHA-------------------------
      renderer->setBlendState(platform::BlendState::Alpha());
      for (const utilities::Renderable* renderable : alpha)
      {
        renderer->setMesh(renderable->mesh);
        world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("metallic_roughness"), glm::vec2(renderable->metallicness, renderable->roughness)));
        renderer->setSubMesh(renderable->sub_mesh);
        renderer->setTexture((renderable->albedo_texture) ? renderable->albedo_texture : default_albedo_, 0u);
        renderer->setTexture((renderable->normal_texture) ? renderable->normal_texture : default_normal_, 1u);
        renderer->setTexture((renderable->metallic_roughness_texture) ? renderable->metallic_roughness_texture : default_metallic_roughness_, 2u);
        world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("model_matrix"), renderable->model_matrix));

        auto sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);
        // TODO (Hilze): Implement.
        if (sub_mesh.io.double_sided == true || (sub_mesh.io.tex_alb >= 0 && renderable->mesh->getAttachedTextures().at(sub_mesh.io.tex_alb)->getLayer(0u).containsAlpha()))
          renderer->setRasterizerState(platform::RasterizerState::SolidNone());
        else
          renderer->setRasterizerState(platform::RasterizerState::SolidFront());

        renderer->draw();
      }
    }
    MeshRenderComponent MeshRenderSystem::addComponent(const entity::Entity& entity)
    {
      if (transform_system_->hasComponent(entity) == false)
      {
        transform_system_->addComponent(entity);
      }

      data_.push_back(MeshRenderData(entity));
      data_to_entity_[(uint32_t)data_.size() - 1u] = entity;
      entity_to_data_[entity] = (uint32_t)data_.size() - 1u;
      
      dynamic_renderables_.push_back(entity);

      return MeshRenderComponent(entity, this);
    }
    MeshRenderComponent MeshRenderSystem::getComponent(const entity::Entity& entity)
    {
      return MeshRenderComponent(entity, this);
    }
    bool MeshRenderSystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity) != entity_to_data_.end();
    }
    void MeshRenderSystem::removeComponent(const entity::Entity& entity)
    {
      const auto& it = entity_to_data_.find(entity);
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
		String MeshRenderSystem::profilerInfo() const
		{
			return "Dynamics: " + toString(dynamic_renderables_.size()) + "\nStatics: " + toString(static_renderables_.size());
		}
    MeshRenderData& MeshRenderSystem::lookUpData(const entity::Entity& entity)
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
      return data_.at(entity_to_data_.at(entity));
    }
    const MeshRenderData& MeshRenderSystem::lookUpData(const entity::Entity& entity) const
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
      return data_.at(entity_to_data_.at(entity));
    }
    MeshRenderComponent::MeshRenderComponent(const entity::Entity& entity, MeshRenderSystem* system) :
      IComponent(entity), system_(system)
    {
    }
    MeshRenderComponent::MeshRenderComponent(const MeshRenderComponent& other) :
      IComponent(other.entity_), system_(other.system_)
    {
    }
    MeshRenderComponent::MeshRenderComponent() :
      IComponent(entity::Entity()), system_(nullptr)
    {
    }
    void MeshRenderComponent::setMesh(asset::MeshHandle mesh)
    {
      system_->setMesh(entity_, mesh);
    }
    asset::MeshHandle MeshRenderComponent::getMesh() const
    {
      return system_->getMesh(entity_);
    }
    void MeshRenderComponent::setSubMesh(const uint32_t& sub_mesh)
    {
      system_->setSubMesh(entity_, sub_mesh);
    }
    uint32_t MeshRenderComponent::getSubMesh() const
    {
      return system_->getSubMesh(entity_);
    }
    void MeshRenderComponent::setMetallicness(const float& metallicness)
    {
      system_->setMetallicness(entity_, metallicness);
    }
    float MeshRenderComponent::getMetallicness() const
    {
      return system_->getMetallicness(entity_);
    }
    void MeshRenderComponent::setRoughness(const float& roughness)
    {
      system_->setRoughness(entity_, roughness);
    }
    float MeshRenderComponent::getRoughness() const
    {
      return system_->getRoughness(entity_);
    }
    asset::VioletTextureHandle MeshRenderComponent::getAlbedoTexture() const
    {
      return system_->getAlbedoTexture(entity_);
    }
    void MeshRenderComponent::setNormalTexture(asset::VioletTextureHandle texture)
    {
      system_->setNormalTexture(entity_, texture);
    }
    asset::VioletTextureHandle MeshRenderComponent::getNormalTexture() const
    {
      return system_->getNormalTexture(entity_);
    }
    void MeshRenderComponent::setMetallicRoughnessTexture(asset::VioletTextureHandle texture)
    {
      system_->setMetallicRoughnessTexture(entity_, texture);
    }
    asset::VioletTextureHandle MeshRenderComponent::getMetallicRoughnessTexture() const
    {
      return system_->getMetallicRoughnessTexture(entity_);
    }
    void MeshRenderComponent::setAlbedoTexture(asset::VioletTextureHandle texture)
    {
      system_->setAlbedoTexture(entity_, texture);
    }
    void MeshRenderComponent::attachMesh(asset::MeshHandle mesh)
    {
      system_->attachMesh(entity_, mesh);
    }
    bool MeshRenderComponent::getVisible() const
    {
      return system_->getVisible(entity_);
    }
    void MeshRenderComponent::setVisible(const bool& visible)
    {
      system_->setVisible(entity_, visible);
    }
    bool MeshRenderComponent::getCastShadows() const
    {
      return system_->getCastShadows(entity_);
    }
    void MeshRenderComponent::setCastShadows(const bool& cast_shadows)
    {
      system_->setCastShadows(entity_, cast_shadows);
    }
    MeshRenderData::MeshRenderData(const MeshRenderData & other)
    {
      mesh                       = other.mesh;
      sub_mesh                   = other.sub_mesh;
      albedo_texture             = other.albedo_texture;
      normal_texture             = other.normal_texture;
      metallic_roughness_texture = other.metallic_roughness_texture;
      metallicness               = other.metallicness;
      roughness                  = other.roughness;
      visible                    = other.visible;
      cast_shadows               = other.cast_shadows;
      entity                     = other.entity;
    }
    MeshRenderData & MeshRenderData::operator=(const MeshRenderData & other)
    {
      mesh                       = other.mesh;
      sub_mesh                   = other.sub_mesh;
      albedo_texture             = other.albedo_texture;
      normal_texture             = other.normal_texture;
      metallic_roughness_texture = other.metallic_roughness_texture;
      metallicness               = other.metallicness;
      roughness                  = other.roughness;
      visible                    = other.visible;
      cast_shadows               = other.cast_shadows;
      entity                     = other.entity;

      return *this;
    }
  }
}
