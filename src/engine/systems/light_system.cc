#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "light_system.h"
#include "transform_system.h"
#include "mesh_render_system.h"
#include "camera_system.h"
#include "assets/shader_io.h"
#include "platform/post_process_manager.h"
#include "platform/rasterizer_state.h"
#include "platform/blend_state.h"
#include "platform/depth_stencil_state.h"
#include "platform/sampler_state.h"
#include <glm/gtc/matrix_transform.hpp>
#include "platform/debug_renderer.h"
#include <algorithm>
#include "interfaces/irenderer.h"
#include <platform/scene.h>

namespace lambda
{
  namespace components
  {
		namespace LightSystem
		{
			LightComponent addComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				if (!TransformSystem::hasComponent(entity, scene))
					TransformSystem::addComponent(entity, scene);

				scene.light.add(entity);

				auto& data = scene.light.get(entity);
				data.type = LightType::kUnknown;
				data.culler.push_back(utilities::Culler());
				data.culler.back().setCullFrequency(3u);
				data.culler.back().setShouldCull(true);
				data.depth.push_back(100.0f);
				data.view.resize(1u);
				data.projection.resize(1u);
				data.view_position.resize(1u);
				//data.culler.setCullShadowCasters(true);
				data.texture = scene.light.default_texture;

