#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "light_system.h"
#include "transform_system.h"
#include "mesh_render_system.h"
#include "camera_system.h"
#include "interfaces/iworld.h"
#include "assets/shader_io.h"
#include "platform/post_process_manager.h"
#include "platform/rasterizer_state.h"
#include "platform/blend_state.h"
#include "platform/sampler_state.h"
#include <glm/gtc/matrix_transform.hpp>
#include "platform/shader_variable.h"
#include "platform/debug_renderer.h"
#include <algorithm>
#include "interfaces/irenderer.h"

namespace lambda
{
  namespace components
  {
    LightSystem::~LightSystem()
    {
    }
    BaseLightComponent LightSystem::addComponent(const entity::Entity& entity)
    {
      if (false == transform_system_->hasComponent(entity))
      {
        transform_system_->addComponent(entity);
      }

      data_.push_back(LightData(entity));
      data_to_entity_[(uint32_t)data_.size() - 1u] = entity;
      entity_to_data_[entity] = (uint32_t)data_.size() - 1u;
      
      auto& data = lookUpData(entity);
      data.type  = LightType::kUnknown;
      data.culler.push_back(utilities::Culler());
      data.culler.back().setCullFrequency(3u);
      data.culler.back().setShouldCull(true);
      data.depth.push_back(100.0f);
      data.view.resize(1u);
      data.projection.resize(1u);
      data.view_position.resize(1u);
      //data.culler.setCullShadowCasters(true);
      data.texture = default_texture_;

      return BaseLightComponent(entity, this);
    }
    BaseLightComponent LightSystem::getComponent(const entity::Entity& entity)
    {
      return BaseLightComponent(entity, this);
    }
    bool LightSystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity) != entity_to_data_.end();
    }
    void LightSystem::removeComponent(const entity::Entity& entity)
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
    DirectionalLightComponent LightSystem::addDirectionalLight(const entity::Entity& entity)
    {
      addComponent(entity);

      lookUpData(entity).type = LightType::kDirectional;
      return DirectionalLightComponent(entity, this);
    }
    PointLightComponent LightSystem::addPointLight(const entity::Entity& entity)
    {
      addComponent(entity);

      lookUpData(entity).type = LightType::kPoint;
      return PointLightComponent(entity, this);
    }
    SpotLightComponent LightSystem::addSpotLight(const entity::Entity& entity)
    {
      addComponent(entity);

      lookUpData(entity).type = LightType::kSpot;
      return SpotLightComponent(entity, this);
    }
    DirectionalLightComponent LightSystem::addCascadedLight(const entity::Entity& entity)
    {
      addComponent(entity);

      lookUpData(entity).type = LightType::kCascade;
      return DirectionalLightComponent(entity, this);
    }
    DirectionalLightComponent LightSystem::getDirectionalLight(const entity::Entity& entity)
    {
      return DirectionalLightComponent(entity, this);
    }
    PointLightComponent LightSystem::getPointLight(const entity::Entity& entity)
    {
      return PointLightComponent(entity, this);
    }
    SpotLightComponent LightSystem::getSpotLight(const entity::Entity& entity)
    {
      return SpotLightComponent(entity, this);
    }
    void LightSystem::setShadersDirectional(asset::ShaderHandle generate, Vector<asset::ShaderHandle> modify, asset::ShaderHandle publish)
    {
      shaders_directional_.shader_generate = generate;
      shaders_directional_.shader_modify   = modify;
      shaders_directional_.shader_publish  = publish;
    }
    void LightSystem::setShadersSpot(asset::ShaderHandle generate, Vector<asset::ShaderHandle> modify, asset::ShaderHandle publish)
    {
      shaders_spot_.shader_generate = generate;
      shaders_spot_.shader_modify   = modify;
      shaders_spot_.shader_publish  = publish;
    }
    void LightSystem::setShadersPoint(asset::ShaderHandle generate, Vector<asset::ShaderHandle> modify, asset::ShaderHandle publish)
    {
      shaders_point_.shader_generate = generate;
      shaders_point_.shader_modify   = modify;
      shaders_point_.shader_publish  = publish;
    }
    void LightSystem::setShadersCascade(asset::ShaderHandle generate, Vector<asset::ShaderHandle> modify, asset::ShaderHandle publish)
    {
      shaders_cascade_.shader_generate = generate;
      shaders_cascade_.shader_modify   = modify;
      shaders_cascade_.shader_publish  = publish;
    }
    void LightSystem::setShadersDirectionalRSM(asset::ShaderHandle generate, Vector<asset::ShaderHandle> modify, asset::ShaderHandle publish)
    {
      shaders_directional_rsm_.shader_generate = generate;
      shaders_directional_rsm_.shader_modify   = modify;
      shaders_directional_rsm_.shader_publish  = publish;
    }
    void LightSystem::setShadersSpotRSM(asset::ShaderHandle generate, Vector<asset::ShaderHandle> modify, asset::ShaderHandle publish)
    {
      shaders_spot_rsm_.shader_generate = generate;
      shaders_spot_rsm_.shader_modify   = modify;
      shaders_spot_rsm_.shader_publish  = publish;
    }
    void LightSystem::initialize(world::IWorld& world)
    {
      transform_system_ = world.getScene().getSystem<TransformSystem>();
      mesh_render_system_ = world.getScene().getSystem<MeshRenderSystem>();
      camera_system_ = world.getScene().getSystem<CameraSystem>();
      world_ = &world;
	  
      world_->getPostProcessManager().addTarget(platform::RenderTarget(Name("light_map"),
        1.0f, TextureFormat::kR16G16B16A16
      ));

      full_screen_mesh_ = asset::AssetManager::getInstance().createAsset(
        Name("__fs_quad__"), foundation::Memory::constructShared<lambda::asset::Mesh>(asset::Mesh::createScreenQuad())
      );

      default_shadow_map_ = platform::RenderTarget(Name("__default_shadow_map__"),
        asset::TextureManager::getInstance()->create(
          Name("__default_shadow_map__"),
          1u, 1u, 1u, TextureFormat::kR16G16B16A16
          , kTextureFlagIsRenderTarget // TODO (Hilze): Remove!
        )
      );
      
      default_texture_ = asset::TextureManager::getInstance()->create(
        Name("__default_light_texture__"),
        1u, 1u, 1u, TextureFormat::kR8G8B8A8,
        kTextureFlagIsRenderTarget, // TODO (Hilze): Remove!
        Vector<unsigned char>(4u, 255u)
      );

      world_->getRenderer()->clearRenderTarget(default_shadow_map_.getTexture(), glm::vec4(1.0f));
    }
    void LightSystem::deinitialize()
    {
      camera_system_.reset();
      mesh_render_system_.reset();
      transform_system_.reset();
    }

    void LightSystem::onRender()
    {
      world_->getRenderer()->beginTimer("Lighting");
      foundation::SharedPointer<platform::IRenderer> renderer = world_->getRenderer();
      
      // Prepare the light buffer.
      world_->getRenderer()->pushMarker("Clear Light Buffer");
      renderer->clearRenderTarget(
        world_->getPostProcessManager().getTarget(Name("light_map")).getTexture(),
        glm::vec4(0.0f)
      );
      world_->getRenderer()->popMarker();

      for (LightData& data : data_)
      {
        // Skip disabled lights.
        if (data.enabled == false)
        {
          switch (data.type)
          {
          case LightType::kDirectional: world_->getRenderer()->setMarker("Directional Light - Disabled"); break;
          case LightType::kSpot:        world_->getRenderer()->setMarker("Spot Light - Disabled"); break;
          case LightType::kPoint:       world_->getRenderer()->setMarker("Point Light - Disabled"); break;
          case LightType::kCascade:     world_->getRenderer()->setMarker("Cascade Light - Disabled"); break;
          default:
            break;
          }
          continue;
        }

        switch (data.type)
        {
        case LightType::kDirectional:
          if (shaders_directional_.shader_generate != nullptr && shaders_directional_.shader_publish != nullptr)
          {
            world_->getRenderer()->pushMarker("Directional Light");
            renderDirectional(data.entity);
            world_->getRenderer()->popMarker();
          }
          break;
        case LightType::kSpot:
          if (shaders_spot_.shader_generate != nullptr && shaders_spot_.shader_publish != nullptr)
          {
            world_->getRenderer()->pushMarker("Spot Light");
            renderSpot(data.entity);
            world_->getRenderer()->popMarker();
          }
          break;
        case LightType::kPoint:
          if (shaders_point_.shader_generate != nullptr && shaders_point_.shader_publish != nullptr)
          {
            world_->getRenderer()->pushMarker("Point Light");
            renderPoint(data.entity);
            world_->getRenderer()->popMarker();
          }
          break;
        case LightType::kCascade:
          if (shaders_cascade_.shader_generate != nullptr && shaders_cascade_.shader_publish != nullptr)
          {
            world_->getRenderer()->pushMarker("Cascade Light");
            renderCascade(data.entity);
            world_->getRenderer()->popMarker();
          }
          break;
        default:
          break;
        }
      }

      // Reset states.
      renderer->setRasterizerState(platform::RasterizerState::SolidFront());
      camera_system_->bindCamera(camera_system_->getMainCamera());

      world_->getRenderer()->endTimer("Lighting");
    }
    void LightSystem::setColour(const entity::Entity& entity, const glm::vec3& colour)
    {
      lookUpData(entity).colour = colour;
    }
    glm::vec3 LightSystem::getColour(const entity::Entity& entity) const
    {
      return lookUpData(entity).colour;
    }
    void LightSystem::setAmbient(const entity::Entity& entity, const glm::vec3& ambient)
    {
      lookUpData(entity).ambient = ambient;
    }
    glm::vec3 LightSystem::getAmbient(const entity::Entity& entity) const
    {
      return lookUpData(entity).ambient;
    }
    void LightSystem::setIntensity(const entity::Entity& entity, const float& intensity)
    {
      lookUpData(entity).intensity = intensity;
    }
    float LightSystem::getIntensity(const entity::Entity& entity) const
    {
      return lookUpData(entity).intensity;
    }
    void LightSystem::setShadowType(const entity::Entity& entity, const ShadowType& shadow_type)
    {
      lookUpData(entity).shadow_type = shadow_type;
    }
    ShadowType LightSystem::getShadowType(const entity::Entity& entity) const
    {
      return lookUpData(entity).shadow_type;
    }
    void LightSystem::setRenderTarget(const entity::Entity& entity, Vector<platform::RenderTarget> render_target)
    {
      auto& data = lookUpData(entity);
      data.render_target = render_target;
      static unsigned int idx = 0u;

      data.depth_target.resize(data.render_target.size());
      data.culler.resize(data.render_target.size());
      data.view_position.resize(data.render_target.size(), glm::vec3(0.0f));
      data.projection.resize(data.render_target.size(), glm::mat4x4(1.0f));
      data.view.resize(data.render_target.size(), glm::mat4x4(1.0f));
      data.depth.resize(data.render_target.size(), data.depth.back());

      for (int i = 0; i < (data.rsm ? 1u : data.render_target.size()); ++i)
      {
        Name name = Name("shadow_map_depth_" + toString(i) + "_" + toString(idx));
        data.depth_target.at(i) = platform::RenderTarget(name,
          asset::TextureManager::getInstance()->create(
            name,
            data.render_target.at(i).getTexture()->getLayer(0u).getWidth(),
            data.render_target.at(i).getTexture()->getLayer(0u).getHeight(),
            1u,
            TextureFormat::kR24G8,
            kTextureFlagIsRenderTarget // TODO (Hilze): Remove!
          )
        );
      }
      idx++;
    }
    Vector<platform::RenderTarget> LightSystem::getRenderTarget(const entity::Entity& entity) const
    {
      return lookUpData(entity).render_target;
    }
    void LightSystem::setCutOff(const entity::Entity& entity, const utilities::Angle& cut_off)
    {
      lookUpData(entity).cut_off = cut_off;
    }
    utilities::Angle LightSystem::getCutOff(const entity::Entity& entity) const
    {
      return lookUpData(entity).cut_off;
    }
    void LightSystem::setOuterCutOff(const entity::Entity& entity, const utilities::Angle& outer_cut_off)
    {
      lookUpData(entity).outer_cut_off = outer_cut_off;
    }
    utilities::Angle LightSystem::getOuterCutOff(const entity::Entity& entity) const
    {
      return lookUpData(entity).outer_cut_off;
    }
    void LightSystem::setDepth(const entity::Entity& entity, const float& depth)
    {
      lookUpData(entity).depth.back() = depth;
    }
    float LightSystem::getDepth(const entity::Entity& entity) const
    {
      return lookUpData(entity).depth.back();
    }
    void LightSystem::setSize(const entity::Entity& entity, const float& size)
    {
      lookUpData(entity).size = size;
    }
    float LightSystem::getSize(const entity::Entity& entity) const
    {
      return lookUpData(entity).size;
    }
    void LightSystem::setTexture(const entity::Entity& entity, asset::VioletTextureHandle texture)
    {
      lookUpData(entity).texture = texture;
    }
    asset::VioletTextureHandle LightSystem::getTexture(const entity::Entity& entity) const
    {
      return lookUpData(entity).texture;
    }
    void LightSystem::setEnabled(const entity::Entity& entity, const bool& enabled)
    {
      lookUpData(entity).enabled = enabled;
    }
    bool LightSystem::getEnabled(const entity::Entity& entity) const
    {
      return lookUpData(entity).enabled;
    }
    void LightSystem::setDynamicFrequency(const entity::Entity& entity, const uint8_t& frequency)
    {
      lookUpData(entity).dynamic_frequency = frequency;
    }
    uint8_t LightSystem::getDynamicFrequency(const entity::Entity& entity) const
    {
      return lookUpData(entity).dynamic_frequency;
    }
    void LightSystem::setRSM(const entity::Entity& entity, bool rsm)
    {
      lookUpData(entity).rsm = rsm;
    }
    bool LightSystem::getRSM(const entity::Entity& entity) const
    {
      return lookUpData(entity).rsm;
    }
    void LightSystem::setLightType(const entity::Entity& entity, LightType type)
    {
      lookUpData(entity).type = type;
    }
    LightType LightSystem::getLightType(const entity::Entity& entity) const
    {
      return lookUpData(entity).type;
    }
    LightData& LightSystem::lookUpData(const entity::Entity& entity)
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
      return data_.at(entity_to_data_.at(entity));
    }
    const LightData& LightSystem::lookUpData(const entity::Entity& entity) const
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
      return data_.at(entity_to_data_.at(entity));
    }
    void LightSystem::renderDirectional(const entity::Entity& entity)
    {
      foundation::SharedPointer<platform::IRenderer> renderer = world_->getRenderer();
      LightData& data = lookUpData(entity);
      const ShaderPass& shader_pack = data.rsm ? shaders_directional_rsm_ : shaders_directional_;

      const TransformComponent transform = transform_system_->getComponent(entity);
      const glm::vec3 forward = glm::normalize(transform.getWorldForward());

      const bool update = (data.shadow_type == ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != ShadowType::kGenerated);

      if (data.dynamic_index >= data.dynamic_frequency)
      {
        data.dynamic_index = 0u;
      }

      Vector<platform::RenderTarget> shadow_maps(data.render_target.begin(), data.render_target.end());
      if (shadow_maps.empty()) shadow_maps.push_back(default_shadow_map_);

      // Setup camera.
      if (update)
      {
        // Set everything up.
        float light_depth = data.depth.back();
        float smh_size = data.size * 0.5f;
        glm::vec3 translation = transform.getWorldTranslation();

        // Remove shimmering
        {
          float texels_per_unit = (float)shadow_maps.begin()->getTexture()->getLayer(0u).getWidth() / data.size;
          glm::vec3 scalar(texels_per_unit);

          glm::mat4x4 look_at = glm::lookAtRH(glm::vec3(0.0f), -forward, glm::vec3(0.0f, 1.0f, 0.0f));
          look_at *= glm::vec4(scalar, 1.0f);
          glm::mat4x4 look_at_inv = glm::inverse(look_at);

          translation   = glm::vec3(glm::vec4(translation, 1.0f) * look_at);
          translation.x = std::floorf(translation.x);
          translation.y = std::floorf(translation.y);
          translation   = glm::vec3(glm::vec4(translation, 1.0f) * look_at_inv);
        }

        // Update matrices.
        data.view.back()          = glm::lookAtRH(translation, translation + forward, glm::vec3(0.0f, 1.0f, 0.0f));
        data.view_position.back() = translation;
        data.projection.back() = glm::orthoRH(-smh_size, smh_size, -smh_size, smh_size, -light_depth * 0.5f, light_depth * 0.5f);
      }

      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_view_projection_matrix"), data.projection.back() * data.view.back()));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_camera_position"), data.view_position.back()));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_direction"), -forward));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_colour"), data.colour * data.intensity));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_ambient"), data.ambient));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_far"), data.depth.back()));


      // Generate shadow maps.
      if (update && data.shadow_type != ShadowType::kNone)
      {
        if (data.shadow_type == ShadowType::kGenerateOnce)
        {
          data.shadow_type = ShadowType::kGenerated;
        }

        // Clear the shadow map.
        for (const auto& shadow_map : shadow_maps)
        {
          renderer->clearRenderTarget(shadow_map.getTexture(), glm::vec4(0.0f));
        }
        const platform::RenderTarget depth_map = data.depth_target.at(0u);
        renderer->clearRenderTarget(depth_map.getTexture(), glm::vec4(1.0f));

        Vector<platform::RenderTarget> output = shadow_maps;
        output.push_back(depth_map);

        // Render to the shadow map.
        renderer->bindShaderPass(
          platform::ShaderPass(
            Name("shadow_generate"),
            shader_pack.shader_generate,
            {},
            output
          )
        );

        // Handle frustum culling.
        if (data.shadow_type == ShadowType::kDynamic)
        {
          data.culler.back().setCullFrequency(data.dynamic_frequency);
        }
        else
        {
          data.culler.back().setCullFrequency(1u);
        }
        
        utilities::Frustum frustum;
        frustum.construct(data.projection.back(), data.view.back());

        // Render to the shadow map.
        mesh_render_system_->renderAll(data.culler.back(), frustum);

        // Set up the post processing passes.
        renderer->setMesh(full_screen_mesh_);
        renderer->setSubMesh(0u);
        renderer->setRasterizerState(platform::RasterizerState::SolidBack());
        renderer->setBlendState(platform::BlendState::Alpha());

        // Draw all modify shaders.
        for (const asset::ShaderHandle& it : shader_pack.shader_modify)
        {
          renderer->bindShaderPass(
            platform::ShaderPass(
              Name("shadow_modify"),
              it,
              { shadow_maps.at(0u) },
              { shadow_maps.at(0u) }
            )
          );
          renderer->draw();
        }
      }

      // Render lights to the light map.
      // Set up the post processing passes.
      renderer->setMesh(full_screen_mesh_);
      renderer->setSubMesh(0u);
      renderer->setRasterizerState(platform::RasterizerState::SolidBack());

      // Render light using the shadow map.
      Vector<platform::RenderTarget> input = {
        shadow_maps.at(0u),
        platform::RenderTarget(Name("texture"), data.texture),
        world_->getPostProcessManager().getTarget(Name("position")),
        world_->getPostProcessManager().getTarget(Name("normal")),
        world_->getPostProcessManager().getTarget(Name("metallic_roughness"))
      };
      if (shadow_maps.size() > 1u) input.insert(input.end(), shadow_maps.begin() + 1u, shadow_maps.end());
      renderer->bindShaderPass(
        platform::ShaderPass(
          Name("shadow_publish"),
          shader_pack.shader_publish,
          input, {
            world_->getPostProcessManager().getTarget(Name("light_map"))
          }
        )
      );
      
      renderer->setBlendState(platform::BlendState(
        false,                                /*alpha_to_coverage*/
        true,                                 /*blend_enabled*/
        platform::BlendState::BlendMode::kOne /*src_blend*/,
        platform::BlendState::BlendMode::kOne /*dest_blend*/,
        platform::BlendState::BlendOp::kAdd   /*blend_op*/,
        platform::BlendState::BlendMode::kOne /*src_blend_alpha*/,
        platform::BlendState::BlendMode::kOne /*dest_blend_alpha*/,
        platform::BlendState::BlendOp::kAdd   /*blend_op_alpha*/,
        (unsigned char)platform::BlendState::WriteMode::kColourWriteEnableRGB /*write_mask*/
      ));
      renderer->draw();
      renderer->setBlendState(platform::BlendState::Alpha());
    }
    void LightSystem::renderSpot(const entity::Entity& entity)
    {
      foundation::SharedPointer<platform::IRenderer> renderer = world_->getRenderer();
      LightData& data = lookUpData(entity);
      const ShaderPass& shader_pack = data.rsm ? shaders_spot_rsm_ : shaders_spot_;

      const TransformComponent transform = transform_system_->getComponent(entity);
      const glm::vec3 forward = glm::normalize(transform.getWorldForward());

      Vector<platform::RenderTarget> shadow_maps(data.render_target.begin(), data.render_target.end());
      if (shadow_maps.empty()) shadow_maps.push_back(default_shadow_map_);
      const bool update = (data.shadow_type == ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != ShadowType::kGenerated);

      if (data.dynamic_index >= data.dynamic_frequency)
      {
        data.dynamic_index = 0u;
      }


      // Setup camera.
      if (update)
      {
        // Set everything up.
        glm::vec3 translation = transform.getWorldTranslation();

#ifdef SHIMMERING_REMOVED
        // Remove shimmering
        float texels_per_unit = shadow_map.getTexture()->getSize().x / data.depth.back();
        glm::vec3 scalar(texels_per_unit);

        glm::vec3 base_look_at = -forward;

        glm::mat4x4 look_at = glm::lookAtRH(glm::vec3(0.0f), base_look_at, glm::vec3(0.0f, 1.0f, 0.0f));
        look_at *= glm::vec4(scalar, 1.0f);
        glm::mat4x4 look_at_inv = glm::inverse(look_at);

        translation = glm::vec3(glm::vec4(translation, 1.0f) * look_at);
        translation.x = std::floorf(translation.x);
        translation.y = std::floorf(translation.y);
        translation = glm::vec3(glm::vec4(translation, 1.0f) * look_at_inv);
#endif

        data.view.back()          = glm::lookAtRH(translation, translation - forward, glm::vec3(0.0f, 1.0f, 0.0f));
        data.view_position.back() = translation;
        data.projection.back()    = glm::perspectiveRH(data.outer_cut_off.asRad(), 1.0f, 0.001f, data.depth.back());
      }

      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_view_projection_matrix"), data.projection.back() * data.view.back()));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_camera_position"), data.view_position.back()));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_direction"), -forward));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_colour"), data.colour * data.intensity));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_ambient"), data.ambient));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_far"), data.depth.back()));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_position"), transform.getWorldTranslation()));

      // Required by: spot.
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_cut_off"), data.cut_off.asRad()));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_outer_cut_off"), data.outer_cut_off.asRad()));

      // Generate shadow maps.
      if (update && data.shadow_type != ShadowType::kNone)
      {
        if (data.shadow_type == ShadowType::kGenerateOnce)
        {
          data.shadow_type = ShadowType::kGenerated;
        }

        // Clear the shadow map.
        for (const auto& shadow_map : shadow_maps)
        {
          renderer->clearRenderTarget(shadow_map.getTexture(), glm::vec4(0.0f));
        }
        const platform::RenderTarget depth_map = data.depth_target.at(0u);
        renderer->clearRenderTarget(depth_map.getTexture(), glm::vec4(1.0f));

        Vector<platform::RenderTarget> output = shadow_maps;
        output.push_back(depth_map);

        // Render to the shadow map.
        renderer->bindShaderPass(
          platform::ShaderPass(
            Name("shadow_generate"),
            shader_pack.shader_generate,
            {},
            output
          )
        );

          
        // Handle frustum culling.
        if (data.shadow_type == ShadowType::kDynamic)
        {
          data.culler.back().setCullFrequency(std::max(1u, 10u / data.dynamic_frequency));
        }
        else
        {
          data.culler.back().setCullFrequency(1u);
        }

        utilities::Frustum frustum;
        frustum.construct(data.projection.back(), data.view.back());

        // Render to the shadow map.
        mesh_render_system_->renderAll(data.culler.back(), frustum);

        // Set up the post processing passes.
        renderer->setMesh(full_screen_mesh_);
        renderer->setSubMesh(0u);
        renderer->setRasterizerState(platform::RasterizerState::SolidBack());
        renderer->setBlendState(platform::BlendState::Alpha());

        for (const asset::ShaderHandle& it : shader_pack.shader_modify)
        {
          renderer->bindShaderPass(
            platform::ShaderPass(
              Name("shadow_modify"),
              it,
              { shadow_maps.at(0u) },
              { shadow_maps.at(0u) }
            )
          );
          renderer->draw();
        }
      }

      // Render lights to the light map.
      // Set up the post processing passes.
      renderer->setMesh(full_screen_mesh_);
      renderer->setSubMesh(0u);
      renderer->setRasterizerState(platform::RasterizerState::SolidBack());

      // Render light using the shadow map.
      Vector<platform::RenderTarget> input = {
        shadow_maps.at(0u),
        platform::RenderTarget(Name("texture"), data.texture),
        world_->getPostProcessManager().getTarget(Name("position")),
        world_->getPostProcessManager().getTarget(Name("normal")),
        world_->getPostProcessManager().getTarget(Name("metallic_roughness"))
      };
      if (shadow_maps.size() > 1u) input.insert(input.end(), shadow_maps.begin() + 1u, shadow_maps.end());
      renderer->bindShaderPass(
        platform::ShaderPass(
          Name("shadow_publish"),
          shader_pack.shader_publish,
          input, {
            world_->getPostProcessManager().getTarget(Name("light_map"))
          }
          )
      );
                
      renderer->setBlendState(platform::BlendState(
        false,                                /*alpha_to_coverage*/
        true,                                 /*blend_enabled*/
        platform::BlendState::BlendMode::kOne /*src_blend*/,
        platform::BlendState::BlendMode::kOne /*dest_blend*/,
        platform::BlendState::BlendOp::kAdd   /*blend_op*/,
        platform::BlendState::BlendMode::kOne /*src_blend_alpha*/,
        platform::BlendState::BlendMode::kOne /*dest_blend_alpha*/,
        platform::BlendState::BlendOp::kAdd   /*blend_op_alpha*/,
        (unsigned char)platform::BlendState::WriteMode::kColourWriteEnableRGB /*write_mask*/
      ));
      renderer->draw();
      renderer->setBlendState(platform::BlendState::Alpha());
    }
    void LightSystem::renderPoint(const entity::Entity& entity)
    {
      foundation::SharedPointer<platform::IRenderer> renderer = world_->getRenderer();
      LightData& data = lookUpData(entity);
      
      const TransformComponent transform = transform_system_->getComponent(entity);
      const glm::vec3 forward = glm::normalize(transform.getWorldForward());

      const platform::RenderTarget shadow_map = (data.render_target.empty()) ? default_shadow_map_ : data.render_target.at(0u);
      const bool update = (data.shadow_type == ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != ShadowType::kGenerated);

      if (data.dynamic_index >= data.dynamic_frequency)
      {
        data.dynamic_index = 0u;
      }


      // Setup camera.
      if (update)
      {
        // Set everything up.
        glm::vec3 translation = transform.getWorldTranslation();
        translation += -forward * (data.depth.back() * 0.5f);


        // Remove shimmering
        float texels_per_unit = (float)shadow_map.getTexture()->getLayer(0u).getWidth() / data.size;
        glm::vec3 scalar(texels_per_unit);

        glm::vec3 base_look_at = -forward;

        glm::mat4x4 look_at = glm::lookAtRH(glm::vec3(0.0f), base_look_at, glm::vec3(0.0f, 1.0f, 0.0f));
        look_at *= glm::vec4(scalar, 1.0f);
        glm::mat4x4 look_at_inv = glm::inverse(look_at);

        translation = glm::vec3(glm::vec4(translation, 1.0f) * look_at);
        translation.x = std::floorf(translation.x);
        translation.y = std::floorf(translation.y);
        translation = glm::vec3(glm::vec4(translation, 1.0f) * look_at_inv);

        data.view.back()          = glm::lookAtRH(translation, translation + forward, glm::vec3(0.0f, 1.0f, 0.0f));
        data.view_position.back() = translation;
        data.projection.back()    = glm::orthoRH(-data.size * 0.5f, data.size * 0.5f, -data.size * 0.5f, data.size * 0.5f, -data.depth.back() * 0.5f, data.depth.back() * 0.5f);
      }

      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_view_projection_matrix"), data.projection.back() * data.view.back()));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_camera_position"), data.view_position.back()));

      // Generate shadow maps.
      if (update && data.shadow_type != ShadowType::kNone)
      {
        if (data.shadow_type == ShadowType::kGenerateOnce)
        {
          data.shadow_type = ShadowType::kGenerated;
        }

        // Clear the shadow map.
        const platform::RenderTarget depth_map = data.depth_target.at(0u);

        renderer->clearRenderTarget(shadow_map.getTexture(), glm::vec4(0.0f));
        renderer->clearRenderTarget(depth_map.getTexture(), glm::vec4(1.0f));

        // Render to the shadow map.
        renderer->bindShaderPass(
          platform::ShaderPass(
            Name("shadow_generate"),
            shaders_point_.shader_generate,
            {},
            { shadow_map, depth_map }
          )
        );

          
        // Handle frustum culling.
        if (data.shadow_type == ShadowType::kDynamic)
        {
          data.culler.back().setCullFrequency(std::max(1u, 10u / data.dynamic_frequency));
        }
        else
        {
          data.culler.back().setCullFrequency(1u);
        }

        utilities::Frustum frustum;
        frustum.construct(data.projection.back(), data.view.back());

        // Render to the shadow map.
        mesh_render_system_->renderAll(data.culler.back(), frustum);

        // Set up the post processing passes.
        renderer->setMesh(full_screen_mesh_);
        renderer->setSubMesh(0u);
        renderer->setRasterizerState(platform::RasterizerState::SolidBack());
        renderer->setBlendState(platform::BlendState::Alpha());

        for (const asset::ShaderHandle& it : shaders_point_.shader_modify)
        {
          renderer->bindShaderPass(
            platform::ShaderPass(
              Name("shadow_modify"),
              it,
              { shadow_map },
              { shadow_map }
            )
          );
          renderer->draw();
        }
      }

      // Render lights to the light map.
      // Set up the post processing passes.
      renderer->setMesh(full_screen_mesh_);
      renderer->setSubMesh(0u);
      renderer->setRasterizerState(platform::RasterizerState::SolidBack());

      // Render light using the shadow map.
      renderer->bindShaderPass(
        platform::ShaderPass(
          Name("shadow_publish"),
          shaders_point_.shader_publish,
          {
            shadow_map,
            platform::RenderTarget(Name("texture"), data.texture),
            world_->getPostProcessManager().getTarget(Name("position")),
            world_->getPostProcessManager().getTarget(Name("normal")),
            world_->getPostProcessManager().getTarget(Name("metallic_roughness"))
          }, {
            world_->getPostProcessManager().getTarget(Name("light_map"))
          }
          )
      );

      // Required by: dir, point and spot.
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_direction"), -forward));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_colour"),     data.colour * data.intensity));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_ambient"),    data.ambient));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_far"),        data.depth.back()));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_position"),   transform.getWorldTranslation()));
          
      renderer->setBlendState(platform::BlendState(
        false,                                /*alpha_to_coverage*/
        true,                                 /*blend_enabled*/
        platform::BlendState::BlendMode::kOne /*src_blend*/,
        platform::BlendState::BlendMode::kOne /*dest_blend*/,
        platform::BlendState::BlendOp::kAdd   /*blend_op*/,
        platform::BlendState::BlendMode::kOne /*src_blend_alpha*/,
        platform::BlendState::BlendMode::kOne /*dest_blend_alpha*/,
        platform::BlendState::BlendOp::kAdd   /*blend_op_alpha*/,
        (unsigned char)platform::BlendState::WriteMode::kColourWriteEnableRGB /*write_mask*/
      ));
      renderer->draw();
      renderer->setBlendState(platform::BlendState::Alpha());
    }
    void LightSystem::renderCascade(const entity::Entity& entity)
    {
      foundation::SharedPointer<platform::IRenderer> renderer = world_->getRenderer();
      LightData& data = lookUpData(entity);
      
      const TransformComponent transform = transform_system_->getComponent(entity);
      const glm::vec3 forward = glm::normalize(transform.getWorldForward());

      const bool update = (data.shadow_type == ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != ShadowType::kGenerated);

      if (data.dynamic_index >= data.dynamic_frequency)
      {
        data.dynamic_index = 0u;
      }
        
      // Generate shadow maps.
      if (update && data.shadow_type != ShadowType::kNone)
      {
        if (data.shadow_type == ShadowType::kGenerateOnce)
        {
          data.shadow_type = ShadowType::kGenerated;
        }

        entity::Entity main_camera = camera_system_->getMainCamera();
        const float camera_near = camera_system_->getNearPlane(main_camera).asMeter();
        const float camera_far = camera_system_->getFarPlane(main_camera).asMeter();
        const float camera_diff = camera_far - camera_near;

        const uint8_t cascade_count = 3u;
        const float cascade_multipliers[cascade_count - 1u] = {
          0.05f, 0.25f
        };
        const float cascade_near[cascade_count] = {
          camera_near,
          camera_near + camera_diff * cascade_multipliers[0u],
          camera_near + camera_diff * cascade_multipliers[1u],
        };
        const float cascade_far[cascade_count] = {
          camera_near + camera_diff * cascade_multipliers[0u],
          camera_near + camera_diff * cascade_multipliers[1u],
          camera_far,
        };

        for (uint8_t i = 0u; i < cascade_count; ++i)
        {
          // Get the camera matrices for this cascade.
          glm::mat4x4 camera_projection = glm::perspective(
            camera_system_->getFov(main_camera).asRad(),
            (float)world_->getWindow()->getSize().x / (float)world_->getWindow()->getSize().y,
            cascade_near[i],
            cascade_far[i]
          );
          glm::mat4x4 camera_view = glm::inverse(transform_system_->getWorld(main_camera));

          // Get the camera frustum for this cascade.
          utilities::Frustum main_camera_frustum;
          main_camera_frustum.construct(camera_projection, camera_view);
          Vector<glm::vec3> corners = main_camera_frustum.getCorners();

          // Get the center of the camera frustum for this cascade.
          glm::vec3 center(0.0f);
          for (const glm::vec3& corner : corners)
          {
            center += corner;
          }
          center /= 8.0f;

          float radius = glm::length(corners.at(0u) - corners.at(6u)) * 0.25f;

          // Remove shimmering
          float texels_per_unit = (float)data.render_target.at(i).getTexture()->getLayer(0u).getWidth() / (radius * 2.0f);
          glm::vec3 scalar(texels_per_unit);

          glm::mat4x4 look_at = glm::lookAtRH(glm::vec3(0.0f), -forward, glm::vec3(0.0f, 1.0f, 0.0f));
          look_at *= glm::vec4(scalar, 1.0f);
          glm::mat4x4 look_at_inv = glm::inverse(look_at);

          center = glm::vec3(glm::vec4(center, 1.0f) * look_at);
          center.x = std::floorf(center.x);
          center.y = std::floorf(center.y);
          center = glm::vec3(glm::vec4(center, 1.0f) * look_at_inv);

          // Construct a frustum out of the min and max.
          data.view_position.at(i) = center;
          data.view.at(i)          = glm::lookAtRH(data.view_position.at(i), data.view_position.at(i) + forward, glm::vec3(0.0f, 1.0f, 0.0f));
          data.projection.at(i)    = glm::orthoRH(-radius, radius, -radius, radius, -100.0f, 100.0f);
          data.depth.at(i)         = 200.0f;

          world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_view_projection_matrix"), data.projection.at(i) * data.view.at(i)));

          // Clear the shadow map.
          renderer->clearRenderTarget(data.render_target.at(i).getTexture(), glm::vec4(0.0f));
          renderer->clearRenderTarget(data.depth_target.at(i).getTexture(), glm::vec4(1.0f));

          // Render to the shadow map.
          renderer->bindShaderPass(
            platform::ShaderPass(
              Name("shadow_generate"),
              shaders_cascade_.shader_generate,
              {},
              { data.render_target.at(i), data.depth_target.at(i) }
            )
          );

          // Handle frustum culling.
          if (data.shadow_type == ShadowType::kDynamic)
          {
            data.culler.at(i).setCullFrequency(std::max(1u, 10u / data.dynamic_frequency));
          }
          else
          {
            data.culler.at(i).setCullFrequency(1u);
          }

          utilities::Frustum frustum;
          frustum.construct(data.projection.at(i), data.view.at(i));

          // Render to the shadow map.
          mesh_render_system_->renderAll(data.culler.at(i), frustum);

          // Set up the post processing passes.
          renderer->setMesh(full_screen_mesh_);
          renderer->setSubMesh(0u);
          renderer->setRasterizerState(platform::RasterizerState::SolidBack());
          renderer->setBlendState(platform::BlendState::Alpha());

          for (const asset::ShaderHandle& it : shaders_cascade_.shader_modify)
          {
            renderer->bindShaderPass(
              platform::ShaderPass(
                Name("shadow_modify"),
                it,
                { data.render_target.at(i) },
                { data.render_target.at(i) }
              )
            );
            renderer->draw();
          }
        }
      }

      // Render lights to the light map.
      // Set up the post processing passes.
      renderer->setMesh(full_screen_mesh_);
      renderer->setSubMesh(0u);
      renderer->setRasterizerState(platform::RasterizerState::SolidBack());

      // Render light using the shadow map.
      renderer->bindShaderPass(
        platform::ShaderPass(
          Name("shadow_publish"),
          shaders_cascade_.shader_publish,
          {
            data.render_target.at(0u),
            data.render_target.at(1u),
            data.render_target.at(2u),
            platform::RenderTarget(Name("texture"), data.texture),
            world_->getPostProcessManager().getTarget(Name("position")),
            world_->getPostProcessManager().getTarget(Name("normal")),
            world_->getPostProcessManager().getTarget(Name("metallic_roughness"))
          }, {
            world_->getPostProcessManager().getTarget(Name("light_map"))
          }
        )
      );

      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_view_projection_matrix_01"), data.projection.at(0u) * data.view.at(0u)));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_view_projection_matrix_02"), data.projection.at(1u) * data.view.at(1u)));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_view_projection_matrix_03"), data.projection.at(2u) * data.view.at(2u)));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_camera_position_01"),        data.view_position.at(0u)));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_camera_position_02"),        data.view_position.at(1u)));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_camera_position_03"),        data.view_position.at(2u)));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_far_01"),                    data.depth.at(0u)));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_far_02"),                    data.depth.at(1u)));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_far_03"),                    data.depth.at(2u)));

      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_direction"), -forward));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_colour"),     data.colour * data.intensity));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("light_ambient"),    data.ambient));
         
      renderer->setBlendState(platform::BlendState(
        false,                                /*alpha_to_coverage*/
        true,                                 /*blend_enabled*/
        platform::BlendState::BlendMode::kOne /*src_blend*/,
        platform::BlendState::BlendMode::kOne /*dest_blend*/,
        platform::BlendState::BlendOp::kAdd   /*blend_op*/,
        platform::BlendState::BlendMode::kOne /*src_blend_alpha*/,
        platform::BlendState::BlendMode::kOne /*dest_blend_alpha*/,
        platform::BlendState::BlendOp::kAdd   /*blend_op_alpha*/,
        (unsigned char)platform::BlendState::WriteMode::kColourWriteEnableRGB /*write_mask*/
      ));
      renderer->draw();
      renderer->setBlendState(platform::BlendState::Alpha());
    }
    BaseLightComponent::BaseLightComponent(const entity::Entity& entity, LightSystem* system) :
      IComponent(entity), system_(system)
    {
    }
    BaseLightComponent::BaseLightComponent(const BaseLightComponent& other) :
      BaseLightComponent(other.entity_, other.system_)
    {
    }
    BaseLightComponent::BaseLightComponent() :
      BaseLightComponent(entity::InvalidEntity, nullptr)
    {
    }
    DirectionalLightComponent::DirectionalLightComponent(const entity::Entity& entity, LightSystem* system) :
      BaseLightComponent(entity, system)
    {
    }
    DirectionalLightComponent::DirectionalLightComponent(const DirectionalLightComponent& other) :
      BaseLightComponent(other)
    {
    }
    DirectionalLightComponent::DirectionalLightComponent() :
      BaseLightComponent()
    {
    }
    void BaseLightComponent::setSize(const float& size)
    {
      system_->setSize(entity_, size);
    }
    float BaseLightComponent::getSize() const
    {
      return system_->getSize(entity_);
    }
    PointLightComponent::PointLightComponent(const entity::Entity& entity, LightSystem* system) :
      BaseLightComponent(entity, system)
    {
    }
    PointLightComponent::PointLightComponent(const PointLightComponent& other) :
      BaseLightComponent(other)
    {
    }
    PointLightComponent::PointLightComponent() :
      BaseLightComponent()
    {
    }
    SpotLightComponent::SpotLightComponent(const entity::Entity& entity, LightSystem* system) :
      BaseLightComponent(entity, system)
    {
    }
    SpotLightComponent::SpotLightComponent(const SpotLightComponent& other) :
      BaseLightComponent(other)
    {
    }
    SpotLightComponent::SpotLightComponent() :
      BaseLightComponent()
    {
    }
    void BaseLightComponent::setCutOff(const utilities::Angle& cut_off)
    {
      system_->setCutOff(entity_, cut_off);
    }
    utilities::Angle BaseLightComponent::getCutOff() const
    {
      return system_->getCutOff(entity_);
    }
    void BaseLightComponent::setOuterCutOff(const utilities::Angle& outer_cut_off)
    {
      system_->setOuterCutOff(entity_, outer_cut_off);
    }
    utilities::Angle BaseLightComponent::getOuterCutOff() const
    {
      return system_->getOuterCutOff(entity_);
    }
    void BaseLightComponent::setShadowType(const ShadowType& shadow_type)
    {
      system_->setShadowType(entity_, shadow_type);
    }
    ShadowType BaseLightComponent::getShadowType() const
    {
      return system_->getShadowType(entity_);
    }
    void BaseLightComponent::setColour(const glm::vec3& colour)
    {
      system_->setColour(entity_, colour);
    }
    glm::vec3 BaseLightComponent::getColour() const
    {
      return system_->getColour(entity_);
    }
    void BaseLightComponent::setAmbient(const glm::vec3& ambient)
    {
      system_->setAmbient(entity_, ambient);
    }
    glm::vec3 BaseLightComponent::getAmbient() const
    {
      return system_->getAmbient(entity_);
    }
    void BaseLightComponent::setIntensity(const float& intensity)
    {
      system_->setIntensity(entity_, intensity);
    }
    float BaseLightComponent::getIntensity() const
    {
      return system_->getIntensity(entity_);
    }
    void BaseLightComponent::setRenderTarget(Vector<platform::RenderTarget> render_target)
    {
      system_->setRenderTarget(entity_, render_target);
    }
    Vector<platform::RenderTarget> BaseLightComponent::getRenderTarget() const
    {
      return system_->getRenderTarget(entity_);
    }
    void BaseLightComponent::setTexture(asset::VioletTextureHandle texture)
    {
      system_->setTexture(entity_, texture);
    }
    asset::VioletTextureHandle BaseLightComponent::getTexture() const
    {
      return system_->getTexture(entity_);
    }
    void BaseLightComponent::setDepth(const float& depth)
    {
      system_->setDepth(entity_, depth);
    }
    float BaseLightComponent::getDepth() const
    {
      return system_->getDepth(entity_);
    }
    void BaseLightComponent::setEnabled(const bool& enabled)
    {
      system_->setEnabled(entity_, enabled);
    }
    bool BaseLightComponent::getEnabled() const
    {
      return system_->getEnabled(entity_);
    }
    void BaseLightComponent::setDynamicFrequency(const uint8_t& frequency)
    {
      system_->setDynamicFrequency(entity_, frequency);
    }
    uint8_t BaseLightComponent::getDynamicFrequency() const
    {
      return system_->getDynamicFrequency(entity_);
    }
    void BaseLightComponent::setLightType(LightType type)
    {
      system_->setLightType(entity_, type);
    }
    LightType BaseLightComponent::getLightType() const
    {
      return system_->getLightType(entity_);
    }
    void BaseLightComponent::setRSM(bool rsm)
    {
      system_->setRSM(entity_, rsm);
    }
    bool BaseLightComponent::getRSM() const
    {
      return system_->getRSM(entity_);
    }
    LightData::LightData(const LightData & other)
    {
      type              = other.type;
      shadow_type       = other.shadow_type;
      colour            = other.colour;
      ambient           = other.ambient;
      intensity         = other.intensity;
      render_target     = other.render_target;
      depth_target      = other.depth_target;
      texture           = other.texture;
      culler            = other.culler;
      entity            = other.entity;
      cut_off           = other.cut_off;
      outer_cut_off     = other.outer_cut_off;
      size              = other.size;
      enabled           = other.enabled;
      rsm               = other.rsm;
      dynamic_frequency = other.dynamic_frequency;
      dynamic_index     = other.dynamic_index;
      depth             = other.depth;
      projection        = other.projection;
      view              = other.view;
      view_position     = other.view_position;
    }
    LightData & LightData::operator=(const LightData & other)
    {
      type              = other.type;
      shadow_type       = other.shadow_type;
      colour            = other.colour;
      ambient           = other.ambient;
      intensity         = other.intensity;
      render_target     = other.render_target;
      depth_target      = other.depth_target;
      texture           = other.texture;
      culler            = other.culler;
      entity            = other.entity;
      cut_off           = other.cut_off;
      outer_cut_off     = other.outer_cut_off;
      size              = other.size;
      enabled           = other.enabled;
      rsm               = other.rsm;
      dynamic_frequency = other.dynamic_frequency;
      dynamic_index     = other.dynamic_index;
      depth             = other.depth;
      projection        = other.projection;
      view              = other.view;
      view_position     = other.view_position;

      return *this;
    }
  }
}
