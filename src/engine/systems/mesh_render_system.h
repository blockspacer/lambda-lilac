#pragma once
#include "interfaces/isystem.h"
#include "assets/mesh.h"
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

	namespace world
	{
		struct SceneData;
	}

	namespace components
	{
		class MeshRenderComponent : public IComponent
		{
		public:
			MeshRenderComponent(const entity::Entity& entity, world::SceneData& scene);
			MeshRenderComponent(const MeshRenderComponent& other);
			MeshRenderComponent();

			void setMesh(asset::VioletMeshHandle mesh);
			asset::VioletMeshHandle getMesh() const;
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
			void setDMRATexture(asset::VioletTextureHandle texture);
			asset::VioletTextureHandle getDMRATexture() const;
			void attachMesh(asset::VioletMeshHandle mesh);
			bool getVisible() const;
			void setVisible(const bool& visible);
			bool getCastShadows() const;
			void setCastShadows(const bool& cast_shadows);

		private:
			world::SceneData* scene_;
		};

		namespace MeshRenderSystem
		{
			struct Data
			{
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				asset::VioletMeshHandle mesh;
				uint32_t sub_mesh = 0u;
				asset::VioletTextureHandle albedo_texture;
				asset::VioletTextureHandle normal_texture;
				asset::VioletTextureHandle dmra_texture;
				float metallicness = 1.0f;
				float roughness = 1.0f;
				bool visible = true;
				bool cast_shadows = true;
				bool valid = true;

				entity::Entity entity;
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

				Vector<entity::Entity>         dynamic_renderables;
				Vector<utilities::Renderable*> static_renderables;
				utilities::ZoneManager         static_zone_manager;

				asset::VioletTextureHandle default_albedo;
				asset::VioletTextureHandle default_normal;
				asset::VioletTextureHandle default_dmra;
			};

			MeshRenderComponent addComponent(const entity::Entity& entity, world::SceneData& scene);
			MeshRenderComponent getComponent(const entity::Entity& entity, world::SceneData& scene);
			bool hasComponent(const entity::Entity& entity, world::SceneData& scene);
			void removeComponent(const entity::Entity& entity, world::SceneData& scene);

			void  collectGarbage(world::SceneData& scene);
			void  initialize(world::SceneData& scene);
			void  deinitialize(world::SceneData& scene);

			void setMesh(const entity::Entity& entity, asset::VioletMeshHandle mesh, world::SceneData& scene);
			void setSubMesh(const entity::Entity& entity, const uint32_t& sub_mesh, world::SceneData& scene);
			void setAlbedoTexture(const entity::Entity& entity, asset::VioletTextureHandle texture, world::SceneData& scene);
			void setNormalTexture(const entity::Entity& entity, asset::VioletTextureHandle texture, world::SceneData& scene);
			void setDMRATexture(const entity::Entity& entity, asset::VioletTextureHandle texture, world::SceneData& scene);
			void setMetallicness(const entity::Entity& entity, const float& metallicness, world::SceneData& scene);
			void setRoughness(const entity::Entity& entity, const float& roughness, world::SceneData& scene);
			asset::VioletMeshHandle getMesh(const entity::Entity& entity, world::SceneData& scene);
			uint32_t getSubMesh(const entity::Entity& entity, world::SceneData& scene);
			asset::VioletTextureHandle getAlbedoTexture(const entity::Entity& entity, world::SceneData& scene);
			asset::VioletTextureHandle getNormalTexture(const entity::Entity& entity, world::SceneData& scene);
			asset::VioletTextureHandle getDMRATexture(const entity::Entity& entity, world::SceneData& scene);
			float getMetallicness(const entity::Entity& entity, world::SceneData& scene);
			float getRoughness(const entity::Entity& entity, world::SceneData& scene);
			void attachMesh(const entity::Entity& entity, asset::VioletMeshHandle mesh, world::SceneData& scene);
			bool getVisible(const entity::Entity& entity, world::SceneData& scene);
			void setVisible(const entity::Entity& entity, const bool& visible, world::SceneData& scene);
			bool getCastShadows(const entity::Entity& entity, world::SceneData& scene);
			void setCastShadows(const entity::Entity& entity, const bool& cast_shadows, world::SceneData& scene);
			void makeStatic(const entity::Entity& entity, world::SceneData& scene);
			void makeDynamic(const entity::Entity& entity, world::SceneData& scene);

			void createRenderList(utilities::Culler& culler, const utilities::Frustum& frustum, world::SceneData& scene);
			void createSortedRenderList(utilities::LinkedNode* statics, utilities::LinkedNode* dynamics, Vector<utilities::Renderable*>& opaque, Vector<utilities::Renderable*>& alpha, world::SceneData& scene);
			void renderAll(utilities::Culler& culler, const utilities::Frustum& frustum, world::SceneData& scene, bool is_rh = true);
			void renderAll(utilities::LinkedNode* statics, utilities::LinkedNode* dynamics, world::SceneData& scene, bool is_rh = true);
			void renderAll(const Vector<utilities::Renderable*>& opaque, const Vector<utilities::Renderable*>& alpha, world::SceneData& scene, bool is_rh = true);
		}
	}
}