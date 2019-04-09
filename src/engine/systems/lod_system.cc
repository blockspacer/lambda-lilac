#include <systems/lod_system.h>
#include <interfaces/iworld.h>
#include <platform/scene.h>

#include <algorithm>

namespace lambda
{
	namespace components
	{
		void LOD::setMesh(asset::VioletMeshHandle mesh)
		{
			mesh_ = mesh;
		}
		void LOD::setDistance(const float& distance)
		{
			distance_ = distance;
		}
		asset::VioletMeshHandle LOD::getMesh() const
		{
			return mesh_;
		}
		float LOD::getDistance() const
		{
			return distance_;
		}

		namespace LODSystem
		{
			LODComponent LODSystem::addComponent(const entity::Entity& entity, world::SceneData& scene)
			{
				if (!TransformSystem::hasComponent(entity, scene))
					TransformSystem::addComponent(entity, scene);

				if (!MeshRenderSystem::hasComponent(entity, scene))
					MeshRenderSystem::addComponent(entity, scene);

				scene.lod.add(entity);

				auto& data = scene.lod.get(entity);
				data.base_lod.setDistance(0.0f);
				data.base_lod.setMesh(MeshRenderSystem::getMesh(entity, scene));

				return LODComponent(entity, scene);
			}
			LODComponent getComponent(const entity::Entity& entity, world::SceneData& scene)
			{
				return LODComponent(entity, scene);
			}
			bool hasComponent(const entity::Entity& entity, world::SceneData& scene)
			{
				return scene.lod.has(entity);
			}
			void removeComponent(const entity::Entity& entity, world::SceneData& scene)
			{
				scene.lod.remove(entity);
			}
			void collectGarbage(world::SceneData& scene)
			{
				if (!scene.lod.marked_for_delete.empty())
				{
					for (entity::Entity entity : scene.lod.marked_for_delete)
					{
						const auto& it = scene.lod.entity_to_data.find(entity);
						if (it != scene.lod.entity_to_data.end())
						{
							uint32_t idx = it->second;
							scene.lod.unused_data_entries.push(idx);
							scene.lod.data_to_entity.erase(idx);
							scene.lod.entity_to_data.erase(entity);
							scene.lod.data[idx].valid = false;
						}
					}
					scene.lod.marked_for_delete.clear();
				}
			}
			void deinitialize(world::SceneData& scene)
			{
				Vector<entity::Entity> entities;
				for (const auto& it : scene.lod.entity_to_data)
					entities.push_back(it.first);

				for (const auto& entity : entities)
					scene.lod.remove(entity);
				collectGarbage(scene);
			}
			void setBaseLOD(const entity::Entity& entity, const LOD& lod, world::SceneData& scene)
			{
				scene.lod.get(entity).base_lod = lod;
			}
			void addLOD(const entity::Entity& entity, const LOD& lod, world::SceneData& scene)
			{
				auto& data = scene.lod.get(entity);
				data.lods.push_back(lod);

				std::sort(data.lods.begin(), data.lods.end(), std::greater<LOD>());
			}
			LOD getBaseLOD(const entity::Entity& entity, world::SceneData& scene)
			{
				return scene.lod.get(entity).base_lod;
			}
			Vector<LOD> getLODs(const entity::Entity& entity, world::SceneData& scene)
			{
				return scene.lod.get(entity).lods;
			}
		}

		// The system data.
		namespace LODSystem
		{
			Data& SystemData::add(const entity::Entity& entity)
			{
				uint32_t idx = 0ul;
				if (!unused_data_entries.empty())
				{
					idx = unused_data_entries.front();
					unused_data_entries.pop();
					data[idx] = Data(entity);
				}
				else
				{
					idx = (uint32_t)data.size();
					data.push_back(Data(entity));
					data_to_entity[idx] = entity;
				}

				data_to_entity[idx] = entity;
				entity_to_data[entity] = idx;

				return data[idx];
			}

			Data& SystemData::get(const entity::Entity& entity)
			{
				auto it = entity_to_data.find(entity);
				LMB_ASSERT(it != entity_to_data.end(), "LOD: %llu does not have a component", entity);
				LMB_ASSERT(data[it->second].valid, "LOD: %llu's data was not valid", entity);
				return data[it->second];
			}

			void SystemData::remove(const entity::Entity& entity)
			{
				marked_for_delete.insert(entity);
			}

			bool SystemData::has(const entity::Entity& entity)
			{
				return entity_to_data.find(entity) != entity_to_data.end();
			}
		}

		namespace LODSystem
		{
			Data::Data(const Data& other)
			{
				lods = other.lods;
				base_lod = other.base_lod;
				entity = other.entity;
				valid = other.valid;
			}
			Data& Data::operator=(const Data& other)
			{
				lods = other.lods;
				base_lod = other.base_lod;
				entity = other.entity;
				valid = other.valid;

				return *this;
			}
		}

		LODComponent::LODComponent(const entity::Entity& entity, world::SceneData& scene) :
			IComponent(entity), scene_(&scene)
		{
		}
		LODComponent::LODComponent(const LODComponent& other) :
			IComponent(other.entity_), scene_(other.scene_)
		{
		}
		LODComponent::LODComponent() :
			IComponent(entity::Entity()), scene_(nullptr)
		{
		}
		void LODComponent::setBaseLOD(const LOD& lod)
		{
			LODSystem::setBaseLOD(entity_, lod, *scene_);
		}
		LOD LODComponent::getBaseLOD() const
		{
			return LODSystem::getBaseLOD(entity_, *scene_);
		}
		void LODComponent::addLOD(const LOD& lod)
		{
			return LODSystem::addLOD(entity_, lod, *scene_);
		}
		Vector<LOD> LODComponent::getLODs() const
		{
			return LODSystem::getLODs(entity_, *scene_);
		}
	}
}