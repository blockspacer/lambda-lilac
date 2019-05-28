#pragma once
#include "interfaces/icomponent.h"
#include "assets/mesh.h"
#include "interfaces/iwindow.h"
#include "assets/mesh_io.h"
#include "utils/bvh.h"
#include "utils/renderable.h"

namespace lambda
{
	namespace utilities
	{
		class Frustum;
		class Culler;
		struct Renderable;
		struct LinkedNode;
	}

	namespace scene
	{
		struct Scene;
	}

	namespace components
	{
		class MeshRenderComponent : public IComponent
		{
		public:
			MeshRenderComponent(const entity::Entity& entity, scene::Scene& scene);
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
			void setEmissiveness(const glm::vec3& emissiveness);
			glm::vec3 getEmissiveness() const;
			void setAlbedoTexture(asset::VioletTextureHandle texture);
			asset::VioletTextureHandle getAlbedoTexture() const;
			void setNormalTexture(asset::VioletTextureHandle texture);
			asset::VioletTextureHandle getNormalTexture() const;
			void setDMRATexture(asset::VioletTextureHandle texture);
			asset::VioletTextureHandle getDMRATexture() const;
			void setEmissiveTexture(asset::VioletTextureHandle texture);
			asset::VioletTextureHandle getEmissiveTexture() const;
			void attachMesh(asset::VioletMeshHandle mesh);
			bool getVisible() const;
			void setVisible(const bool& visible);
			bool getCastShadows() const;
			void setCastShadows(const bool& cast_shadows);

		private:
			scene::Scene* scene_;
		};

		namespace MeshRenderSystem
		{
			struct Data
			{
				Data() {}
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				asset::VioletMeshHandle mesh;
				uint32_t sub_mesh = 0u;
				asset::VioletTextureHandle albedo_texture;
				asset::VioletTextureHandle normal_texture;
				asset::VioletTextureHandle dmra_texture;
				asset::VioletTextureHandle emissive_texture;
				float metallicness = 0.0f;
				float roughness    = 1.0f;
				glm::vec3 emissiveness = glm::vec3(0.0f, 0.0f, 0.0f);
				bool visible       = true;
				bool cast_shadows  = true;
				bool valid         = true;
				utilities::Renderable renderable;

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

				Vector<uint32_t>         dynamic_renderables;
				Vector<uint32_t>         static_renderables;
				utilities::BVH*          static_bvh;
				utilities::TransientBVH* dynamic_bvh;

				asset::VioletTextureHandle default_albedo;
				asset::VioletTextureHandle default_normal;
				asset::VioletTextureHandle default_dmra;
				asset::VioletTextureHandle default_emissive;
			};

			MeshRenderComponent addComponent(const entity::Entity& entity, scene::Scene& scene);
			MeshRenderComponent getComponent(const entity::Entity& entity, scene::Scene& scene);
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene);
			void removeComponent(const entity::Entity& entity, scene::Scene& scene);

			void collectGarbage(scene::Scene& scene);
			void initialize(scene::Scene& scene);
			void deinitialize(scene::Scene& scene);
			void updateDynamicsBvh(scene::Scene& scene);

			void setMesh(const entity::Entity& entity, asset::VioletMeshHandle mesh, scene::Scene& scene);
			void setSubMesh(const entity::Entity& entity, const uint32_t& sub_mesh, scene::Scene& scene);
			void setAlbedoTexture(const entity::Entity& entity, asset::VioletTextureHandle texture, scene::Scene& scene);
			void setNormalTexture(const entity::Entity& entity, asset::VioletTextureHandle texture, scene::Scene& scene);
			void setDMRATexture(const entity::Entity& entity, asset::VioletTextureHandle texture, scene::Scene& scene);
			void setEmissiveTexture(const entity::Entity& entity, asset::VioletTextureHandle texture, scene::Scene& scene);
			void setMetallicness(const entity::Entity& entity, const float& metallicness, scene::Scene& scene);
			void setRoughness(const entity::Entity& entity, const float& roughness, scene::Scene& scene);
			void setEmissiveness(const entity::Entity& entity, const glm::vec3& emissiveness, scene::Scene& scene);
			asset::VioletMeshHandle getMesh(const entity::Entity& entity, scene::Scene& scene);
			uint32_t getSubMesh(const entity::Entity& entity, scene::Scene& scene);
			asset::VioletTextureHandle getAlbedoTexture(const entity::Entity& entity, scene::Scene& scene);
			asset::VioletTextureHandle getNormalTexture(const entity::Entity& entity, scene::Scene& scene);
			asset::VioletTextureHandle getDMRATexture(const entity::Entity& entity, scene::Scene& scene);
			asset::VioletTextureHandle getEmissiveTexture(const entity::Entity& entity, scene::Scene& scene);
			float getMetallicness(const entity::Entity& entity, scene::Scene& scene);
			float getRoughness(const entity::Entity& entity, scene::Scene& scene);
			glm::vec3 getEmissiveness(const entity::Entity& entity, scene::Scene& scene);
			void attachMesh(const entity::Entity& entity, asset::VioletMeshHandle mesh, scene::Scene& scene);
			bool getVisible(const entity::Entity& entity, scene::Scene& scene);
			void setVisible(const entity::Entity& entity, const bool& visible, scene::Scene& scene);
			bool getCastShadows(const entity::Entity& entity, scene::Scene& scene);
			void setCastShadows(const entity::Entity& entity, const bool& cast_shadows, scene::Scene& scene);
			void makeStatic(const entity::Entity& entity, scene::Scene& scene);
			void makeDynamic(const entity::Entity& entity, scene::Scene& scene);

			void createRenderList(utilities::Culler& culler, const utilities::Frustum& frustum, scene::Scene& scene);
			void createSortedRenderList(utilities::LinkedNode* linked_node, Vector<utilities::Renderable*>& opaque, Vector<utilities::Renderable*>& alpha, scene::Scene& scene);
			void renderAll(utilities::Culler& culler, const utilities::Frustum& frustum, scene::Scene& scene, bool is_rh = true);
			void renderAll(utilities::LinkedNode* statics, utilities::LinkedNode* dynamics, scene::Scene& scene, bool is_rh = true);
			void renderAll(const Vector<utilities::Renderable*>& opaque, const Vector<utilities::Renderable*>& alpha, scene::Scene& scene, bool is_rh = true);
		}
	}
}
