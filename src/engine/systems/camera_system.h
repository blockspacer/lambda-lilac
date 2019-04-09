#pragma once
#include "interfaces/isystem.h"
#include "interfaces/icomponent.h"
#include "interfaces/iwindow.h"
#include "interfaces/irenderer.h"
#include "utils/angle.h"
#include "utils/distance.h"
#include "platform/shader_pass.h"
#include "platform/frustum.h"
#include "platform/culling.h"

namespace lambda
{
	namespace components
	{
		class CameraComponent : public IComponent
		{
		public:
			CameraComponent(const entity::Entity& entity, world::SceneData& scene);
			CameraComponent(const CameraComponent& other);
			CameraComponent();

			void setFov(const utilities::Angle& fov);
			utilities::Angle getFov() const;
			void setNearPlane(const utilities::Distance& near_plane);
			utilities::Distance getNearPlane() const;
			void setFarPlane(const utilities::Distance& far_plane);
			utilities::Distance getFarPlane() const;
			void addShaderPass(const platform::ShaderPass& shader_pass);
			void setShaderPasses(const Vector<platform::ShaderPass>& shader_pass);
			platform::ShaderPass getShaderPass(uint32_t id) const;
			Vector<platform::ShaderPass> getShaderPasses() const;

		private:
			world::SceneData* scene_;
		};

		namespace CameraSystem
		{
			struct Data
			{
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				utilities::Angle    fov = utilities::Angle::fromDeg(90.0f);
				utilities::Distance near_plane = utilities::Distance::fromMeter(0.1f);
				utilities::Distance far_plane = utilities::Distance::fromMeter(1000.0f);
				Vector<platform::ShaderPass> shader_passes;

				entity::Entity entity;
				bool valid = true;
			};

			struct SystemData
			{
				Vector<Data>                  data;
				Map<entity::Entity, uint32_t> entity_to_data;
				Map<uint32_t, entity::Entity> data_to_entity;
				Set<entity::Entity>           marked_for_delete;
				Queue<uint32_t>               unused_data_entries;

				Data& add(const entity::Entity& entity);
				Data& get(const entity::Entity& entity);
				void  remove(const entity::Entity& entity);
				bool  has(const entity::Entity& entity);

				entity::Entity main_camera = entity::InvalidEntity;
				utilities::Frustum main_camera_frustum;
				utilities::Culler main_camera_culler;
			};

			CameraComponent addComponent(const entity::Entity& entity, world::SceneData& scene);
			CameraComponent getComponent(const entity::Entity& entity, world::SceneData& scene);
			bool hasComponent(const entity::Entity& entity, world::SceneData& scene);
			void removeComponent(const entity::Entity& entity, world::SceneData& scene);

			void collectGarbage(world::SceneData& scene);
			void initialize(world::SceneData& scene);
			void deinitialize(world::SceneData& scene);

			void setFov(const entity::Entity& entity, const utilities::Angle& fov, world::SceneData& scene);
			utilities::Angle getFov(const entity::Entity& entity, world::SceneData& scene);
			void setNearPlane(const entity::Entity& entity, const utilities::Distance& near_plane, world::SceneData& scene);
			utilities::Distance getNearPlane(const entity::Entity& entity, world::SceneData& scene);
			void setFarPlane(const entity::Entity& entity, const utilities::Distance& far_plane, world::SceneData& scene);
			utilities::Distance getFarPlane(const entity::Entity& entity, world::SceneData& scene);
			void addShaderPass(const entity::Entity& entity, const platform::ShaderPass& shader_pass, world::SceneData& scene);
			void setShaderPasses(const entity::Entity& entity, const Vector<platform::ShaderPass>& shader_pass, world::SceneData& scene);
			platform::ShaderPass getShaderPass(const entity::Entity& entity, uint32_t id, world::SceneData& scene);
			Vector<platform::ShaderPass> getShaderPasses(const entity::Entity& entity, world::SceneData& scene);

			void bindCamera(const entity::Entity& entity, world::SceneData& scene);
			entity::Entity getMainCamera(world::SceneData& scene);
			void setMainCamera(const entity::Entity& main_camera, world::SceneData& scene);
		}
	}
}