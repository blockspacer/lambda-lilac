#include "mesh_render_system.h"
#include "transform_system.h"
#include "entity_system.h"
#include <glm/glm.hpp>
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
#include <platform/scene.h>
#include <interfaces/irenderer.h>
#include <memory/frame_heap.h>

namespace lambda
{
	namespace components
	{
		namespace MeshRenderSystem
		{
			void getMinMax(const glm::vec3& in_min, const glm::vec3& in_max, const glm::mat4x4& matrix, glm::vec3& out_min, glm::vec3& out_max)
			{

				const glm::vec3 corners[] = {
					matrix * glm::vec4(in_min.x, in_min.y, in_min.z, 1.0f),
					matrix * glm::vec4(in_min.x, in_min.y, in_max.z, 1.0f),
					matrix * glm::vec4(in_max.x, in_min.y, in_max.z, 1.0f),
					matrix * glm::vec4(in_max.x, in_min.y, in_min.z, 1.0f),

					matrix * glm::vec4(in_min.x, in_max.y, in_min.z, 1.0f),
					matrix * glm::vec4(in_min.x, in_max.y, in_max.z, 1.0f),
					matrix * glm::vec4(in_max.x, in_max.y, in_max.z, 1.0f),
					matrix * glm::vec4(in_max.x, in_max.y, in_min.z, 1.0f),
				};

				out_min = corners[0];
				out_max = corners[0];

				for (uint32_t i = 0; i < sizeof(corners) / sizeof(corners[0]); ++i)
				{
					out_min = glm::min(corners[i], out_min);
					out_max = glm::max(corners[i], out_max);
				}
			}

			MeshRenderComponent addComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				if (!TransformSystem::hasComponent(entity, scene))
					TransformSystem::addComponent(entity, scene);

				scene.mesh_render.add(entity);

				utilities::Renderable* renderable = foundation::Memory::construct<utilities::Renderable>();
				renderable->entity = entity;
				scene.mesh_render.dynamic_renderables.push_back(renderable);

