#include "camera_system.h"
#include "interfaces/iworld.h"
#include "transform_system.h"
#include "mesh_render_system.h"
#include "platform/depth_stencil_state.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <utils/utilities.h>
#include "platform/debug_renderer.h"

namespace lambda
{
  namespace components
  {
    CameraComponent::CameraComponent(const entity::Entity& entity, CameraSystem* system) :
      IComponent(entity), system_(system)
    {
    }
    CameraComponent::CameraComponent(const CameraComponent& other) :
      IComponent(other.entity()), system_(other.system_)
    {
    }
    CameraComponent::CameraComponent() :
      IComponent(entity::Entity()), system_(nullptr)
    {
    }

    void CameraComponent::setFov(const utilities::Angle& fov)
    {
      system_->setFov(entity_, fov);
    }

    utilities::Angle CameraComponent::getFov() const
    {
      return system_->getFov(entity_);
    }

    void CameraComponent::setNearPlane(const utilities::Distance& near_plane)
    {
      system_->setNearPlane(entity_, near_plane);
    }

    utilities::Distance CameraComponent::getNearPlane() const
    {
      return system_->getNearPlane(entity_);
    }

    void CameraComponent::setFarPlane(const utilities::Distance& far_plane)
    {
      system_->setFarPlane(entity_, far_plane);
    }

    utilities::Distance CameraComponent::getFarPlane() const
    {
      return system_->getFarPlane(entity_);
    }

    void CameraComponent::addShaderPass(const platform::ShaderPass & shader_pass)
    {
      system_->addShaderPass(entity_, shader_pass);
    }

    void CameraComponent::setShaderPasses(const Vector<platform::ShaderPass>& shader_pass)
    {
      system_->setShaderPasses(entity_, shader_pass);
    }

    platform::ShaderPass CameraComponent::getShaderPass(uint32_t id) const
    {
      return system_->getShaderPass(entity_, id);
    }

    Vector<platform::ShaderPass> CameraComponent::getShaderPasses() const
    {
      return system_->getShaderPasses(entity_);
    }





