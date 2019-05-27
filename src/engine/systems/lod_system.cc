#include <systems/lod_system.h>
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
			LODComponent LODSystem::addComponent(const entity::Entity& entity, scene::Scene& scene)
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
			LODComponent getComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return LODComponent(entity, scene);
			}
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.lod.has(entity);
			}
			void removeComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.lod.remove(entity);
			}
			void collectGarbage(scene::Scene& scene)
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
			void deinitialize(scene::Scene& scene)
			{
				Vector<entity::Entity> entities;
				for (const auto& it : scene.lod.entity_to_data)
					entities.push_back(it.first);

				for (const auto& entity : entities)
					scene.lod.remove(entity);
				collectGarbage(scene);
			}
			void update(const float& delta_time, scene::Scene& scene)
			{
				// Do not update the LODs every frame. Just not worth it.
				scene.lod.time += delta_time;
				if (scene.lod.time < scene.lod.update_frequency)
					return;

				scene.lod.time -= scene.lod.update_frequency;

				// Update LODs.
				glm::vec3 camera_position = components::TransformSystem::getWorldTranslation(scene.camera.main_camera, scene);

				for (auto& data : scene.lod.data)
				{
					auto* chosen_lod = &data.base_lod;
					float distance = glm::length(components::TransformSystem::getWorldTranslation(data.entity, scene) - camera_position);

					for (auto& lod : data.lods)
					{
						if (distance > lod.getDistance())
						{
							chosen_lod = &lod;
							break;
						}
					}

					components::MeshRenderSystem::setMesh(data.entity, chosen_lod->getMesh(), scene);
				}
			}

			/////////////////////////////////////////////////////////////////////////////
			void serialize(scene::Scene& scene, scene::Serializer& serializer)
			{
				for (auto v : scene.lod.unused_data_entries.get_container())
					serializer.serialize("lod/unused_data_entries/", toString(v));
				for (auto v : scene.lod.marked_for_delete)
					serializer.serialize("lod/marked_for_delete/", toString(v));
				for (auto v : scene.lod.data_to_entity)
					serializer.serialize("lod/data_to_entity/", toString(v.first) + "|" + toString(v.second));
				for (auto v : scene.lod.entity_to_data)
					serializer.serialize("lod/entity_to_data/", toString(v.first) + "|" + toString(v.second));

				for (auto v : scene.lod.data)
				{
					String lods;
					for (const LOD& lod : v.lods)
						lods += (lods.empty() ? "" : "|") + toString(lod.getDistance()) + "|" + lod.getMesh().getName().getName();

					serializer.serialize("lod/data/entity/", toString(v.entity));
					serializer.serialize("lod/data/valid/", toString(v.valid));
					serializer.serialize("lod/data/base_lod/", toString(v.base_lod.getDistance()) + "|" + v.base_lod.getMesh().getName().getName());
					serializer.serialize("lod/data/lods/", lods);
				}
			}

			/////////////////////////////////////////////////////////////////////////////
			void deserialize(scene::Scene& scene, scene::Serializer& serializer)
			{
				scene.lod.unused_data_entries.get_container().clear();
				scene.lod.marked_for_delete.clear();
				scene.lod.data_to_entity.clear();
				scene.lod.entity_to_data.clear();
				auto data_backup = scene.lod.data;
				scene.lod.data.clear();

				for (const String& str : serializer.deserializeNamespace("lod/unused_data_entries/"))
					scene.lod.unused_data_entries.push(std::stoul(stlString(str)));
				for (const String& str : serializer.deserializeNamespace("lod/marked_for_delete/"))
					scene.lod.marked_for_delete.insert(std::stoul(stlString(str)));
				for (const String& str : serializer.deserializeNamespace("lod/entity_to_data/"))
					scene.lod.entity_to_data.insert({ std::stoul(stlString(split(str, '|')[0])), std::stoul(stlString(split(str, '|')[1])) });
				for (const String& str : serializer.deserializeNamespace("lod/data_to_entity/"))
					scene.lod.data_to_entity.insert({ std::stoul(stlString(split(str, '|')[0])), std::stoul(stlString(split(str, '|')[1])) });

				Vector<String> entities = serializer.deserializeNamespace("lod/data/entity/");
				Vector<String> validity = serializer.deserializeNamespace("lod/data/valid/");
				Vector<String> base_lods = serializer.deserializeNamespace("lod/data/base_lod/");
				Vector<String> lods = serializer.deserializeNamespace("lod/data/lods/");

				for (uint32_t i = 0; i < entities.size(); ++i)
				{
					Data data((entity::Entity)std::stoul(stlString(entities[i])));
					data.valid = std::stoul(stlString(validity[i])) > 0ul ? true : false;
					data.base_lod.setDistance(std::stof(stlString(base_lods[i])));
					data.base_lod.setMesh(asset::MeshManager::getInstance()->get(base_lods[i]));

					Vector<String> s = split(lods[i], '|');

					for (uint32_t j = 0; j < s.size(); j += 2)
					{
						LOD lod;
						lod.setDistance(std::stof(stlString(s[j])));
						lod.setMesh(asset::MeshManager::getInstance()->get(s[j + 1]));
						data.lods.push_back(lod);
					}

					scene.lod.data.push_back(data);
				}
			}

			void setBaseLOD(const entity::Entity& entity, const LOD& lod, scene::Scene& scene)
			{
				scene.lod.get(entity).base_lod = lod;
			}
			void addLOD(const entity::Entity& entity, const LOD& lod, scene::Scene& scene)
			{
				auto& data = scene.lod.get(entity);
				data.lods.push_back(lod);

				std::sort(data.lods.begin(), data.lods.end(), std::greater<LOD>());
			}
			LOD getBaseLOD(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.lod.get(entity).base_lod;
			}
			Vector<LOD> getLODs(const entity::Entity& entity, scene::Scene& scene)
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

		LODComponent::LODComponent(const entity::Entity& entity, scene::Scene& scene) :
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