				return MeshRenderComponent(entity, scene);
			}
			MeshRenderComponent getComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return MeshRenderComponent(entity, scene);
			}
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.has(entity);
			}
			void removeComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.mesh_render.remove(entity);
			}

			void collectGarbage(scene::Scene& scene)
			{
				if (!scene.mesh_render.marked_for_delete.empty())
				{
					for (entity::Entity entity : scene.mesh_render.marked_for_delete)
					{
						const auto& it = scene.mesh_render.entity_to_data.find(entity);
						if (it != scene.mesh_render.entity_to_data.end())
						{
							auto dit = eastl::find_if(
								scene.mesh_render.dynamic_renderables.begin(), scene.mesh_render.dynamic_renderables.end(),
								[&entity](const utilities::Renderable* x) { return x->entity == entity; });
							if (dit != scene.mesh_render.dynamic_renderables.end())
							{
								foundation::Memory::destruct(*dit);
								scene.mesh_render.dynamic_renderables.erase(dit);
							}
							auto sit = eastl::find_if(
								scene.mesh_render.static_renderables.begin(), scene.mesh_render.static_renderables.end(),
								[&entity](const utilities::Renderable* x) { return x->entity == entity; });
							if (sit != scene.mesh_render.static_renderables.end())
							{
								scene.mesh_render.static_bvh->remove(entity);
								foundation::Memory::destruct(*sit);
								scene.mesh_render.static_renderables.erase(sit);
							}

							uint32_t idx = it->second;
							scene.mesh_render.unused_data_entries.push(idx);
							scene.mesh_render.data_to_entity.erase(idx);
							scene.mesh_render.entity_to_data.erase(entity);
							scene.mesh_render.data[idx].valid = false;
						}
					}
					scene.mesh_render.marked_for_delete.clear();
				}
			}

			void initialize(scene::Scene& scene)
			{
				scene.mesh_render.default_albedo = asset::TextureManager::getInstance()->create(
					Name("__default_albedo__"),
					1u, 1u, 1u,
					TextureFormat::kR8G8B8A8,
					0u,
					Vector<unsigned char>{ 255u, 255u, 255u, 255u }
				);

				scene.mesh_render.default_normal = asset::TextureManager::getInstance()->create(
					Name("__default_normal__"),
					1u, 1u, 1u,
					TextureFormat::kR8G8B8A8,
					0u,
					Vector<unsigned char>{ 128u, 128u, 255u, 255u }
				);

				scene.mesh_render.default_dmra = asset::TextureManager::getInstance()->create(
					Name("__default_dmra__"),
					1u, 1u, 1u,
					TextureFormat::kR8G8B8A8,
					0u,
					Vector<unsigned char>{ 255u, 0u, 0u, 255u }
				);

				scene.mesh_render.default_emissive = asset::TextureManager::getInstance()->create(
					Name("__default_emissive__"),
					1u, 1u, 1u,
					TextureFormat::kR8G8B8A8,
					0u,
					Vector<unsigned char>{ 255u, 255u, 255u, 255u }
				);

				scene.mesh_render.static_bvh = foundation::Memory::construct<utilities::BVH>();
				scene.mesh_render.dynamic_bvh = foundation::Memory::construct<utilities::TransientBVH>();
			}
			void deinitialize(scene::Scene& scene)
			{
				Vector<entity::Entity> entities;
				for (const auto& it : scene.mesh_render.entity_to_data)
					entities.push_back(it.first);

				for (const auto& entity : entities)
					scene.mesh_render.remove(entity);
				collectGarbage(scene);

				scene.mesh_render.static_bvh->clear();
				scene.mesh_render.dynamic_bvh->clear();
				foundation::Memory::destruct(scene.mesh_render.static_bvh);
				foundation::Memory::destruct(scene.mesh_render.dynamic_bvh);

				scene.mesh_render.default_albedo   = nullptr;
				scene.mesh_render.default_normal   = nullptr;
				scene.mesh_render.default_dmra     = nullptr;
				scene.mesh_render.default_emissive = nullptr;
			}
			void updateDynamicsBvh(scene::Scene& scene)
			{
				scene.mesh_render.dynamic_bvh->clear();

				for (utilities::Renderable* renderable : scene.mesh_render.dynamic_renderables)
				{
					auto& data = scene.mesh_render.get(renderable->entity);
					
					renderable->mesh             = data.mesh;
					renderable->sub_mesh         = data.sub_mesh;
					renderable->albedo_texture   = data.albedo_texture;
					renderable->normal_texture   = data.normal_texture;
					renderable->dmra_texture     = data.dmra_texture;
					renderable->emissive_texture = data.emissive_texture;
					renderable->metallicness     = data.metallicness;
					renderable->roughness        = data.roughness;
					renderable->emissiveness     = data.emissiveness;
					renderable->model_matrix     = components::TransformSystem::getWorld(data.entity, scene);

					if (data.mesh)
					{
						const asset::SubMesh& sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);
						getMinMax(sub_mesh.min, sub_mesh.max, renderable->model_matrix, renderable->min, renderable->max);
						renderable->center = (renderable->min + renderable->max) * 0.5f;
						renderable->radius = glm::length(renderable->center - renderable->max);

						scene.mesh_render.dynamic_bvh->add(renderable->entity, renderable, utilities::BVHAABB(renderable->min, renderable->max));
					}
				}
			}
			void setMesh(const entity::Entity& entity, asset::VioletMeshHandle mesh, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).mesh = mesh;
			}
			void setSubMesh(const entity::Entity& entity, const uint32_t& sub_mesh, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).sub_mesh = sub_mesh;
			}
			void setAlbedoTexture(const entity::Entity& entity, asset::VioletTextureHandle texture, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).albedo_texture = texture ? texture : scene.mesh_render.default_albedo;
			}
			void setNormalTexture(const entity::Entity& entity, asset::VioletTextureHandle texture, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).normal_texture = texture ? texture : scene.mesh_render.default_normal;
			}
			void setMetallicness(const entity::Entity& entity, const float& metallicness, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).metallicness = metallicness;
			}
			void setRoughness(const entity::Entity& entity, const float& roughness, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).roughness = roughness;
			}
			void setEmissiveness(const entity::Entity& entity, const glm::vec3& emissiveness, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).emissiveness = emissiveness;
			}
			void setDMRATexture(const entity::Entity& entity, asset::VioletTextureHandle texture, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).dmra_texture = texture ? texture : scene.mesh_render.default_dmra;
			}
			void setEmissiveTexture(const entity::Entity& entity, asset::VioletTextureHandle texture, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).emissive_texture = texture ? texture : scene.mesh_render.default_emissive;
			}
			asset::VioletMeshHandle getMesh(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).mesh;
			}
			uint32_t getSubMesh(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).sub_mesh;
			}
			asset::VioletTextureHandle getAlbedoTexture(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).albedo_texture;
			}
			asset::VioletTextureHandle getNormalTexture(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).normal_texture;
			}
			asset::VioletTextureHandle getDMRATexture(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).dmra_texture;
			}
			asset::VioletTextureHandle getEmissiveTexture(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).emissive_texture;
			}
			float getMetallicness(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).metallicness;
			}
			float getRoughness(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).roughness;
			}
			glm::vec3 getEmissiveness(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).emissiveness;
			}
			void attachMesh(const entity::Entity& entity, asset::VioletMeshHandle mesh, scene::Scene& scene)
			{
				// Get all textures.
				Vector<asset::VioletTextureHandle> textures = mesh->getAttachedTextures();
				const glm::uvec3 texture_count = mesh->getAttachedTextureCount();
				size_t alb_offset  = 0u;
				size_t nor_offset  = texture_count.x;
				size_t dmra_offset = nor_offset + texture_count.y;
				size_t emi_offset  = dmra_offset + texture_count.z;

				// Prepare all of the entities.
				Vector<entity::Entity> entities(mesh->getSubMeshes().size());
				Vector<TransformComponent> transforms(mesh->getSubMeshes().size());
				for (size_t i = 0; i < mesh->getSubMeshes().size(); ++i)
				{
					// Create an entity.
					entity::Entity& e = entities.at(i);
					e = scene.entity.create();

					// Create a transform.
					TransformComponent& transform = transforms.at(i);
					transform = TransformSystem::addComponent(e, scene);
					// Set default parent to the mesh renderer component.
					transform.setParent(TransformSystem::getComponent(entity, scene));
				}

				// Attach all of the mesh renderers.
				for (size_t i = 0; i < mesh->getSubMeshes().size(); ++i)
				{
					asset::SubMesh& sub_mesh = mesh->getSubMeshes().at(i);
					TransformComponent transform = transforms.at(i);

					transform.setLocalTranslation(sub_mesh.io.translation);
					transform.setLocalRotation(sub_mesh.io.rotation);
					transform.setLocalScale(sub_mesh.io.scale);

					if ((int)i != sub_mesh.io.parent)
					{
						transform.setParent(transforms.at(sub_mesh.io.parent));
					}

					// Mesh
					if (sub_mesh.offsets[asset::MeshElements::kPositions].count > 0u)
					{
						MeshRenderComponent mesh_render = addComponent(entities.at(i), scene);
						mesh_render.setMesh(mesh);
						mesh_render.setSubMesh((uint32_t)i);
						mesh_render.setMetallicness(sub_mesh.io.metallic);
						mesh_render.setRoughness(sub_mesh.io.roughness);
						mesh_render.setEmissiveness(sub_mesh.io.emissiveness);

						// Textures
						if (sub_mesh.io.tex_alb != -1)
						{
							assert(sub_mesh.offsets[asset::MeshElements::kPositions].count > 0);
							mesh_render.setAlbedoTexture(textures.at(alb_offset + sub_mesh.io.tex_alb));
						}
						if (sub_mesh.io.tex_nor != -1)
						{
							assert(sub_mesh.offsets[asset::MeshElements::kPositions].count > 0);
							mesh_render.setNormalTexture(textures.at(nor_offset + sub_mesh.io.tex_nor));
						}
						if (sub_mesh.io.tex_dmra != -1)
						{
							assert(sub_mesh.offsets[asset::MeshElements::kPositions].count > 0);
							mesh_render.setDMRATexture(textures.at(dmra_offset + sub_mesh.io.tex_dmra));
						}
						if (sub_mesh.io.tex_emi != -1)
						{
							assert(sub_mesh.offsets[asset::MeshElements::kPositions].count > 0);
							mesh_render.setEmissiveTexture(textures.at(emi_offset + sub_mesh.io.tex_emi));
						}
					}
				}
			}
			bool getVisible(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).visible;
			}
			void setVisible(const entity::Entity& entity, const bool& visible, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).visible = visible;
			}
			bool getCastShadows(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.mesh_render.get(entity).cast_shadows;
			}
			void setCastShadows(const entity::Entity& entity, const bool& cast_shadows, scene::Scene& scene)
			{
				scene.mesh_render.get(entity).cast_shadows = cast_shadows;
			}
			void makeStatic(const entity::Entity& entity, scene::Scene& scene)
			{
				for (const auto& data : scene.mesh_render.static_renderables)
					if (data->entity == entity)
						return;

				utilities::Renderable* renderable = nullptr;
				auto it = eastl::find_if(
					scene.mesh_render.dynamic_renderables.begin(), scene.mesh_render.dynamic_renderables.end(),
					[&entity](const utilities::Renderable* x) { return x->entity == entity; });
				if (it != scene.mesh_render.dynamic_renderables.end())
				{
					renderable = *it;
					scene.mesh_render.dynamic_renderables.erase(it);
				}

				LMB_ASSERT(renderable, "MESH RENDER: Could not find dynamic renderable with entity: %llu", entity);

				const Data& data = scene.mesh_render.get(entity);
				
				renderable->entity           = entity;
				renderable->model_matrix     = TransformSystem::getWorld(entity, scene);
				renderable->mesh             = data.mesh;
				renderable->sub_mesh         = data.sub_mesh;
				renderable->albedo_texture   = data.albedo_texture;
				renderable->normal_texture   = data.normal_texture;
				renderable->dmra_texture     = data.dmra_texture;
				renderable->emissive_texture = data.emissive_texture;
				renderable->metallicness     = data.metallicness;
				renderable->roughness        = data.roughness;
				renderable->emissiveness     = data.emissiveness;

				if (renderable->mesh)
				{
					const asset::SubMesh& sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);
					getMinMax(sub_mesh.min, sub_mesh.max, renderable->model_matrix, renderable->min, renderable->max);
					renderable->center = (renderable->min + renderable->max) * 0.5f;
					renderable->radius = glm::length(renderable->center - renderable->max);

					scene.mesh_render.static_bvh->add(entity, renderable, utilities::BVHAABB(renderable->min, renderable->max));
				}

				scene.mesh_render.static_renderables.push_back(renderable);
			}
			void makeDynamic(const entity::Entity& entity, scene::Scene& scene)
			{
				for (utilities::Renderable* renderable : scene.mesh_render.dynamic_renderables)
					if (renderable->entity == entity)
						return;

				utilities::Renderable* renderable = nullptr;

				for (uint32_t i = 0u; i < scene.mesh_render.static_renderables.size(); ++i)
				{
					if (scene.mesh_render.static_renderables.at(i)->entity == entity)
					{
						renderable = scene.mesh_render.static_renderables.at(i);
						scene.mesh_render.static_renderables.erase(scene.mesh_render.static_renderables.begin() + i);
						break;
					}
				}

				LMB_ASSERT(renderable, "MESH RENDER: Could not find static renderable with entity: %llu", entity);

				scene.mesh_render.static_bvh->remove(entity);
				scene.mesh_render.dynamic_renderables.push_back(renderable);
			}

			void createRenderList(utilities::Culler& culler, const utilities::Frustum& frustum, scene::Scene& scene)
			{
				culler.cullStatics(*scene.mesh_render.static_bvh, frustum);
				culler.cullDynamics(*scene.mesh_render.dynamic_bvh, frustum);
			}

			void createSortedRenderList(utilities::LinkedNode* linked_node, Vector<utilities::Renderable*>& opaque, Vector<utilities::Renderable*>& alpha, scene::Scene& scene)
			{
				if (linked_node)
				{
					for (utilities::LinkedNode* node = linked_node->next; node != nullptr; node = node->next)
					{
						utilities::Renderable* renderable = (utilities::Renderable*)node->data;
						// TODO (Hilze): Implement.
						if (renderable->albedo_texture && renderable->albedo_texture->getLayer(0u).containsAlpha())
							alpha.push_back(renderable);
						else
							opaque.push_back(renderable);
					}
				}
			}

			struct DepthSort
			{
				bool larger = false;
				Map<size_t, float> entity_to_depth;
				inline bool operator() (const Data* mesh1, const Data* mesh2)
				{
					if (larger)
						return entity_to_depth.at(mesh1->entity) > entity_to_depth.at(mesh2->entity);
					else
						return entity_to_depth.at(mesh1->entity) < entity_to_depth.at(mesh2->entity);
				}
			};

			void renderAll(utilities::Culler& culler, const utilities::Frustum& frustum, scene::Scene& scene, bool is_rh)
			{
				createRenderList(culler, frustum, scene);

				utilities::LinkedNode statics = culler.getStatics();
				utilities::LinkedNode dynamics = culler.getDynamics();

				renderAll(&statics, &dynamics, scene, is_rh);
			}
			void renderAll(utilities::LinkedNode* statics, utilities::LinkedNode* dynamics, scene::Scene& scene, bool is_rh)
			{
				Vector<utilities::Renderable*> opaque;
				Vector<utilities::Renderable*> alpha;
				createSortedRenderList(statics, opaque, alpha, scene);
				createSortedRenderList(dynamics, opaque, alpha, scene);
				renderAll(opaque, alpha, scene, is_rh);
			}
			void renderAll(const Vector<utilities::Renderable*>& opaque, const Vector<utilities::Renderable*>& alpha, scene::Scene& scene, bool is_rh)
			{
				// -------------------------RENDER-STATIC-OPAQUE-------------------------
				scene.renderer->setBlendState(platform::BlendState::Default());
				for (const utilities::Renderable* renderable : opaque)
				{
					scene.renderer->setMesh(renderable->mesh);
					scene.renderer->setSubMesh(renderable->sub_mesh);

					scene.renderer->setTexture(renderable->albedo_texture, 0);
					scene.renderer->setTexture(renderable->normal_texture, 1);
					scene.renderer->setTexture(renderable->dmra_texture, 2);

					//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("metallic_roughness"), glm::vec2(renderable->metallicness, renderable->roughness)));
					//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("model_matrix"), renderable->model_matrix));

					auto sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);
					// TODO (Hilze): Implement.
					if (sub_mesh.io.double_sided == true || (sub_mesh.io.tex_alb >= 0 && renderable->mesh->getAttachedTextures().at(sub_mesh.io.tex_alb)->getLayer(0u).containsAlpha()))
					{
						scene.renderer->setRasterizerState(platform::RasterizerState::SolidNone());
					}
					else
					{
						if (is_rh)
							scene.renderer->setRasterizerState(platform::RasterizerState::SolidFront());
						else
							scene.renderer->setRasterizerState(platform::RasterizerState::SolidBack());
					}

					scene.renderer->draw();
				}
				// -------------------------RENDER-STATIC-ALPHA-------------------------
				scene.renderer->setBlendState(platform::BlendState::Alpha());
				for (const utilities::Renderable* renderable : alpha)
				{
					scene.renderer->setMesh(renderable->mesh);
					//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("metallic_roughness"), glm::vec2(renderable->metallicness, renderable->roughness)));
					scene.renderer->setSubMesh(renderable->sub_mesh);
					scene.renderer->setTexture(renderable->albedo_texture, 0);
					scene.renderer->setTexture(renderable->normal_texture, 1);
					scene.renderer->setTexture(renderable->dmra_texture, 2);
					//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("model_matrix"), renderable->model_matrix));

					auto sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);
					// TODO (Hilze): Implement.
					if (sub_mesh.io.double_sided == true || (sub_mesh.io.tex_alb >= 0 && renderable->mesh->getAttachedTextures().at(sub_mesh.io.tex_alb)->getLayer(0u).containsAlpha()))
						scene.renderer->setRasterizerState(platform::RasterizerState::SolidNone());
					else
						scene.renderer->setRasterizerState(platform::RasterizerState::SolidFront());

					scene.renderer->draw();
				}
			}
		}

		// The system data.
		namespace MeshRenderSystem
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

				Data& d = data[idx];

				d.albedo_texture   = default_albedo;
				d.normal_texture   = default_normal;
				d.dmra_texture     = default_dmra;
				d.emissive_texture = default_emissive;

				return d;
			}

			Data& SystemData::get(const entity::Entity& entity)
			{
				auto it = entity_to_data.find(entity);
				LMB_ASSERT(it != entity_to_data.end(), "MESHRENDER: %llu does not have a component", entity);
				LMB_ASSERT(data[it->second].valid, "MESHRENDER: %llu's data was not valid", entity);
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

		// The mesh render data.
		namespace MeshRenderSystem
		{
			Data::Data(const Data& other)
			{
				mesh = other.mesh;
				sub_mesh = other.sub_mesh;
				albedo_texture = other.albedo_texture;
				normal_texture = other.normal_texture;
				dmra_texture = other.dmra_texture;
				emissive_texture = other.emissive_texture;
				metallicness = other.metallicness;
				roughness = other.roughness;
				emissiveness = other.emissiveness;
				visible = other.visible;
				cast_shadows = other.cast_shadows;
				entity = other.entity;
				valid = other.valid;
			}
			Data& Data::operator=(const Data& other)
			{
				mesh = other.mesh;
				sub_mesh = other.sub_mesh;
				albedo_texture = other.albedo_texture;
				normal_texture = other.normal_texture;
				dmra_texture = other.dmra_texture;
				emissive_texture = other.emissive_texture;
				metallicness = other.metallicness;
				roughness = other.roughness;
				emissiveness = other.emissiveness;
				visible = other.visible;
				cast_shadows = other.cast_shadows;
				entity = other.entity;
				valid = other.valid;

				return *this;
			}
		}

		MeshRenderComponent::MeshRenderComponent(const entity::Entity& entity, scene::Scene& scene) :
			IComponent(entity), scene_(&scene)
		{
		}
		MeshRenderComponent::MeshRenderComponent(const MeshRenderComponent& other) :
			IComponent(other.entity_), scene_(other.scene_)
		{
		}
		MeshRenderComponent::MeshRenderComponent() :
			IComponent(entity::Entity()), scene_(nullptr)
		{
		}
		void MeshRenderComponent::setMesh(asset::VioletMeshHandle mesh)
		{
			MeshRenderSystem::setMesh(entity_, mesh, *scene_);
		}
		asset::VioletMeshHandle MeshRenderComponent::getMesh() const
		{
			return MeshRenderSystem::getMesh(entity_, *scene_);
		}
		void MeshRenderComponent::setSubMesh(const uint32_t& sub_mesh)
		{
			MeshRenderSystem::setSubMesh(entity_, sub_mesh, *scene_);
		}
		uint32_t MeshRenderComponent::getSubMesh() const
		{
			return MeshRenderSystem::getSubMesh(entity_, *scene_);
		}
		void MeshRenderComponent::setMetallicness(const float& metallicness)
		{
			MeshRenderSystem::setMetallicness(entity_, metallicness, *scene_);
		}
		float MeshRenderComponent::getMetallicness() const
		{
			return MeshRenderSystem::getMetallicness(entity_, *scene_);
		}
		void MeshRenderComponent::setRoughness(const float& roughness)
		{
			MeshRenderSystem::setRoughness(entity_, roughness, *scene_);
		}
		float MeshRenderComponent::getRoughness() const
		{
			return MeshRenderSystem::getRoughness(entity_, *scene_);
		}
		asset::VioletTextureHandle MeshRenderComponent::getAlbedoTexture() const
		{
			return MeshRenderSystem::getAlbedoTexture(entity_, *scene_);
		}
		glm::vec3 MeshRenderComponent::getEmissiveness() const
		{
			return MeshRenderSystem::getEmissiveness(entity_, *scene_);
		}
		void MeshRenderComponent::setEmissiveness(const glm::vec3& emissiveness)
		{
			MeshRenderSystem::setEmissiveness(entity_, emissiveness, *scene_);
		}
		void MeshRenderComponent::setAlbedoTexture(asset::VioletTextureHandle texture)
		{
			MeshRenderSystem::setAlbedoTexture(entity_, texture, *scene_);
		}
		void MeshRenderComponent::setNormalTexture(asset::VioletTextureHandle texture)
		{
			MeshRenderSystem::setNormalTexture(entity_, texture, *scene_);
		}
		asset::VioletTextureHandle MeshRenderComponent::getNormalTexture() const
		{
			return MeshRenderSystem::getNormalTexture(entity_, *scene_);
		}
		void MeshRenderComponent::setDMRATexture(asset::VioletTextureHandle texture)
		{
			MeshRenderSystem::setDMRATexture(entity_, texture, *scene_);
		}
		asset::VioletTextureHandle MeshRenderComponent::getDMRATexture() const
		{
			return MeshRenderSystem::getDMRATexture(entity_, *scene_);
		}
		void MeshRenderComponent::setEmissiveTexture(asset::VioletTextureHandle texture)
		{
			MeshRenderSystem::setEmissiveTexture(entity_, texture, *scene_);
		}
		asset::VioletTextureHandle MeshRenderComponent::getEmissiveTexture() const
		{
			return MeshRenderSystem::getEmissiveTexture(entity_, *scene_);
		}
		void MeshRenderComponent::attachMesh(asset::VioletMeshHandle mesh)
		{
			MeshRenderSystem::attachMesh(entity_, mesh, *scene_);
		}
		bool MeshRenderComponent::getVisible() const
		{
			return MeshRenderSystem::getVisible(entity_, *scene_);
		}
		void MeshRenderComponent::setVisible(const bool& visible)
		{
			MeshRenderSystem::setVisible(entity_, visible, *scene_);
		}
		bool MeshRenderComponent::getCastShadows() const
		{
			return MeshRenderSystem::getCastShadows(entity_, *scene_);
		}
		void MeshRenderComponent::setCastShadows(const bool& cast_shadows)
		{
			MeshRenderSystem::setCastShadows(entity_, cast_shadows, *scene_);
		}
	}
}