				return LightComponent(entity, scene);
			}
			LightComponent getComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return LightComponent(entity, scene);
			}
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.has(entity);
			}
			void removeComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.light.remove(entity);
			}
			void collectGarbage(scene::Scene& scene)
			{
				if (!scene.light.marked_for_delete.empty())
				{
					for (entity::Entity entity : scene.light.marked_for_delete)
					{
						const auto& it = scene.light.entity_to_data.find(entity);
						if (it != scene.light.entity_to_data.end())
						{
							uint32_t idx = it->second;
							scene.light.unused_data_entries.push(idx);
							scene.light.data_to_entity.erase(idx);
							scene.light.entity_to_data.erase(entity);
							scene.light.data[idx].valid = false;
						}
					}
					scene.light.marked_for_delete.clear();
				}
			}
			LightComponent addDirectionalLight(const entity::Entity& entity, scene::Scene& scene)
			{
				addComponent(entity, scene);

				scene.light.get(entity).type = LightType::kDirectional;
				return LightComponent(entity, scene);
			}
			LightComponent addPointLight(const entity::Entity& entity, scene::Scene& scene)
			{
				addComponent(entity, scene);

				scene.light.get(entity).type = LightType::kPoint;
				return LightComponent(entity, scene);
			}
			LightComponent addSpotLight(const entity::Entity& entity, scene::Scene& scene)
			{
				addComponent(entity, scene);

				scene.light.get(entity).type = LightType::kSpot;
				return LightComponent(entity, scene);
			}
			LightComponent addCascadedLight(const entity::Entity& entity, scene::Scene& scene)
			{
				addComponent(entity, scene);

				scene.light.get(entity).type = LightType::kCascade;
				return LightComponent(entity, scene);
			}
		void setShaders(String generate, String modify, uint32_t modify_count, String publish, String shadow_type, scene::Scene& scene)
		{
			scene.light.shader_generate     = generate;
			scene.light.shader_modify       = modify;
			scene.light.shader_modify_count = modify_count;
			scene.light.shader_publish      = publish;
			scene.light.shader_shadow_type  = shadow_type;
		}
		void initialize(scene::Scene& scene)
			{
			scene.post_process_manager->addTarget(platform::RenderTarget(Name("light_map"),
					1.0f, TextureFormat::kR16G16B16A16
				));

			scene.light.full_screen_mesh = asset::MeshManager::getInstance()->create(Name("__fs_quad__"), asset::Mesh::createScreenQuad());

			Vector<char> data(4);
			float clear_value[2] = { FLT_MAX, FLT_MAX };
			memcpy(data.data(), clear_value, sizeof(float) * 2);

			scene.light.default_shadow_map = platform::RenderTarget(Name("__scene.light.default_shadow_map_"),
					asset::TextureManager::getInstance()->create(
						Name("__scene.light.default_shadow_map_"),
						1u, 1u, 6u, TextureFormat::kR32G32, 0
					)
				);
      
			scene.light.default_texture = asset::TextureManager::getInstance()->create(
					Name("__default_light_texture__"),
					1u, 1u, 1u, TextureFormat::kR8G8B8A8,
					0,
					Vector<char>(4, 255)
				);
			}
		void deinitialize(scene::Scene& scene)
		{
			Vector<entity::Entity> entities;
			for (const auto& it : scene.light.entity_to_data)
				entities.push_back(it.first);

			for (const auto& entity : entities)
				removeComponent(entity, scene);
			collectGarbage(scene);
		}

			void onRender(scene::Scene& scene)
			{
				LMB_ASSERT(false, "");
				scene.renderer->beginTimer("Lighting");
      
				// Prepare the light buffer.
				scene.renderer->pushMarker("Clear Light Buffer");
				scene.renderer->clearRenderTarget(
					scene.post_process_manager->getTarget(Name("light_map")).getTexture(),
					glm::vec4(0.0f)
				);
				scene.renderer->popMarker();


				auto main_camera = CameraSystem::getMainCamera(scene);
				TransformComponent transform = TransformSystem::getComponent(main_camera, scene);
				const glm::mat4x4 view = glm::inverse(transform.getWorld());
				const glm::mat4x4 projection = glm::perspective(
					CameraSystem::getFov(main_camera, scene).asRad(),
					(float)scene.window->getSize().x / (float)scene.window->getSize().y,
					CameraSystem::getNearPlane(main_camera, scene).asMeter(),
					CameraSystem::getFarPlane(main_camera, scene).asMeter()
				);

				utilities::Frustum frustum;
				frustum.construct(projection, view);

				for (Data& data : scene.light.data)
				{
					bool enabled = data.enabled;

					if (enabled)
					{
						switch (data.type)
						{
						case LightType::kPoint:
						{
							glm::vec3 position = TransformSystem::getWorldTranslation(data.entity, scene);
							float radius = data.depth.back();
							if (!frustum.ContainsSphere(position, radius))
								enabled = false;
						}
						}
					}

					// Skip disabled lights.
					if (!enabled)
					{
						switch (data.type)
						{
						case LightType::kDirectional: scene.renderer->setMarker("Directional Light - Disabled"); break;
						case LightType::kSpot:        scene.renderer->setMarker("Spot Light - Disabled"); break;
						case LightType::kPoint:       scene.renderer->setMarker("Point Light - Disabled"); break;
						case LightType::kCascade:     scene.renderer->setMarker("Cascade Light - Disabled"); break;
						default:
							break;
						}
						continue;
					}

					switch (data.type)
					{
					case LightType::kDirectional:
						{
							scene.renderer->pushMarker("Directional Light");
							renderDirectional(data.entity, scene);
							scene.renderer->popMarker();
						}
						break;
					case LightType::kSpot:
						{
							scene.renderer->pushMarker("Spot Light");
							renderSpot(data.entity, scene);
				scene.renderer->popMarker();
						}
						break;
					case LightType::kPoint:
						{
							scene.renderer->pushMarker("Point Light");
							renderPoint(data.entity, scene);
				scene.renderer->popMarker();
						}
						break;
					case LightType::kCascade:
						{
							scene.renderer->pushMarker("Cascade Light");
							renderCascade(data.entity, scene);
				scene.renderer->popMarker();
						}
						break;
					default:
						break;
					}
				}

				// Reset states.
				scene.renderer->setRasterizerState(platform::RasterizerState::SolidFront());
			CameraSystem::bindCamera(CameraSystem::getMainCamera(scene), scene);

				scene.renderer->endTimer("Lighting");
			}
			void setColour(const entity::Entity& entity, const glm::vec3& colour, scene::Scene& scene)
			{
				scene.light.get(entity).colour = colour;
			}
			glm::vec3 getColour(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).colour;
			}
			void setAmbient(const entity::Entity& entity, const glm::vec3& ambient, scene::Scene& scene)
			{
				scene.light.get(entity).ambient = ambient;
			}
			glm::vec3 getAmbient(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).ambient;
			}
			void setIntensity(const entity::Entity& entity, const float& intensity, scene::Scene& scene)
			{
				scene.light.get(entity).intensity = intensity;
			}
			float getIntensity(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).intensity;
			}
			void setShadowType(const entity::Entity& entity, const ShadowType& shadow_type, scene::Scene& scene)
			{
				scene.light.get(entity).shadow_type = shadow_type;
				createShadowMaps(entity, scene);
			}
			ShadowType getShadowType(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).shadow_type;
			}
			void setShadowMapSizePx(const entity::Entity & entity, uint32_t shadow_map_size_px, scene::Scene& scene)
			{
				scene.light.get(entity).shadow_map_size_px = shadow_map_size_px;
				createShadowMaps(entity, scene);
			}
			uint32_t getShadowMapSizePx(const entity::Entity & entity, scene::Scene& scene)
			{
				return scene.light.get(entity).shadow_map_size_px;
			}
			void setCutOff(const entity::Entity& entity, const utilities::Angle& cut_off, scene::Scene& scene)
			{
				scene.light.get(entity).cut_off = cut_off;
			}
			utilities::Angle getCutOff(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).cut_off;
			}
			void setOuterCutOff(const entity::Entity& entity, const utilities::Angle& outer_cut_off, scene::Scene& scene)
			{
				scene.light.get(entity).outer_cut_off = outer_cut_off;
			}
			utilities::Angle getOuterCutOff(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).outer_cut_off;
			}
			void setDepth(const entity::Entity& entity, const float& depth, scene::Scene& scene)
			{
				for (float& d : scene.light.get(entity).depth)
					d = depth;
			}
			float getDepth(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).depth.back();
			}
			void setSize(const entity::Entity& entity, const float& size, scene::Scene& scene)
			{
				scene.light.get(entity).size = size;
			}
			float getSize(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).size;
			}
			void setTexture(const entity::Entity& entity, asset::VioletTextureHandle texture, scene::Scene& scene)
			{
				scene.light.get(entity).texture = texture;
			}
			asset::VioletTextureHandle getTexture(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).texture;
			}
			void setEnabled(const entity::Entity& entity, const bool& enabled, scene::Scene& scene)
			{
				scene.light.get(entity).enabled = enabled;
			}
			bool getEnabled(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).enabled;
			}
			void setDynamicFrequency(const entity::Entity& entity, const uint8_t& frequency, scene::Scene& scene)
			{
				scene.light.get(entity).dynamic_frequency = frequency;
			}
			uint8_t getDynamicFrequency(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).dynamic_frequency;
			}
			void setRSM(const entity::Entity& entity, bool rsm, scene::Scene& scene)
			{
				scene.light.get(entity).rsm = rsm;
			createShadowMaps(entity, scene);
			}
			bool getRSM(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).rsm;
			}
			void setLightType(const entity::Entity& entity, LightType type, scene::Scene& scene)
			{
				scene.light.get(entity).type = type;
			createShadowMaps(entity, scene);
		}
			LightType getLightType(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.light.get(entity).type;
			}
			void createShadowMaps(const entity::Entity& entity, scene::Scene& scene)
			{
				Data& data = scene.light.get(entity);

				uint32_t size = 1u; // TODO (Hilze): Do RSM stuff here.
				uint32_t layers = (data.type == LightType::kPoint) ? 6u : 1u;

				data.depth_target.resize(size * layers);
				data.render_target.resize(size * layers);
				data.depth_target_texture.resize(size * layers);
				data.render_target_texture.resize(size * layers);
				data.culler.resize(size * layers);
				data.view_position.resize(size * layers, glm::vec3(0.0f));
				data.projection.resize(size * layers, glm::mat4x4(1.0f));
				data.view.resize(size * layers, glm::mat4x4(1.0f));
				data.depth.resize(size * layers, data.depth.back());

				for (uint32_t i = 0u; i < size; ++i)
				{
					static int kIdx = 0;
					Name depth_name = Name("shadow_map_depth_" + toString(kIdx));
					Name color_name = Name("shadow_map_color_" + toString(kIdx++));

					// Create the textures.
					data.render_target_texture[i] = asset::TextureManager::getInstance()->create(
						color_name,
						data.shadow_map_size_px,
						data.shadow_map_size_px,
						layers,
						TextureFormat::kR32G32,
						kTextureFlagIsRenderTarget
					);
					data.render_target_texture[i]->setKeepInMemory(true);
					data.depth_target_texture[i] = asset::TextureManager::getInstance()->create(
						depth_name,
						data.shadow_map_size_px,
						data.shadow_map_size_px,
						layers,
						TextureFormat::kD32,
						kTextureFlagIsRenderTarget
					);
					// Creat the render targets.
					data.render_target[i] = platform::RenderTarget(color_name, data.render_target_texture[i]);
					data.depth_target[i]  = platform::RenderTarget(depth_name, data.depth_target_texture[i]);

					// Clear the render target.
					//scene.renderer->clearRenderTarget(data.render_target_texture[i], glm::vec4(FLT_MAX));
				}
			}
			void renderDirectional(const entity::Entity& entity, scene::Scene& scene)
			{
				Data& data = scene.light.get(entity);

				const TransformComponent transform = TransformSystem::getComponent(entity, scene);
				const glm::vec3 forward = glm::normalize(transform.getWorldForward());

				const bool update = (data.shadow_type == ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != ShadowType::kGenerated);

				if (data.dynamic_index >= data.dynamic_frequency)
				{
					data.dynamic_index = 0u;
				}

				Vector<platform::RenderTarget> shadow_maps(data.render_target.begin(), data.render_target.end());
				if (shadow_maps.empty()) shadow_maps.push_back(scene.light.default_shadow_map);

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

				/*scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_view_projection_matrix"), data.projection.back() * data.view.back()));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_camera_position"), data.view_position.back()));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_direction"), -forward));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_colour"), data.colour * data.intensity));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_ambient"), data.ambient));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_far"), data.depth.back()));*/

			String shadow_type = data.shadow_type == ShadowType::kNone ? "|NO_" : ("|" + scene.light.shader_shadow_type + "_");
			String shader_type = shadow_type + "DIRECTIONAL";

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
						scene.renderer->clearRenderTarget(shadow_map.getTexture(), glm::vec4(0.0f));
					}
					const platform::RenderTarget depth_map = data.depth_target.at(0u);
					scene.renderer->clearRenderTarget(depth_map.getTexture(), glm::vec4(1.0f));

					Vector<platform::RenderTarget> output = shadow_maps;
					output.push_back(depth_map);

					// Render to the shadow map.
					scene.renderer->bindShaderPass(
						platform::ShaderPass(
							Name("shadow_generate"),
				asset::ShaderManager::getInstance()->get(scene.light.shader_generate + shader_type),
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
					MeshRenderSystem::renderAll(data.culler.back(), frustum, scene);

					// Set up the post processing passes.
					scene.renderer->setMesh(scene.light.full_screen_mesh);
					scene.renderer->setSubMesh(0u);
					scene.renderer->setRasterizerState(platform::RasterizerState::SolidBack());
					scene.renderer->setBlendState(platform::BlendState::Alpha());

					// Draw all modify shaders.
			for (uint32_t i = 0; i < scene.light.shader_modify_count; ++i)
					{
						scene.renderer->bindShaderPass(
							platform::ShaderPass(
								Name("shadow_modify"),
					asset::ShaderManager::getInstance()->get(scene.light.shader_modify + shadow_type + "HORIZONTAL"),
								{ shadow_maps.at(0u) },
								{ shadow_maps.at(0u) }
							)
						);
						scene.renderer->draw();
						scene.renderer->bindShaderPass(
							platform::ShaderPass(
								Name("shadow_modify"),
					asset::ShaderManager::getInstance()->get(scene.light.shader_modify + shadow_type + "VERTICAL"),
								{ shadow_maps.at(0u) },
								{ shadow_maps.at(0u) }
							)
						);
						scene.renderer->draw();
					}
				}

				// Render lights to the light map.
				// Set up the post processing passes.
				scene.renderer->setMesh(scene.light.full_screen_mesh);
				scene.renderer->setSubMesh(0u);
				scene.renderer->setRasterizerState(platform::RasterizerState::SolidBack());

				// Render light using the shadow map.
				Vector<platform::RenderTarget> input = {
					shadow_maps.at(0u),
					platform::RenderTarget(Name("texture"), data.texture),
					scene.post_process_manager->getTarget(Name("position")),
					scene.post_process_manager->getTarget(Name("normal")),
					scene.post_process_manager->getTarget(Name("metallic_roughness"))
				};
				if (shadow_maps.size() > 1u)
				input.insert(input.end(), shadow_maps.begin() + 1u, shadow_maps.end());
				scene.renderer->bindShaderPass(
					platform::ShaderPass(
						Name("shadow_publish"),
					asset::ShaderManager::getInstance()->get(scene.light.shader_publish + shader_type),
						input, {
							scene.post_process_manager->getTarget(Name("light_map"))
						}
					)
				);
      
				scene.renderer->setBlendState(platform::BlendState(
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
				scene.renderer->draw();
				scene.renderer->setBlendState(platform::BlendState::Alpha());
			}

			void renderSpot(const entity::Entity& entity, scene::Scene& scene)
			{
				Data& data = scene.light.get(entity);

				const TransformComponent transform = TransformSystem::getComponent(entity, scene);
				const glm::vec3 forward = glm::normalize(transform.getWorldForward());

				Vector<platform::RenderTarget> shadow_maps(data.render_target.begin(), data.render_target.end());
				if (shadow_maps.empty()) shadow_maps.push_back(scene.light.default_shadow_map);
				const bool update = (data.shadow_type == ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != ShadowType::kGenerated);

				if (data.dynamic_index >= data.dynamic_frequency)
					data.dynamic_index = 0u;

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

					data.view.back()          = glm::lookAtRH(translation, translation + forward, glm::vec3(0.0f, 1.0f, 0.0f));
					data.view_position.back() = translation;
					data.projection.back()    = glm::perspectiveRH(data.outer_cut_off.asRad(), 1.0f, 0.001f, data.depth.back());
				}

				//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_view_projection_matrix"), data.projection.back() * data.view.back()));
				//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_camera_position"), data.view_position.back()));
				//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_direction"), forward));
				//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_colour"), data.colour * data.intensity));
				//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_ambient"), data.ambient));
				//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_far"), data.depth.back()));
				//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_position"), transform.getWorldTranslation()));

				//// Required by: spot.
				//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_cut_off"), data.cut_off.asRad()));
				//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_outer_cut_off"), data.outer_cut_off.asRad()));

			String shadow_type = data.shadow_type == ShadowType::kNone ? "|NO_" : ("|" + scene.light.shader_shadow_type + "_");
			String shader_type = shadow_type + "SPOT";

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
						scene.renderer->clearRenderTarget(shadow_map.getTexture(), glm::vec4(0.0f));
					}
					const platform::RenderTarget depth_map = data.depth_target.at(0u);
					scene.renderer->clearRenderTarget(depth_map.getTexture(), glm::vec4(1.0f));

					Vector<platform::RenderTarget> output = shadow_maps;
					output.push_back(depth_map);

					// Render to the shadow map.
					scene.renderer->bindShaderPass(
						platform::ShaderPass(
							Name("shadow_generate"),
							asset::ShaderManager::getInstance()->get(scene.light.shader_generate + shader_type),
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
			MeshRenderSystem::renderAll(data.culler.back(), frustum, scene);

					// Set up the post processing passes.
					scene.renderer->setMesh(scene.light.full_screen_mesh);
					scene.renderer->setSubMesh(0u);
					scene.renderer->setRasterizerState(platform::RasterizerState::SolidBack());
					scene.renderer->setBlendState(platform::BlendState::Alpha());

			for (uint32_t i = 0; i < scene.light.shader_modify_count; ++i)
			{
						scene.renderer->bindShaderPass(
							platform::ShaderPass(
								Name("shadow_modify"),
								asset::ShaderManager::getInstance()->get(scene.light.shader_modify + shader_type + "VERTICAL"),
								{ shadow_maps.at(0u) },
								{ shadow_maps.at(0u) }
							)
						);
						scene.renderer->draw();
						scene.renderer->bindShaderPass(
							platform::ShaderPass(
								Name("shadow_modify"),
								asset::ShaderManager::getInstance()->get(scene.light.shader_modify + shader_type + "HORIZONTAL"),
								{ shadow_maps.at(0u) },
								{ shadow_maps.at(0u) }
							)
						);
						scene.renderer->draw();
					}
				}

				// Render lights to the light map.
				// Set up the post processing passes.
				scene.renderer->setMesh(scene.light.full_screen_mesh);
				scene.renderer->setSubMesh(0u);
				scene.renderer->setRasterizerState(platform::RasterizerState::SolidBack());

				// Render light using the shadow map.
				Vector<platform::RenderTarget> input = {
					shadow_maps.at(0u),
					platform::RenderTarget(Name("texture"), data.texture),
					scene.post_process_manager->getTarget(Name("position")),
					scene.post_process_manager->getTarget(Name("normal")),
					scene.post_process_manager->getTarget(Name("metallic_roughness"))
				};
				if (shadow_maps.size() > 1u) input.insert(input.end(), shadow_maps.begin() + 1u, shadow_maps.end());
				scene.renderer->bindShaderPass(
					platform::ShaderPass(
						Name("shadow_publish"),
						asset::ShaderManager::getInstance()->get(scene.light.shader_publish + shader_type),
						input, {
							scene.post_process_manager->getTarget(Name("light_map"))
						}
						)
				);
                
				scene.renderer->setBlendState(platform::BlendState(
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
				scene.renderer->draw();
				scene.renderer->setBlendState(platform::BlendState::Alpha());
			}

			void renderPoint(const entity::Entity& entity, scene::Scene& scene)
			{
				Data& data = scene.light.get(entity);
      
				const TransformComponent transform = TransformSystem::getComponent(entity, scene);

				static const glm::vec3 g_forwards[6u] = {
					glm::vec3( 1.0f, 0.0f, 0.0f),
					glm::vec3(-1.0f, 0.0f, 0.0f),
					glm::vec3(0.0f,  1.0f, 0.0f),
					glm::vec3(0.0f, -1.0f, 0.0f),
					glm::vec3(0.0f, 0.0f,  1.0f),
					glm::vec3(0.0f, 0.0f, -1.0f),
				};

				static const glm::vec3 g_ups[6u] = {
					glm::vec3(0.0f, 1.0f,  0.0f),
					glm::vec3(0.0f, 1.0f,  0.0f),
					glm::vec3(0.0f, 0.0f,  1.0f),
					glm::vec3(0.0f, 0.0f, -1.0f),
					glm::vec3(0.0f, 1.0f,  0.0f),
					glm::vec3(0.0f, 1.0f,  0.0f),
				};

				platform::RenderTarget shadow_map = (data.render_target.empty()) ? scene.light.default_shadow_map : data.render_target.at(0u);
				const bool update = (data.shadow_type == ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != ShadowType::kGenerated);

				if (data.dynamic_index >= data.dynamic_frequency)
					data.dynamic_index = 0u;

				// Setup camera.
				if (update)
				{
					for (uint32_t i = 0; i < 6; ++i)
					{
						// Set everything up.
						glm::vec3 translation = transform.getWorldTranslation();

						auto view_size = data.view.size();
						auto view_position_size = data.view_position.size();
						auto projection_size = data.projection.size();

						data.view[i]          = glm::lookAtLH(glm::vec3(0.0f), g_forwards[i], g_ups[i]), glm::vec3(-1.0f, 1.0f, 1.0f);
						if (i == 2 || i == 3)   data.view[i] = glm::rotate(data.view[i], 1.5708f * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
						data.view[i]          = glm::translate(data.view[i], -translation);
						data.view_position[i] = translation;
						data.projection[i]    = glm::perspectiveLH(1.5708f, 1.0f, 0.001f, data.depth[i]); // 1.5708f radians == 90 degrees.
					}
				}

				glm::vec3 pos = data.view_position.back();
				/*scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_camera_position"), pos));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_far"),        data.depth.back()));*/
	  
				String shadow_type = data.shadow_type == ShadowType::kNone ? "|NO_" : ("|" + scene.light.shader_shadow_type + "_");
					String shader_type = shadow_type + "POINT";

				// Generate shadow maps.
				if (update && data.shadow_type != ShadowType::kNone)
				{
					bool statics_only = (data.shadow_type == ShadowType::kGenerateOnce);

					if (data.shadow_type == ShadowType::kGenerateOnce)
						data.shadow_type = ShadowType::kGenerated;

					// Clear the shadow map.
					platform::RenderTarget depth_map = data.depth_target.at(0u);

					scene.renderer->clearRenderTarget(shadow_map.getTexture(), glm::vec4(FLT_MAX));
					scene.renderer->clearRenderTarget(depth_map.getTexture(), glm::vec4(1.0f));

					for (uint32_t i = 0; i < 6; ++i)
					{
						scene.renderer->pushMarker("Render Face " + toString(i));

						//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_view_projection_matrix"), data.projection[i] * data.view[i]));
          
						// Render to the shadow map.
						shadow_map.setLayer(i);
						depth_map.setLayer(i);
						scene.renderer->bindShaderPass(
							platform::ShaderPass(
								Name("shadow_generate"),
															asset::ShaderManager::getInstance()->get(scene.light.shader_generate + shader_type),
								{},
								{ shadow_map, depth_map }
							)
						);

						// Handle frustum culling.
						if (data.shadow_type == ShadowType::kDynamic)
							data.culler[i].setCullFrequency(std::max(1u, 10u / data.dynamic_frequency));
						else
							data.culler[i].setCullFrequency(1u);
						utilities::Frustum frustum;
						frustum.construct(data.projection[i], data.view[i]);

						// Render to the shadow map.
						if (statics_only)
						{
							MeshRenderSystem::createRenderList(data.culler.back(), frustum, scene);
							utilities::LinkedNode statics = data.culler.back().getStatics();
							utilities::LinkedNode dynamics;
							MeshRenderSystem::renderAll(&statics, &dynamics, scene, false);
						}
						else
							MeshRenderSystem::renderAll(data.culler.back(), frustum, scene, false);


						// Set up the post processing passes.
						scene.renderer->setMesh(scene.light.full_screen_mesh);
						scene.renderer->setSubMesh(0u);
						scene.renderer->setBlendState(platform::BlendState::Default());

						// TODO (Hilze): Support the blurring of cubemaps.
						/*for (int l = 0; l < 6; ++l)
						{
							shadow_map.setLayer(l);
							scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("face"), (float)l));
							for (uint32_t i = 0; i < scene.light.shader_modify_count; ++i)
							{
								scene.renderer->bindShaderPass(
									platform::ShaderPass(
										Name("shadow_modify"),
										asset::ShaderManager::getInstance()->get(scene.light.shader_modify + shadow_type + "HORIZONTAL_CUBE"),
										{ shadow_map },
										{ shadow_map }
									)
								);
								scene.renderer->draw();
								scene.renderer->bindShaderPass(
									platform::ShaderPass(
										Name("shadow_modify"),
										asset::ShaderManager::getInstance()->get(scene.light.shader_modify + shadow_type + "VERTICAL_CUBE"),
										{ shadow_map },
										{ shadow_map }
									)
								);
								scene.renderer->draw();

							}
						}*/

						scene.renderer->popMarker();
					}
		
					shadow_map.setLayer(-1);
					depth_map.setLayer(-1);
				}

				// Render lights to the light map.
				// Set up the post processing passes.
				scene.renderer->setMesh(scene.light.full_screen_mesh);
				scene.renderer->setSubMesh(0u);
				scene.renderer->setRasterizerState(platform::RasterizerState::SolidBack());
				scene.renderer->setBlendState(platform::BlendState::Alpha());

				// Render light using the shadow map.
				scene.renderer->bindShaderPass(
				platform::ShaderPass(
					Name("shadow_publish"),
									asset::ShaderManager::getInstance()->get(scene.light.shader_publish + shader_type),
					{
						shadow_map,
						platform::RenderTarget(Name("texture"), data.texture),
						scene.post_process_manager->getTarget(Name("position")),
						scene.post_process_manager->getTarget(Name("normal")),
						scene.post_process_manager->getTarget(Name("metallic_roughness"))
					}, {
						scene.post_process_manager->getTarget(Name("light_map"))
					}
					)
				);

				// Required by: dir, point and spot.
				/*scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_direction"),  glm::vec3(0.0f)));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_colour"),     data.colour * data.intensity));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_ambient"),    data.ambient));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_far"),        data.depth.back()));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_position"),   transform.getWorldTranslation()));*/
          
				scene.renderer->setBlendState(platform::BlendState(
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
				scene.renderer->draw();
				scene.renderer->setBlendState(platform::BlendState::Alpha());
			}
			void renderCascade(const entity::Entity& entity, scene::Scene& scene)
			{
				Data& data = scene.light.get(entity);
      
				const TransformComponent transform = TransformSystem::getComponent(entity, scene);
				const glm::vec3 forward = glm::normalize(transform.getWorldForward());

				const bool update = (data.shadow_type == ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != ShadowType::kGenerated);

				if (data.dynamic_index >= data.dynamic_frequency)
				{
					data.dynamic_index = 0u;
				}

			String shadow_type = data.shadow_type == ShadowType::kNone ? "|NO_" : ("|" + scene.light.shader_shadow_type + "_");
			String shader_type = shadow_type + "CASCADE";

				// Generate shadow maps.
				if (update && data.shadow_type != ShadowType::kNone)
				{
					if (data.shadow_type == ShadowType::kGenerateOnce)
					{
						data.shadow_type = ShadowType::kGenerated;
					}

					entity::Entity main_camera = CameraSystem::getMainCamera(scene);
					const float camera_near = CameraSystem::getNearPlane(main_camera, scene).asMeter();
					const float camera_far = CameraSystem::getFarPlane(main_camera, scene).asMeter();
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
							CameraSystem::getFov(main_camera, scene).asRad(),
							(float)scene.window->getSize().x / (float)scene.window->getSize().y,
							cascade_near[i],
							cascade_far[i]
						);
						glm::mat4x4 camera_view = glm::inverse(TransformSystem::getWorld(main_camera, scene));

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
						float texels_per_unit = (float)data.render_target[i].getTexture()->getLayer(0u).getWidth() / (radius * 2.0f);
						glm::vec3 scalar(texels_per_unit);

						glm::mat4x4 look_at = glm::lookAtRH(glm::vec3(0.0f), -forward, glm::vec3(0.0f, 1.0f, 0.0f));
						look_at *= glm::vec4(scalar, 1.0f);
						glm::mat4x4 look_at_inv = glm::inverse(look_at);

						center = glm::vec3(glm::vec4(center, 1.0f) * look_at);
						center.x = std::floorf(center.x);
						center.y = std::floorf(center.y);
						center = glm::vec3(glm::vec4(center, 1.0f) * look_at_inv);

						// Construct a frustum out of the min and max.
						data.view_position[i] = center;
						data.view[i]          = glm::lookAtRH(data.view_position[i], data.view_position[i] + forward, glm::vec3(0.0f, 1.0f, 0.0f));
						data.projection[i]    = glm::orthoRH(-radius, radius, -radius, radius, -100.0f, 100.0f);
						data.depth[i]         = 200.0f;

						//scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_view_projection_matrix"), data.projection[i] * data.view[i]));

						// Clear the shadow map.
						scene.renderer->clearRenderTarget(data.render_target[i].getTexture(), glm::vec4(0.0f));
						scene.renderer->clearRenderTarget(data.depth_target[i].getTexture(), glm::vec4(1.0f));

						// Render to the shadow map.
						scene.renderer->bindShaderPass(
							platform::ShaderPass(
								Name("shadow_generate"),
								asset::ShaderManager::getInstance()->get(scene.light.shader_generate + shader_type),
								{},
								{ data.render_target[i], data.depth_target[i] }
							)
						);

						// Handle frustum culling.
						if (data.shadow_type == ShadowType::kDynamic)
						{
							data.culler[i].setCullFrequency(std::max(1u, 10u / data.dynamic_frequency));
						}
						else
						{
							data.culler[i].setCullFrequency(1u);
						}

						utilities::Frustum frustum;
						frustum.construct(data.projection[i], data.view[i]);

						// Render to the shadow map.
				MeshRenderSystem::renderAll(data.culler[i], frustum, scene);

						// Set up the post processing passes.
						scene.renderer->setMesh(scene.light.full_screen_mesh);
						scene.renderer->setSubMesh(0u);
						scene.renderer->setRasterizerState(platform::RasterizerState::SolidBack());
						scene.renderer->setBlendState(platform::BlendState::Alpha());

				for (uint32_t i = 0; i < scene.light.shader_modify_count; ++i)
				{
							scene.renderer->bindShaderPass(
								platform::ShaderPass(
									Name("shadow_modify"),
									asset::ShaderManager::getInstance()->get(scene.light.shader_modify + shadow_type + "HORIZONTAL"),
									{ data.render_target[i] },
									{ data.render_target[i] }
								)
							);
							scene.renderer->draw();
							scene.renderer->bindShaderPass(
								platform::ShaderPass(
									Name("shadow_modify"),
									asset::ShaderManager::getInstance()->get(scene.light.shader_modify + shadow_type + "VERTICAL"),
									{ data.render_target[i] },
									{ data.render_target[i] }
								)
							);
							scene.renderer->draw();
						}
					}
				}

				// Render lights to the light map.
				// Set up the post processing passes.
				scene.renderer->setMesh(scene.light.full_screen_mesh);
				scene.renderer->setSubMesh(0u);
				scene.renderer->setRasterizerState(platform::RasterizerState::SolidBack());

				// Render light using the shadow map.
				scene.renderer->bindShaderPass(
					platform::ShaderPass(
						Name("shadow_publish"),
						asset::ShaderManager::getInstance()->get(scene.light.shader_publish + shader_type),
						{
							data.render_target.at(0u),
							data.render_target.at(1u),
							data.render_target.at(2u),
							platform::RenderTarget(Name("texture"), data.texture),
							scene.post_process_manager->getTarget(Name("position")),
							scene.post_process_manager->getTarget(Name("normal")),
							scene.post_process_manager->getTarget(Name("metallic_roughness"))
						}, {
							scene.post_process_manager->getTarget(Name("light_map"))
						}
					)
				);

				/*scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_view_projection_matrix_01"), data.projection.at(0u) * data.view.at(0u)));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_view_projection_matrix_02"), data.projection.at(1u) * data.view.at(1u)));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_view_projection_matrix_03"), data.projection.at(2u) * data.view.at(2u)));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_camera_position_01"),        data.view_position.at(0u)));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_camera_position_02"),        data.view_position.at(1u)));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_camera_position_03"),        data.view_position.at(2u)));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_far_01"),                    data.depth.at(0u)));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_far_02"),                    data.depth.at(1u)));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_far_03"),                    data.depth.at(2u)));

				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_direction"), -forward));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_colour"),     data.colour * data.intensity));
				scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_ambient"),    data.ambient));*/
         
				scene.renderer->setBlendState(platform::BlendState(
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
				scene.renderer->draw();
				scene.renderer->setBlendState(platform::BlendState::Alpha());
			}
		}






		// The system data.
		namespace LightSystem
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
				LMB_ASSERT(it != entity_to_data.end(), "LIGHT: %llu does not have a component", entity);
				LMB_ASSERT(data[it->second].valid, "LIGHT: %llu's data was not valid", entity);
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







		namespace LightSystem
		{
			Data::Data(const Data& other)
			{
				type = other.type;
				shadow_type = other.shadow_type;
				colour = other.colour;
				ambient = other.ambient;
				intensity = other.intensity;
				render_target = other.render_target;
				depth_target = other.depth_target;
				texture = other.texture;
				culler = other.culler;
				entity = other.entity;
				cut_off = other.cut_off;
				outer_cut_off = other.outer_cut_off;
				size = other.size;
				enabled = other.enabled;
				rsm = other.rsm;
				dynamic_frequency = other.dynamic_frequency;
				dynamic_index = other.dynamic_index;
				depth = other.depth;
				projection = other.projection;
				view = other.view;
				view_position = other.view_position;
				valid = other.valid;
				render_target_texture = other.render_target_texture;
				depth_target_texture = other.depth_target_texture;
			}
			Data & Data::operator=(const Data & other)
			{
				type = other.type;
				shadow_type = other.shadow_type;
				colour = other.colour;
				ambient = other.ambient;
				intensity = other.intensity;
				render_target = other.render_target;
				depth_target = other.depth_target;
				texture = other.texture;
				culler = other.culler;
				entity = other.entity;
				cut_off = other.cut_off;
				outer_cut_off = other.outer_cut_off;
				size = other.size;
				enabled = other.enabled;
				rsm = other.rsm;
				dynamic_frequency = other.dynamic_frequency;
				dynamic_index = other.dynamic_index;
				depth = other.depth;
				projection = other.projection;
				view = other.view;
				view_position = other.view_position;
				valid = other.valid;
				render_target_texture = other.render_target_texture;
				depth_target_texture = other.depth_target_texture;

				return *this;
			}
		}
















    LightComponent::LightComponent(const entity::Entity& entity, scene::Scene& scene) 
			: IComponent(entity)
			, scene_(&scene)
    {
    }
    LightComponent::LightComponent(const LightComponent& other)
			: IComponent(other.entity_)
			, scene_(other.scene_)
		{
    }
    LightComponent::LightComponent()
			: IComponent(entity::InvalidEntity)
			, scene_(nullptr)
		{
    }
    void LightComponent::setSize(const float& size)
    {
      LightSystem::setSize(entity_, size, *scene_);
    }
    float LightComponent::getSize() const
    {
      return LightSystem::getSize(entity_, *scene_);
    }
    void LightComponent::setCutOff(const utilities::Angle& cut_off)
    {
      LightSystem::setCutOff(entity_, cut_off, *scene_);
    }
    utilities::Angle LightComponent::getCutOff() const
    {
      return LightSystem::getCutOff(entity_, *scene_);
    }
    void LightComponent::setOuterCutOff(const utilities::Angle& outer_cut_off)
    {
      LightSystem::setOuterCutOff(entity_, outer_cut_off, *scene_);
    }
    utilities::Angle LightComponent::getOuterCutOff() const
    {
      return LightSystem::getOuterCutOff(entity_, *scene_);
    }
    void LightComponent::setShadowType(const ShadowType& shadow_type)
    {
      LightSystem::setShadowType(entity_, shadow_type, *scene_);
    }
    ShadowType LightComponent::getShadowType() const
    {
      return LightSystem::getShadowType(entity_, *scene_);
    }
    void LightComponent::setColour(const glm::vec3& colour)
    {
      LightSystem::setColour(entity_, colour, *scene_);
    }
    glm::vec3 LightComponent::getColour() const
    {
      return LightSystem::getColour(entity_, *scene_);
    }
    void LightComponent::setAmbient(const glm::vec3& ambient)
    {
      LightSystem::setAmbient(entity_, ambient, *scene_);
    }
    glm::vec3 LightComponent::getAmbient() const
    {
      return LightSystem::getAmbient(entity_, *scene_);
    }
    void LightComponent::setIntensity(const float& intensity)
    {
      LightSystem::setIntensity(entity_, intensity, *scene_);
    }
    float LightComponent::getIntensity() const
    {
      return LightSystem::getIntensity(entity_, *scene_);
    }
		void LightComponent::setShadowMapSizePx(uint32_t shadow_map_size_px)
		{
			LightSystem::setShadowMapSizePx(entity_, shadow_map_size_px, *scene_);
		}
		uint32_t LightComponent::getShadowMapSizePx() const
		{
			return LightSystem::getShadowMapSizePx(entity_, *scene_);
		}
    void LightComponent::setTexture(asset::VioletTextureHandle texture)
    {
      LightSystem::setTexture(entity_, texture, *scene_);
    }
    asset::VioletTextureHandle LightComponent::getTexture() const
    {
      return LightSystem::getTexture(entity_, *scene_);
    }
    void LightComponent::setDepth(const float& depth)
    {
      LightSystem::setDepth(entity_, depth, *scene_);
    }
    float LightComponent::getDepth() const
    {
      return LightSystem::getDepth(entity_, *scene_);
    }
    void LightComponent::setEnabled(const bool& enabled)
    {
      LightSystem::setEnabled(entity_, enabled, *scene_);
    }
    bool LightComponent::getEnabled() const
    {
      return LightSystem::getEnabled(entity_, *scene_);
    }
    void LightComponent::setDynamicFrequency(const uint8_t& frequency)
    {
      LightSystem::setDynamicFrequency(entity_, frequency, *scene_);
    }
    uint8_t LightComponent::getDynamicFrequency() const
    {
      return LightSystem::getDynamicFrequency(entity_, *scene_);
    }
    void LightComponent::setLightType(LightType type)
    {
      LightSystem::setLightType(entity_, type, *scene_);
    }
    LightType LightComponent::getLightType() const
    {
      return LightSystem::getLightType(entity_, *scene_);
    }
    void LightComponent::setRSM(bool rsm)
    {
      LightSystem::setRSM(entity_, rsm, *scene_);
    }
    bool LightComponent::getRSM() const
    {
      return LightSystem::getRSM(entity_, *scene_);
    }
  }
}