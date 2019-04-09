#pragma once
#include <interfaces/icomponent.h>
#include <interfaces/isystem.h>
#include <assets/mesh.h>
#include <systems/mesh_render_system.h>
#include <systems/transform_system.h>
#include <systems/camera_system.h>

namespace lambda
{
	namespace components
	{
		class LOD
		{
		public:
			void setMesh(asset::VioletMeshHandle mesh);
			void setDistance(const float& distance);
			asset::VioletMeshHandle getMesh() const;
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
			asset::VioletMeshHandle mesh_;
			float distance_; // Distance AFTER which this LOD should be used.
		};

		class LODComponent : public IComponent
		{
		public:
			LODComponent(const entity::Entity& entity, world::SceneData& scene);
			LODComponent(const LODComponent& other);
			LODComponent();

			void setBaseLOD(const LOD& lod);
			LOD getBaseLOD() const;
			void addLOD(const LOD& lod);
			Vector<LOD> getLODs() const;

		private:
			world::SceneData* scene_;
		};

		namespace LODSystem
		{
			struct Data
			{
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				Vector<LOD> lods;
				LOD base_lod;
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

				float time;
				float update_frequency = 1.0f / 30.0f;
			};

			LODComponent addComponent(const entity::Entity& entity, world::SceneData& scene);
			LODComponent getComponent(const entity::Entity& entity, world::SceneData& scene);
			bool hasComponent(const entity::Entity& entity, world::SceneData& scene);
			void removeComponent(const entity::Entity& entity, world::SceneData& scene);
			
			void  collectGarbage(world::SceneData& scene);
			void  deinitialize(world::SceneData& scene);

			void setBaseLOD(const entity::Entity& entity, const LOD& lod, world::SceneData& scene);
			void addLOD(const entity::Entity& entity, const LOD& lod, world::SceneData& scene);
			LOD getBaseLOD(const entity::Entity& entity, world::SceneData& scene);
			Vector<LOD> getLODs(const entity::Entity& entity, world::SceneData& scene);

		}
	}
}