    CameraSystem::~CameraSystem()
    {
    }
    void CameraSystem::initialize(world::IWorld& world)
    {
      transform_system_   = world.getScene().getSystem<TransformSystem>();
      mesh_render_system_ = world.getScene().getSystem<MeshRenderSystem>();
      world_              =&world;

      main_camera_culler_.setCullFrequency(10u);
      main_camera_culler_.setShouldCull(true);
      main_camera_culler_.setCullShadowCasters(false);
    }
    void CameraSystem::deinitialize()
    {
      mesh_render_system_.reset();
      transform_system_.reset();
    }
    void CameraSystem::onRender()
    {
      // Check if there is no camera. If so, the program should exit.
      if (main_camera_ == 0u)
      {
        LMB_ASSERT(false, "There was no camera set");
        return;
      }

      bindCamera(main_camera_);
      const CameraData& data = lookUpData(main_camera_);

      // Create render list.
      Vector<utilities::Renderable*> opaque;
      Vector<utilities::Renderable*> alpha;
      mesh_render_system_->createRenderList(main_camera_culler_, main_camera_frustum_);
      auto statics  = main_camera_culler_.getStatics();
      auto dynamics = main_camera_culler_.getDynamics();
      mesh_render_system_->createSortedRenderList(&statics, &dynamics, opaque, alpha);

      // Draw all passes.
      world_->getRenderer()->beginTimer("Main Camera");
      for (uint32_t i = 0u; i < data.shader_passes.size(); ++i)
      {
        //if (i == 0u)
          world_->getRenderer()->setDepthStencilState(platform::DepthStencilState::Default());
        //else
        //  world_->getRenderer()->setDepthStencilState(platform::DepthStencilState::Equal());
        world_->getRenderer()->pushMarker("Main Camera");
        world_->getRenderer()->bindShaderPass(data.shader_passes[i]);
        mesh_render_system_->renderAll(opaque, alpha);
        world_->getRenderer()->popMarker();
      }
      world_->getRenderer()->endTimer("Main Camera");

      // Reset the depth stencil state. // TODO (Hilze): Find out how to handle depth stencil state.
      world_->getRenderer()->setDepthStencilState(platform::DepthStencilState::Default());
    }
    CameraComponent CameraSystem::addComponent(const entity::Entity& entity)
    {
      if (false == transform_system_->hasComponent(entity))
        transform_system_->addComponent(entity);
			
			if (!unused_data_entries_.empty())
			{
				uint32_t idx = unused_data_entries_.front();
				unused_data_entries_.pop();
				
				data_[idx]              = CameraData(entity);
				data_to_entity_[idx]    = entity;
				entity_to_data_[entity] = idx;
			}
			else
			{
				data_.push_back(CameraData(entity));
				uint32_t idx = (uint32_t)data_.size() - 1u;
				data_to_entity_[idx]    = entity;
				entity_to_data_[entity] = idx;
			}
      
      if (main_camera_ == 0u)
        setMainCamera(entity);
      
      return CameraComponent(entity, this);
    }
    CameraComponent CameraSystem::getComponent(const entity::Entity& entity)
    {
      return CameraComponent(entity, this);
    }
    bool CameraSystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity) != entity_to_data_.end();
    }
    void CameraSystem::removeComponent(const entity::Entity& entity)
    {
      marked_for_delete_.insert(entity);
    }
		void CameraSystem::collectGarbage()
		{
			if (!marked_for_delete_.empty())
			{
				for (entity::Entity entity : marked_for_delete_)
				{
					const auto& it = entity_to_data_.find(entity);
					if (it != entity_to_data_.end())
					{
						uint32_t idx = it->second;
						unused_data_entries_.push(idx);
						data_to_entity_.erase(idx);
						entity_to_data_.erase(entity);
						data_[idx].valid = false;
					}
				}
				marked_for_delete_.clear();
			}
		}
    void CameraSystem::setFov(const entity::Entity& entity, const utilities::Angle& fov)
    {
      lookUpData(entity).fov = fov;
    }
    utilities::Angle CameraSystem::getFov(const entity::Entity& entity) const
    {
      return lookUpData(entity).fov;
    }
    void CameraSystem::setNearPlane(const entity::Entity& entity, const utilities::Distance& near_plane)
    {
      lookUpData(entity).near_plane = near_plane;
    }
    utilities::Distance CameraSystem::getNearPlane(const entity::Entity& entity) const
    {
      return lookUpData(entity).near_plane;
    }
    void CameraSystem::setFarPlane(const entity::Entity& entity, const utilities::Distance& far_plane)
    {
      lookUpData(entity).far_plane = far_plane;
    }
    utilities::Distance CameraSystem::getFarPlane(const entity::Entity& entity) const
    {
      return lookUpData(entity).far_plane;
    }
    void CameraSystem::addShaderPass(const entity::Entity & entity, const platform::ShaderPass & shader_pass)
    {
      lookUpData(entity).shader_passes.push_back(shader_pass);
    }
    void CameraSystem::setShaderPasses(const entity::Entity & entity, const Vector<platform::ShaderPass>& shader_pass)
    {
      lookUpData(entity).shader_passes = shader_pass;
    }
    platform::ShaderPass CameraSystem::getShaderPass(const entity::Entity & entity, uint32_t id) const
    {
      return lookUpData(entity).shader_passes[id];
    }
    Vector<platform::ShaderPass> CameraSystem::getShaderPasses(const entity::Entity & entity) const
    {
      return lookUpData(entity).shader_passes;
    }
    void CameraSystem::bindCamera(const entity::Entity& entity)
    {
      CameraData& data = lookUpData(entity);

      TransformComponent transform = transform_system_->getComponent(data.entity);
      const glm::mat4x4 view = glm::inverse(transform.getWorld());
      const glm::mat4x4 projection = glm::perspective(
        data.fov.asRad(),
        (float)world_->getWindow()->getSize().x / (float)world_->getWindow()->getSize().y,
        data.near_plane.asMeter(),
        data.far_plane.asMeter()
      );

      // Update the frustum.
      main_camera_frustum_.construct(
        projection,
        view
      );
      
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("view_matrix"), view));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("inverse_view_matrix"), glm::inverse(view)));

      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("projection_matrix"), projection));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("inverse_projection_matrix"), glm::inverse(projection)));

			world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("view_projection_matrix"), projection * view));
			world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("inverse_view_projection_matrix"), glm::inverse(projection * view)));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("camera_position"), transform.getWorldTranslation()));

      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("camera_near"), data.near_plane.asMeter()));
      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("camera_far"), data.far_plane.asMeter()));
    }
    entity::Entity CameraSystem::getMainCamera()
    {
      return main_camera_;
    }
    void CameraSystem::setMainCamera(const entity::Entity& main_camera)
    {
      main_camera_ = main_camera;
    }
    CameraData& CameraSystem::lookUpData(const entity::Entity& entity)
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }
    const CameraData& CameraSystem::lookUpData(const entity::Entity& entity) const
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }
    CameraData::CameraData(const CameraData & other)
    {
      fov           = other.fov;
      near_plane    = other.near_plane;
      far_plane     = other.far_plane;
      shader_passes = other.shader_passes;
      entity        = other.entity;
			valid         = other.valid;
    }
    CameraData & CameraData::operator=(const CameraData & other)
    {
      fov           = other.fov;
      near_plane    = other.near_plane;
      far_plane     = other.far_plane;
      shader_passes = other.shader_passes;
      entity        = other.entity;
			valid         = other.valid;
      return *this;
    }
}
}
