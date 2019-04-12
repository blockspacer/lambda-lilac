#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "scene.h"
#include "systems/light_system.h"
#include "systems/wave_source_system.h"

#include "utils/renderable.h"
#include "platform/depth_stencil_state.h"
#include "platform/blend_state.h"
#include "platform/rasterizer_state.h"
#include <gui/gui.h>

namespace lambda
{
	namespace scene
	{
		void sceneInitialize(scene::Scene& scene)
		{
			components::CameraSystem::initialize(scene);
			components::MeshRenderSystem::initialize(scene);
			components::RigidBodySystem::initialize(scene);
			components::WaveSourceSystem::initialize(scene);
			components::LightSystem::initialize(scene);
		}
		void sceneUpdate(const float& delta_time, scene::Scene& scene)
		{
			components::LODSystem::update(delta_time, scene);
			components::MonoBehaviourSystem::update(delta_time, scene);
			components::WaveSourceSystem::update(delta_time, scene);
		}

		void sceneFixedUpdate(const float& delta_time, scene::Scene& scene)
		{
			components::RigidBodySystem::fixedUpdate(delta_time, scene);
			components::MonoBehaviourSystem::fixedUpdate(delta_time, scene);
		}

		struct CameraBatch
		{
			glm::mat4x4 view;
			glm::mat4x4 projection;
			glm::vec3 position;
			float near;
			float far;
			Vector<utilities::Renderable*> opaque;
			Vector<utilities::Renderable*> alpha;
			Vector<platform::ShaderPass>   shader_passes;
		};

		struct LightBatch
		{
			struct Face
			{
				glm::mat4x4 view_projection;
				glm::vec3   position;
				glm::vec3   direction;
				glm::vec3   colour;
				float       near;
				float       far;

				Vector<utilities::Renderable*> opaque;
				Vector<utilities::Renderable*> alpha;

				platform::ShaderPass           generate;
				Vector<platform::ShaderPass>   modify;
				platform::ShaderPass           publish;
			};
			Vector<Face> faces;
			asset::VioletMeshHandle full_screen_mesh;
		};

		void renderMeshes(platform::IRenderer* renderer, platform::ShaderVariableManager& shader_variable_manager, const Vector<utilities::Renderable*>& opaque, const Vector<utilities::Renderable*>& alpha, bool is_rh)
		{
			// -------------------------RENDER-STATIC-OPAQUE-------------------------
			renderer->setBlendState(platform::BlendState::Default());
			for (const utilities::Renderable* renderable : opaque)
			{
				renderer->setMesh(renderable->mesh);
				renderer->setSubMesh(renderable->sub_mesh);

				renderer->setTexture(renderable->albedo_texture, 0);
				renderer->setTexture(renderable->normal_texture, 1);
				renderer->setTexture(renderable->dmra_texture, 2);

				shader_variable_manager.setVariable(platform::ShaderVariable(Name("metallic_roughness"), glm::vec2(renderable->metallicness, renderable->roughness)));
				shader_variable_manager.setVariable(platform::ShaderVariable(Name("model_matrix"), renderable->model_matrix));

				auto sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);
				// TODO (Hilze): Implement.
				if (sub_mesh.io.double_sided == true || (sub_mesh.io.tex_alb >= 0 && renderable->mesh->getAttachedTextures().at(sub_mesh.io.tex_alb)->getLayer(0u).containsAlpha()))
					renderer->setRasterizerState(platform::RasterizerState::SolidNone());
				else
				{
					if (is_rh)
						renderer->setRasterizerState(platform::RasterizerState::SolidFront());
					else
						renderer->setRasterizerState(platform::RasterizerState::SolidBack());
				}

				renderer->draw();
			}
			// -------------------------RENDER-STATIC-ALPHA-------------------------
			renderer->setBlendState(platform::BlendState::Alpha());
			for (const utilities::Renderable* renderable : alpha)
			{
				renderer->setMesh(renderable->mesh);
				shader_variable_manager.setVariable(platform::ShaderVariable(Name("metallic_roughness"), glm::vec2(renderable->metallicness, renderable->roughness)));
				renderer->setSubMesh(renderable->sub_mesh);
				renderer->setTexture(renderable->albedo_texture, 0);
				renderer->setTexture(renderable->normal_texture, 1);
				renderer->setTexture(renderable->dmra_texture, 2);
				shader_variable_manager.setVariable(platform::ShaderVariable(Name("model_matrix"), renderable->model_matrix));

				auto sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);
				// TODO (Hilze): Implement.
				if (sub_mesh.io.double_sided == true || (sub_mesh.io.tex_alb >= 0 && renderable->mesh->getAttachedTextures().at(sub_mesh.io.tex_alb)->getLayer(0u).containsAlpha()))
					renderer->setRasterizerState(platform::RasterizerState::SolidNone());
				else
					renderer->setRasterizerState(platform::RasterizerState::SolidFront());

				renderer->draw();
			}
		}

		CameraBatch constructCamera(Scene& scene, entity::Entity entity)
		{
			LMB_ASSERT(entity, "CAMERA: Camera was not valid");
			auto camera = scene.camera.get(entity);
			lambda::utilities::Culler  culler;
			lambda::utilities::Frustum frustum;
			CameraBatch camera_batch;

			// Bind Camera.
			{
				auto transform = components::TransformSystem::getComponent(entity, scene);

				camera_batch.near       = camera.near_plane.asMeter();
				camera_batch.far        = camera.far_plane.asMeter();
				camera_batch.view       = glm::inverse(transform.getWorld());
				camera_batch.position   = transform.getWorldTranslation();
				camera_batch.projection = glm::perspective(
					camera.fov.asRad(),
					(float)scene.window->getSize().x / (float)scene.window->getSize().y,
					camera_batch.near,
					camera_batch.far
				);

				// Update the frustum.
				frustum.construct(
					camera_batch.projection,
					camera_batch.view
				);
			}

			// Create render list.
			components::MeshRenderSystem::createRenderList(culler, frustum, scene);
			auto statics  = culler.getStatics();
			auto dynamics = culler.getDynamics();
			components::MeshRenderSystem::createSortedRenderList(&statics, &dynamics, camera_batch.opaque, camera_batch.alpha, scene);
			camera_batch.shader_passes = camera.shader_passes;

			return camera_batch;
		}

		void renderCamera(platform::IRenderer* renderer, platform::ShaderVariableManager& shader_variable_manager, const CameraBatch& camera_batch)
		{
			renderer->beginTimer("Main Camera");
			renderer->pushMarker("Main Camera");

			// Set shader variables.
			shader_variable_manager.setVariable(platform::ShaderVariable(Name("view_matrix"), camera_batch.view));
			shader_variable_manager.setVariable(platform::ShaderVariable(Name("inverse_view_matrix"), glm::inverse(camera_batch.view)));
			shader_variable_manager.setVariable(platform::ShaderVariable(Name("projection_matrix"), camera_batch.projection));
			shader_variable_manager.setVariable(platform::ShaderVariable(Name("inverse_projection_matrix"), glm::inverse(camera_batch.projection)));
			shader_variable_manager.setVariable(platform::ShaderVariable(Name("view_projection_matrix"), camera_batch.projection * camera_batch.view));
			shader_variable_manager.setVariable(platform::ShaderVariable(Name("inverse_view_projection_matrix"), glm::inverse(camera_batch.projection * camera_batch.view)));
			shader_variable_manager.setVariable(platform::ShaderVariable(Name("camera_position"), camera_batch.position));
			shader_variable_manager.setVariable(platform::ShaderVariable(Name("camera_near"), camera_batch.near));
			shader_variable_manager.setVariable(platform::ShaderVariable(Name("camera_far"), camera_batch.far));

			// Draw all passes.
			for (uint32_t i = 0u; i < camera_batch.shader_passes.size(); ++i)
			{
				renderer->setDepthStencilState(platform::DepthStencilState::Default());
				renderer->bindShaderPass(camera_batch.shader_passes[i]);
				renderMeshes(renderer, shader_variable_manager, camera_batch.opaque, camera_batch.alpha, true);
			}

			renderer->popMarker();
			renderer->endTimer("Main Camera");

			// Reset the depth stencil state. // TODO (Hilze): Find out how to handle depth stencil state.
			renderer->setDepthStencilState(platform::DepthStencilState::Default());
		}

		LightBatch constructDirectional(entity::Entity entity, Scene& scene)
		{
			LightBatch light_batch;
			LightBatch::Face light_batch_face;
			light_batch.full_screen_mesh = scene.light.full_screen_mesh;

			auto& data = scene.light.get(entity);

			const auto transform = components::TransformSystem::getComponent(entity, scene);
			const glm::vec3 forward = glm::normalize(transform.getWorldForward());

			const bool update = (data.shadow_type == components::ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != components::ShadowType::kGenerated);

			if (data.dynamic_index >= data.dynamic_frequency)
				data.dynamic_index = 0u;

			Vector<platform::RenderTarget> shadow_maps(data.render_target.begin(), data.render_target.end());
			if (shadow_maps.empty())
				shadow_maps.push_back(scene.light.default_shadow_map);
			
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

			// Shader variables.
			light_batch_face.view_projection = data.projection.back() * data.view.back();
			light_batch_face.position = data.view_position.back();
			light_batch_face.direction = -forward;
			light_batch_face.colour = data.colour * data.intensity;
			scene.shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_ambient"), data.ambient));
			light_batch_face.near = 0.0f;
			light_batch_face.far = data.depth.back();

			String shadow_type = data.shadow_type == components::ShadowType::kNone ? "|NO_" : ("|" + scene.light.shader_shadow_type + "_");
			String shader_type = shadow_type + "DIRECTIONAL";

			// Generate shadow maps.
			if (update && data.shadow_type != components::ShadowType::kNone)
			{
				if (data.shadow_type == components::ShadowType::kGenerateOnce)
					data.shadow_type = components::ShadowType::kGenerated;

				const platform::RenderTarget depth_map = data.depth_target.at(0u);
				Vector<platform::RenderTarget> output = shadow_maps;
				output.push_back(depth_map);

				light_batch_face.generate = platform::ShaderPass(
					Name("shadow_generate"),
					asset::ShaderManager::getInstance()->get(scene.light.shader_generate + shader_type),
					{},
					output
				);

				// Handle frustum culling.
				if (data.shadow_type == components::ShadowType::kDynamic)
					data.culler.back().setCullFrequency(data.dynamic_frequency);
				else
					data.culler.back().setCullFrequency(1u);

				utilities::Frustum frustum;
				frustum.construct(data.projection.back(), data.view.back());

				components::MeshRenderSystem::createRenderList(data.culler.back(), frustum, scene);
				auto statics = data.culler.back().getStatics();
				auto dynamics = data.culler.back().getDynamics();
				components::MeshRenderSystem::createSortedRenderList(&statics, &dynamics, light_batch_face.opaque, light_batch_face.alpha, scene);

				// Draw all modify shaders.
				for (uint32_t i = 0; i < scene.light.shader_modify_count; ++i)
				 {
					light_batch_face.modify.push_back(
						platform::ShaderPass(
							Name("shadow_modify"),
							asset::ShaderManager::getInstance()->get(scene.light.shader_modify + shadow_type + "HORIZONTAL"),
							{ shadow_maps.at(0u) },
							{ shadow_maps.at(0u) }
						)
					);
					light_batch_face.modify.push_back(
						platform::ShaderPass(
							Name("shadow_modify"),
							asset::ShaderManager::getInstance()->get(scene.light.shader_modify + shadow_type + "VERTICAL"),
							{ shadow_maps.at(0u) },
							{ shadow_maps.at(0u) }
						)
					);
				}
			}

			// Render light using the shadow map.
			Vector<platform::RenderTarget> input = {
				shadow_maps.at(0u),
				platform::RenderTarget(Name("texture"), data.texture),
				scene.post_process_manager.getTarget(Name("position")),
				scene.post_process_manager.getTarget(Name("normal")),
				scene.post_process_manager.getTarget(Name("metallic_roughness"))
			};
			if (shadow_maps.size() > 1u)
				input.insert(input.end(), shadow_maps.begin() + 1u, shadow_maps.end());

			light_batch_face.publish = platform::ShaderPass(
				Name("shadow_publish"),
				asset::ShaderManager::getInstance()->get(scene.light.shader_publish + shader_type),
				input, {
					scene.post_process_manager.getTarget(Name("light_map"))
				}
			);

			light_batch.faces.push_back(light_batch_face);
			return light_batch;
		}

		Vector<LightBatch> constructLight(const CameraBatch& camera, Scene& scene)
		{
			Vector<LightBatch> light_batches;

			utilities::Frustum frustum;
			frustum.construct(camera.projection, camera.view);

			for (auto& data : scene.light.data)
			{
				bool enabled = data.enabled;

				if (enabled)
				{
					switch (data.type)
					{
					case components::LightType::kPoint:
					{
						glm::vec3 position = components::TransformSystem::getWorldTranslation(data.entity, scene);
						float radius = data.depth.back();
						if (!frustum.ContainsSphere(position, radius))
							enabled = false;
					}
					}
				}

				if (enabled)
				{
					switch (data.type)
					{
					case components::LightType::kDirectional:
						light_batches.push_back(constructDirectional(data.entity, scene));
						break;
					case components::LightType::kSpot:
						//constructSpot(data.entity, scene, light_batches);
						break;
					case components::LightType::kPoint:
						//constructPoint(data.entity, scene, light_batches);
						break;
					case components::LightType::kCascade:
						//constructCascade(data.entity, scene, light_batches);
						break;
					}
				}
			}

			return light_batches;
		}

		void renderLight(platform::IRenderer* renderer, platform::ShaderVariableManager& shader_variable_manager, platform::PostProcessManager& post_process_manager, const Vector<LightBatch>& light_batches)
		{
			renderer->beginTimer("Lighting");

			// Prepare the light buffer.
			renderer->pushMarker("Clear Light Buffer");
			renderer->clearRenderTarget(
				post_process_manager.getTarget(Name("light_map")).getTexture(),
				glm::vec4(0.0f)
			);
			renderer->popMarker();

			for (const auto& light_batch : light_batches)
			{
				for (const auto& face : light_batch.faces)
				{
					renderer->pushMarker("Light");

					{
						shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_view_projection_matrix"), face.view_projection));
						shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_camera_position"), face.position));
						shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_direction"), face.direction));
						shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_colour"), face.colour));
						shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_near"), face.near));
						shader_variable_manager.setVariable(platform::ShaderVariable(Name("light_far"), face.far));

						// Generate shadow maps.
						if (!face.generate.getOutputs().empty())
						{
							auto outputs = face.generate.getOutputs();
							for (uint32_t i = 0; i < outputs.size(); ++i)
							{
								if (i == outputs.size() - 1)
									renderer->clearRenderTarget(outputs[i].getTexture(), glm::vec4(1.0f));
								else
									renderer->clearRenderTarget(outputs[i].getTexture(), glm::vec4(0.0f));
							}
							renderer->bindShaderPass(face.generate);

							renderMeshes(renderer, shader_variable_manager, face.opaque, face.alpha, true);

							// Set up the post processing passes.
							renderer->setMesh(light_batch.full_screen_mesh);
							renderer->setSubMesh(0u);
							renderer->setRasterizerState(platform::RasterizerState::SolidBack());
							renderer->setBlendState(platform::BlendState::Alpha());

							// Draw all modify shaders.
							for (auto modify : face.modify)
							{
								renderer->bindShaderPass(modify);
								renderer->draw();
							}
						}

						// Render lights to the light map.
						// Set up the post processing passes.
						renderer->setMesh(light_batch.full_screen_mesh);
						renderer->setSubMesh(0u);
						renderer->setRasterizerState(platform::RasterizerState::SolidBack());

						// Render light using the shadow map.
						renderer->bindShaderPass(face.publish);

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


					renderer->popMarker();
				}
			}

			// Reset states.
			renderer->setRasterizerState(platform::RasterizerState::SolidFront());
			renderer->endTimer("Lighting");
		}

#define USE_MT 1

#if USE_MT
		std::atomic<int> k_can_read;
		std::atomic<int> k_can_write;
		scene::Scene k_scene;
		CameraBatch k_camera_batch;
		Vector<LightBatch> k_light_batches;

		///////////////////////////////////////////////////////////////////////////
		void flush()
		{
			while (true)
			{
				while (k_can_read.load() == 0) {
					std::this_thread::sleep_for(std::chrono::microseconds(10u));
				}

				k_can_read = 0;
				k_scene.renderer->setOverrideScene(&k_scene);
				k_scene.renderer->startFrame();

				//k_scene.gui->render(k_scene);

				renderCamera(k_scene.renderer, k_scene.shader_variable_manager, k_camera_batch);
				renderLight(k_scene.renderer, k_scene.shader_variable_manager, k_scene.post_process_manager, k_light_batches);

				k_scene.renderer->endFrame();
				k_scene.renderer->setOverrideScene(nullptr);
				k_scene = {};
				k_can_write = 1;
			}
		}
#endif

		void sceneOnRender(scene::Scene& scene)
		{
			components::RigidBodySystem::onRender(scene);

#if USE_MT
			static std::thread thead;
			if (!thead.joinable())
			{
				k_can_read = 0;
				k_can_write = 1;
				thead = std::thread(flush);
			}

			while (k_can_write.load() == 0) {
				std::this_thread::sleep_for(std::chrono::microseconds(10u));
			}

			k_can_write = 0;

			//// Required by other things..
			//k_scene.camera                  = scene.camera;
			//k_scene.light                   = scene.light;
			//k_scene.transform               = scene.transform;
			//k_scene.mesh_render             = scene.mesh_render;
			//k_scene.debug_renderer          = scene.debug_renderer;
			//
			//// Required by flush.
			k_scene.renderer                = scene.renderer;
			// Required by the renderer.
			k_scene.post_process_manager    = scene.post_process_manager;
			k_scene.shader_variable_manager = scene.shader_variable_manager;
			k_scene.window                  = scene.window;
			k_scene.gui                     = scene.gui;

			scene.debug_renderer.Clear();

			k_camera_batch  = constructCamera(scene, scene.camera.main_camera);
			k_light_batches = constructLight(k_camera_batch, scene);

			k_can_read = 1;
#else
			scene.renderer->setOverrideScene(&scene);
			scene.renderer->startFrame();

			Scene k_scene;
			CameraBatch k_camera_batch;
			LightBatch k_light_batches;

			// Required by the renderer.
			k_scene.post_process_manager    = scene.post_process_manager;
			k_scene.shader_variable_manager = scene.shader_variable_manager;
			k_scene.window                  = scene.window;
			k_scene.gui                     = scene.gui;

			//scene.gui->render(scene);

			CameraBatch camera_batch = constructCamera(scene, scene.camera.main_camera);
			Vector<LightBatch> light_batches = constructLight(camera_batch, scene);

			renderCamera(scene.renderer, scene.shader_variable_manager, camera_batch);
			renderLight(scene.renderer, scene.shader_variable_manager, scene.post_process_manager, light_batches);

			scene.renderer->endFrame();
			scene.renderer->setOverrideScene(nullptr);
#endif
		}
		void sceneCollectGarbage(scene::Scene& scene)
		{
			components::NameSystem::collectGarbage(scene);
			components::LODSystem::collectGarbage(scene);
			components::CameraSystem::collectGarbage(scene);
			components::MeshRenderSystem::collectGarbage(scene);
			components::TransformSystem::collectGarbage(scene);
			components::RigidBodySystem::collectGarbage(scene);
			components::ColliderSystem::collectGarbage(scene);
			components::MonoBehaviourSystem::collectGarbage(scene);
			components::WaveSourceSystem::collectGarbage(scene);
			components::LightSystem::collectGarbage(scene);
		}
		void sceneDeinitialize(scene::Scene& scene)
		{
			components::RigidBodySystem::deinitialize(scene);
			components::ColliderSystem::deinitialize(scene);
			components::NameSystem::deinitialize(scene);
			components::LODSystem::deinitialize(scene);
			components::CameraSystem::deinitialize(scene);
			components::MeshRenderSystem::deinitialize(scene);
			components::TransformSystem::deinitialize(scene);
			components::MonoBehaviourSystem::deinitialize(scene);
			components::WaveSourceSystem::deinitialize(scene);
			components::LightSystem::deinitialize(scene);
		}
	}
}