#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "scene.h"
#include "systems/light_system.h"
#include "systems/wave_source_system.h"

#include "utils/renderable.h"
#include "platform/depth_stencil_state.h"
#include "platform/blend_state.h"
#include "platform/rasterizer_state.h"
#include <gui/gui.h>
#include <memory/frame_heap.h>
#include <algorithm>
#include <utils/decompose_matrix.h>

#define USE_MT 1
#define USE_RENDERABLES 1

#if USE_MT
#include "utils/mt_manager.h"
#endif

namespace lambda
{
	namespace scene
	{
		Serializer::Serializer()
		{
		}

		Serializer::~Serializer()
		{
			for (auto& serializer : namespaces)
				foundation::Memory::destruct(serializer.second);
		}

		void Serializer::serialize(String name, String data)
		{
			Vector<String> s = split(name, '/');
			if (s.size() > 1 || (s.size() == 1 && name.back() == '/'))
			{
				auto it = namespaces.find(s[0]);
				if (it == namespaces.end())
				{
					namespaces.insert({ s[0], foundation::Memory::construct<Serializer>() });
					it = namespaces.find(s[0]);
				}
				namespaces[s[0]]->serialize(name.substr(s[0].size() + 1), data);
			}
			else if (s.empty())
				namespace_datas.push_back(data);
			else
				datas[s[0]] = data;
		}

		String Serializer::deserialize(String name)
		{
			const char* cstr = name.c_str();
			Vector<String> s = split(name, '/');
			if (s.size() > 1 || (s.size() == 1 && name.back() == '/'))
			{
				auto it = namespaces.find(s[0]);
				if (it == namespaces.end())
					return "";
				return it->second->deserialize(name.substr(s[0].size() + 1));
			}
			else if (s.empty())
				return namespace_datas[0];
			else
				return datas[s[0]];
		}

		Vector<String> Serializer::deserializeNamespace(String name)
		{
			Vector<String> s = split(name, '/');
			if (s.size() > 1 || (s.size() == 1 && name.back() == '/'))
			{
				auto it = namespaces.find(s[0]);
				if (it == namespaces.end())
					return{};
				return it->second->deserializeNamespace(name.substr(s[0].size() + 1));
			}
			else if (s.empty())
				return namespace_datas;
			else
				return{ datas[s[0]] };
		}

		void sceneInitialize(scene::Scene& scene)
		{
			components::CameraSystem::initialize(scene);
			components::MeshRenderSystem::initialize(scene);
			components::RigidBodySystem::initialize(scene);
			components::WaveSourceSystem::initialize(scene);
			components::LightSystem::initialize(scene);
			scene.debug_renderer.Initialize(scene);
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

#if USE_RENDERABLES
		struct SceneRenderable
		{
			~SceneRenderable() {}

			glm::mat4x4* mm;
			glm::vec4* mr;
			glm::vec4* em;
			uint32_t model_count = 0ul;
			uint32_t model_size = 0ul;
			asset::VioletMeshHandle mesh;
			uint32_t sub_mesh;
			asset::VioletTextureHandle albedo;
			asset::VioletTextureHandle normal;
			asset::VioletTextureHandle dmra;
			asset::VioletTextureHandle emissive;
		};
#endif

		struct SceneShaderPass
		{
			~SceneShaderPass() {};
			void operator=(const SceneShaderPass& other)
			{
				shader = other.shader;
				input  = other.input;
				output = other.output;
			}

			asset::VioletShaderHandle      shader;
			Vector<platform::RenderTarget> input;
			Vector<platform::RenderTarget> output;
		};

		struct CameraBatch
		{
			~CameraBatch() {};

			glm::mat4x4 view;
			glm::mat4x4 projection;
			glm::vec3 position;
			float near;
			float far;
#if USE_RENDERABLES
			Vector<SceneRenderable*> renderables;
#else
			Vector<utilities::Renderable*> opaque;
			Vector<utilities::Renderable*> alpha;
#endif
			Vector<SceneShaderPass>  shader_passes;

			void operator=(const CameraBatch& other)
			{
				view          = other.view;
				projection    = other.projection;
				position      = other.position;
				near          = other.near;
				far           = other.far;
				shader_passes = other.shader_passes;
#if USE_RENDERABLES
				renderables   = other.renderables;
#else
				opaque        = other.opaque;
				alpha         = other.alpha;
#endif
			}
		};

		struct LightBatch
		{
			~LightBatch() {};

			struct Face
			{
				~Face() {};
			
				glm::mat4x4 view_projection;
				glm::vec3   direction;

#if USE_RENDERABLES
				Vector<SceneRenderable*>  renderables;
#else
				Vector<utilities::Renderable*> opaque;
				Vector<utilities::Renderable*> alpha;
#endif

				SceneShaderPass           generate;
				Vector<SceneShaderPass>   modify;

				void operator=(const Face& other)
				{
					view_projection = other.view_projection;
					direction       = other.direction;
#if USE_RENDERABLES
					renderables     = other.renderables;
#else
					opaque          = other.opaque;
					alpha           = other.alpha;
#endif
					generate        = other.generate;
					modify          = other.modify;
				}
			};

			Vector<Face> faces;
			Vector<platform::RenderTarget> clear_queue_depth;
			Vector<platform::RenderTarget> clear_queue_texture;
			asset::VioletMeshHandle full_screen_mesh;

			glm::vec3   position;
			glm::vec3   colour;
			float       near;
			float       far;
			bool        is_rh;

			SceneShaderPass publish;

			void operator=(const LightBatch& other)
			{
				faces               = other.faces;
				full_screen_mesh    = other.full_screen_mesh;
				position            = other.position;
				colour              = other.colour;
				near                = other.near;
				far                 = other.far;
				publish             = other.publish;
				is_rh               = other.is_rh;
				clear_queue_depth   = other.clear_queue_depth;
				clear_queue_texture = other.clear_queue_texture;
			}
		};

#if USE_RENDERABLES
		static constexpr uint32_t kNumRenderModels = 8ull;
		void convertRenderableList(const Vector<utilities::Renderable*>& u_renderables, Vector<SceneRenderable*>& s_renderables)
		{
			for (const utilities::Renderable* renderable : u_renderables)
			{
				bool found = false;
				for (auto s_renderable : s_renderables)
				{
					if (s_renderable->mesh     == renderable->mesh &&
						s_renderable->sub_mesh == renderable->sub_mesh &&
						s_renderable->albedo   == renderable->albedo_texture &&
						s_renderable->normal   == renderable->normal_texture &&
						s_renderable->dmra     == renderable->dmra_texture &&
						s_renderable->emissive == renderable->emissive_texture)
					{
						if (s_renderable->model_count + 1ul > s_renderable->model_size)
						{
							uint32_t new_model_size = s_renderable->model_size + kNumRenderModels;
							s_renderable->mm = (glm::mat4x4*)foundation::GetFrameHeap()->realloc(
								s_renderable->mm,
								sizeof(glm::mat4x4) * s_renderable->model_size,
								sizeof(glm::mat4x4) * new_model_size
							);
							s_renderable->mr = (glm::vec4*)foundation::GetFrameHeap()->realloc(
								s_renderable->mr,
								sizeof(glm::vec4) * s_renderable->model_size,
								sizeof(glm::vec4) * new_model_size
							);
							s_renderable->em = (glm::vec4*)foundation::GetFrameHeap()->realloc(
								s_renderable->em,
								sizeof(glm::vec4) * s_renderable->model_size,
								sizeof(glm::vec4) * new_model_size
							);
							s_renderable->model_size = new_model_size;
						}
						s_renderable->mm[s_renderable->model_count]   = renderable->model_matrix;
						s_renderable->mr[s_renderable->model_count]   = glm::vec4(renderable->metallicness, renderable->roughness, 0.0f, 0.0f);
						s_renderable->em[s_renderable->model_count++] = glm::vec4(renderable->emissiveness.x, renderable->emissiveness.y, renderable->emissiveness.z, 0.0f);
						found = true;
						break;
					}
				}

				if (!found)
				{
					SceneRenderable* s_renderable = foundation::GetFrameHeap()->construct<SceneRenderable>();
					s_renderable->mesh     = renderable->mesh;
					s_renderable->sub_mesh = renderable->sub_mesh;
					s_renderable->albedo   = renderable->albedo_texture;
					s_renderable->normal   = renderable->normal_texture;
					s_renderable->dmra     = renderable->dmra_texture;
					s_renderable->emissive = renderable->emissive_texture;
					s_renderable->model_count = 0ul;
					s_renderable->model_size  = 1ul;
					s_renderable->mm = (glm::mat4x4*)foundation::GetFrameHeap()->alloc(sizeof(glm::mat4x4) * s_renderable->model_size);
					s_renderable->mr = (glm::vec4*)foundation::GetFrameHeap()->alloc(sizeof(glm::vec4) * s_renderable->model_size);
					s_renderable->em = (glm::vec4*)foundation::GetFrameHeap()->alloc(sizeof(glm::vec4) * s_renderable->model_size);
					
					if (renderable->emissiveness.x != 0.0f || renderable->emissiveness.y != 0.0f || renderable->emissiveness.z != 0.0f)
						int xxx = 0;
					
					s_renderable->mm[s_renderable->model_count]   = renderable->model_matrix;
					s_renderable->mr[s_renderable->model_count]   = glm::vec4(renderable->metallicness, renderable->roughness, 0.0, 0.0f);
					s_renderable->em[s_renderable->model_count++] = glm::vec4(renderable->emissiveness.x, renderable->emissiveness.y, renderable->emissiveness.z, 0.0f);
					s_renderables.push_back(s_renderable);
				}
			}
		}

		void renderMeshes(platform::IRenderer* renderer, const Vector<SceneRenderable*>& renderables, platform::RasterizerState::CullMode cull_mode)
		{
			struct CBData
			{
				glm::mat4x4 mm[64];
				glm::vec4 mr[64];
				glm::vec4 em[64];
			} data;

			platform::IRenderBuffer* cb = renderer->allocRenderBuffer(sizeof(data), platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagTransient | platform::IRenderBuffer::kFlagDynamic, &data);
			renderer->setConstantBuffer(cb, cbPerMeshIdx);

			renderer->setBlendState(platform::BlendState::Alpha());
			for (SceneRenderable* renderable : renderables)
			{
				renderer->setMesh(renderable->mesh);
				renderer->setSubMesh(renderable->sub_mesh);

				renderer->setTexture(renderable->albedo,   0);
				renderer->setTexture(renderable->normal,   1);
				renderer->setTexture(renderable->dmra,     2);
				renderer->setTexture(renderable->emissive, 3);

				auto sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);
				// TODO (Hilze): Implement.
				if (sub_mesh.io.double_sided == true || (sub_mesh.io.tex_alb >= 0 && renderable->mesh->getAttachedTextures().at(sub_mesh.io.tex_alb)->getLayer(0u).containsAlpha()))
					renderer->setRasterizerState(platform::RasterizerState::SolidNone());
				else
				{
					if (cull_mode == platform::RasterizerState::CullMode::kBack)
						renderer->setRasterizerState(platform::RasterizerState::SolidBack());
					else if (cull_mode == platform::RasterizerState::CullMode::kFront)
						renderer->setRasterizerState(platform::RasterizerState::SolidFront());
					else
						renderer->setRasterizerState(platform::RasterizerState::SolidNone());
				}

				uint32_t offset = 0u;
				while (offset < renderable->model_count)
				{
					uint32_t count = std::min(renderable->model_count - offset, 64u);
					memcpy(data.mm, renderable->mm + offset, count * sizeof(glm::mat4x4));
					memcpy(data.mr, renderable->mr + offset, count * sizeof(glm::vec4));
					memcpy(data.em, renderable->em + offset, count * sizeof(glm::vec4));
					memcpy(cb->lock(), &data, sizeof(data));
					cb->unlock();

					renderer->draw(count);
					offset += 64;
				}

				//foundation::GetFrameHeap()->deconstruct(renderable);
			}
		}
#else
		void renderMeshes(platform::IRenderer* renderer, const Vector<utilities::Renderable*>& renderables, platform::RasterizerState::CullMode cull_mode)
		{
			struct CBData
			{
				glm::mat4x4 mm[64];
				glm::vec4 mr[64];
				glm::vec4 em[64];
			} data;

			platform::IRenderBuffer* cb = renderer->allocRenderBuffer(sizeof(data), platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagTransient | platform::IRenderBuffer::kFlagDynamic, &data);
			renderer->setConstantBuffer(cb, cbPerMeshIdx);

			renderer->setBlendState(platform::BlendState::Alpha());
			for (utilities::Renderable* renderable : renderables)
			{
				glm::vec4 mr(renderable->metallicness, renderable->roughness, 0.0f, 0.0f);
				glm::vec4 em(renderable->emissiveness.x, renderable->emissiveness.y, renderable->emissiveness.z, 0.0f);
				memcpy(data.mm, &renderable->model_matrix, sizeof(glm::mat4x4));
				memcpy(data.mr, &mr, sizeof(glm::vec4));
				memcpy(data.em, &em, sizeof(glm::vec4));
				memcpy(cb->lock(), &data, sizeof(data));
				cb->unlock();

				renderer->setMesh(renderable->mesh);
				renderer->setSubMesh(renderable->sub_mesh);

				renderer->setTexture(renderable->albedo_texture,   0);
				renderer->setTexture(renderable->normal_texture,   1);
				renderer->setTexture(renderable->dmra_texture,     2);
				renderer->setTexture(renderable->emissive_texture, 3);

				auto sub_mesh = renderable->mesh->getSubMeshes().at(renderable->sub_mesh);
				// TODO (Hilze): Implement.
				if (sub_mesh.io.double_sided == true || (sub_mesh.io.tex_alb >= 0 && renderable->mesh->getAttachedTextures().at(sub_mesh.io.tex_alb)->getLayer(0u).containsAlpha()))
					renderer->setRasterizerState(platform::RasterizerState::SolidNone());
				else
				{
					if (cull_mode == platform::RasterizerState::CullMode::kBack)
						renderer->setRasterizerState(platform::RasterizerState::SolidBack());
					else if (cull_mode == platform::RasterizerState::CullMode::kFront)
						renderer->setRasterizerState(platform::RasterizerState::SolidFront());
					else
						renderer->setRasterizerState(platform::RasterizerState::SolidNone());
				}

				renderer->draw(1);
			}
		}
#endif

		CameraBatch constructCamera(Scene& scene, entity::Entity entity)
		{
			LMB_ASSERT(entity, "CAMERA: Camera was not valid");
			auto camera = scene.camera.get(entity);
			lambda::utilities::Culler  culler;
			lambda::utilities::Frustum frustum;
			CameraBatch camera_batch;

			// Bind Camera.
			{
				camera_batch.near       = camera.near_plane.asMeter();
				camera_batch.far        = camera.far_plane.asMeter();
				camera_batch.view       = glm::inverse(camera.world_matrix);
				utilities::decomposeMatrix(camera.world_matrix, nullptr, nullptr, &camera_batch.position);
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
#if USE_RENDERABLES
			Vector<utilities::Renderable*> opaque;
			Vector<utilities::Renderable*> alpha;
			components::MeshRenderSystem::createSortedRenderList(&statics,  opaque, alpha, scene);
			components::MeshRenderSystem::createSortedRenderList(&dynamics, opaque, alpha, scene);
			convertRenderableList(opaque, camera_batch.renderables);
			convertRenderableList(alpha, camera_batch.renderables);
#else
			components::MeshRenderSystem::createSortedRenderList(&statics,  camera_batch.opaque, camera_batch.alpha, scene);
			components::MeshRenderSystem::createSortedRenderList(&dynamics, camera_batch.opaque, camera_batch.alpha, scene);
#endif
			for (const auto& shader_pass : camera.shader_passes)
			{
				SceneShaderPass sp;
				sp.shader = shader_pass.getShader();
				sp.input  = shader_pass.getInputs();
				sp.output = shader_pass.getOutputs();
				camera_batch.shader_passes.push_back(sp);
			}

			return camera_batch;
		}

		void renderCamera(platform::IRenderer* renderer, const CameraBatch& camera_batch)
		{
			renderer->beginTimer("Main Camera");
			renderer->pushMarker("Main Camera");

			// Set shader variables.
			struct CBData
			{
				glm::mat4x4 view_matrix;
				glm::mat4x4 projection_matrix;
				glm::mat4x4 view_projection_matrix;
				glm::mat4x4 inverse_view_matrix;
				glm::mat4x4 inverse_projection_matrix;
				glm::mat4x4 inverse_view_projection_matrix;
				glm::vec3   camera_position;
				float       camera_far;
				float       camera_near;
			} data = {
				camera_batch.view,
				camera_batch.projection,
				camera_batch.projection * camera_batch.view,
				glm::inverse(camera_batch.view),
				glm::inverse(camera_batch.projection),
				glm::inverse(camera_batch.view * camera_batch.projection),
				camera_batch.position,
				camera_batch.near,
				camera_batch.far
			};

			platform::IRenderBuffer* cb = renderer->allocRenderBuffer(sizeof(data), platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagTransient | platform::IRenderBuffer::kFlagImmutable, &data);
			renderer->setConstantBuffer(cb, cbPerCameraIdx);

			// Draw all passes.
			for (uint32_t i = 0u; i < camera_batch.shader_passes.size(); ++i)
			{
				if (i == 0)
					renderer->setDepthStencilState(platform::DepthStencilState::Default());
				else
					renderer->setDepthStencilState(platform::DepthStencilState::Equal());

				renderer->bindShaderPass(platform::ShaderPass(Name(""), camera_batch.shader_passes[i].shader, camera_batch.shader_passes[i].input, camera_batch.shader_passes[i].output));
#if USE_RENDERABLES
				renderMeshes(renderer, camera_batch.renderables, platform::RasterizerState::CullMode::kFront);
#else
				renderMeshes(renderer, camera_batch.opaque, platform::RasterizerState::CullMode::kFront);
				renderMeshes(renderer, camera_batch.alpha, platform::RasterizerState::CullMode::kFront);
#endif
			}

			renderer->popMarker();
			renderer->endTimer("Main Camera");

			// Reset the depth stencil state. // TODO (Hilze): Find out how to handle depth stencil state.
			renderer->setDepthStencilState(platform::DepthStencilState::Default());
		}

		static Map<size_t, asset::VioletShaderHandle>  g_lightShaders;
		static Map<size_t, asset::VioletTextureHandle> g_lightTextures;

		static Map<entity::Entity, void*> g_generatedOnce;

		LightBatch constructDirectional(entity::Entity entity, Scene& scene)
		{
			LightBatch light_batch;
			LightBatch::Face light_batch_face;
			light_batch.full_screen_mesh = scene.light.full_screen_mesh;
			light_batch.is_rh = true;

			auto& data = scene.light.get(entity);

			if (data.shadow_type == components::ShadowType::kGenerateOnce)
				data.shadow_type = (g_generatedOnce[entity] == data.depth_target[0].getTexture().get()) ? components::ShadowType::kGenerated : components::ShadowType::kGenerateOnce;

			const glm::vec3 forward = ((glm::mat3x3)data.world_matrix) * glm::vec3(0.0f, 0.0f, -1.0f);

			const bool update = (data.shadow_type == components::ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != components::ShadowType::kGenerated);

			if (data.dynamic_index >= data.dynamic_frequency)
				data.dynamic_index = 0u;

			Vector<platform::RenderTarget> shadow_maps(data.render_target.begin(), data.render_target.end());
			if (shadow_maps.empty())
				shadow_maps.push_back(scene.light.default_shadow_map);
			
			// Setup camera.
			//if (update)
			{
				// Set everything up.
				float light_depth = data.depth.back();
				float smh_size = data.size * 0.5f;
				glm::vec3 translation;
				utilities::decomposeMatrix(data.world_matrix, nullptr, nullptr, &translation);

				// Remove shimmering
				{
					float texels_per_unit = (float)shadow_maps.begin()->getTexture()->getLayer(0u).getWidth() / data.size;
					glm::vec3 scalar(texels_per_unit);

					glm::mat4x4 look_at = glm::lookAtRH(glm::vec3(0.0f), forward, glm::vec3(0.0f, 1.0f, 0.0f));
					look_at *= glm::vec4(scalar, 1.0f);
					glm::mat4x4 look_at_inv = glm::inverse(look_at);

					translation   = glm::vec3(look_at * glm::vec4(translation, 1.0f));
					translation.x = std::floorf(translation.x);
					translation.y = std::floorf(translation.y);
					translation   = glm::vec3(look_at_inv * glm::vec4(translation, 1.0f));
				}

				// Update matrices.
				data.view.back()          = glm::lookAtRH(translation, translation + forward, glm::vec3(0.0f, 1.0f, 0.0f));
				data.view_position.back() = translation;
				data.projection.back() = glm::orthoRH(-smh_size, smh_size, -smh_size, smh_size, -light_depth * 0.5f, light_depth * 0.5f);
			}

			// Shader variables.
			light_batch_face.view_projection = data.projection.back() * data.view.back();
			light_batch_face.direction = -forward;
			
			light_batch.position = data.view_position.back();
			light_batch.colour = data.colour * data.intensity;
			light_batch.near = 0.0f;
			light_batch.far = data.depth.back();


			String shadow_type = data.shadow_type == components::ShadowType::kNone ? "|NO_" : ("|" + scene.light.shader_shadow_type + "_");
			String shader_type = shadow_type + "DIRECTIONAL";

			// Generate shadow maps.
			if (update && data.shadow_type != components::ShadowType::kNone)
			{
				if (data.shadow_type == components::ShadowType::kGenerateOnce)
				{
					g_generatedOnce[entity] = data.depth_target[0].getTexture().get();
					data.shadow_type = components::ShadowType::kGenerated;
				}

				const platform::RenderTarget depth_map = data.depth_target.at(0u);
				Vector<platform::RenderTarget> output = shadow_maps;
				output.push_back(depth_map);

				for (const auto& shadow_map : shadow_maps)
					light_batch.clear_queue_texture.push_back(shadow_map);
				light_batch.clear_queue_depth.push_back(depth_map);

				Name shader_name(scene.light.shader_generate + shader_type);
				if (g_lightShaders.find(shader_name.getHash()) == g_lightShaders.end())
					g_lightShaders[shader_name.getHash()] = asset::ShaderManager::getInstance()->get(shader_name);
				light_batch_face.generate.shader = g_lightShaders[shader_name.getHash()];

				light_batch_face.generate.input  = {};
				light_batch_face.generate.output = output;
				
				for (const auto& o : output)
					g_lightTextures[o.getTexture().getHash()] = o.getTexture();

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
#if USE_RENDERABLES
				Vector<utilities::Renderable*> opaque;
				Vector<utilities::Renderable*> alpha;
				components::MeshRenderSystem::createSortedRenderList(&statics, opaque, alpha, scene);
				components::MeshRenderSystem::createSortedRenderList(&dynamics, opaque, alpha, scene);
				convertRenderableList(opaque, light_batch_face.renderables);
				convertRenderableList(alpha, light_batch_face.renderables);
#else
				components::MeshRenderSystem::createSortedRenderList(&statics, light_batch_face.opaque, light_batch_face.alpha, scene);
				components::MeshRenderSystem::createSortedRenderList(&dynamics, light_batch_face.opaque, light_batch_face.alpha, scene);
#endif

				String config = "__temp_target_" + toString(shadow_maps.at(0u).getTexture()->getLayer(0).getWidth()) + "_" + toString(shadow_maps.at(0u).getTexture()->getLayer(0).getHeight()) + "__";
				asset::VioletTextureHandle temp = asset::TextureManager::getInstance()->create(Name(config),
					shadow_maps.at(0u).getTexture()->getLayer(0).getWidth(),
					shadow_maps.at(0u).getTexture()->getLayer(0).getHeight(),
					1,
					shadow_maps.at(0u).getTexture()->getLayer(0).getFormat(),
					shadow_maps.at(0u).getTexture()->getLayer(0).getFlags()
				);
				temp->setKeepInMemory(true);

				platform::RenderTarget rt_input = shadow_maps.at(0u);
				platform::RenderTarget rt_output(config, temp);
				g_lightTextures[rt_input.getTexture().getHash()] = rt_input.getTexture();
				g_lightTextures[rt_output.getTexture().getHash()] = rt_output.getTexture();

				// Draw all modify shaders.
				for (uint32_t i = 0; i < scene.light.shader_modify_count; ++i)
				 {
					SceneShaderPass modify;
					modify.input  = { rt_input };
					modify.output = { rt_output };
					
					Name shader_name(scene.light.shader_modify + shadow_type + "HORIZONTAL");
					if (g_lightShaders.find(shader_name.getHash()) == g_lightShaders.end())
						g_lightShaders[shader_name.getHash()] = asset::ShaderManager::getInstance()->get(shader_name);
					modify.shader = g_lightShaders[shader_name.getHash()];

					light_batch_face.modify.push_back(modify);
					platform::RenderTarget rt_temp = rt_input;
					rt_input = rt_output;
					rt_output = rt_temp;
					modify.input = { rt_input };
					modify.output = { rt_output };
					
					shader_name = Name(scene.light.shader_modify + shadow_type + "VERTICAL");
					if (g_lightShaders.find(shader_name.getHash()) == g_lightShaders.end())
						g_lightShaders[shader_name.getHash()] = asset::ShaderManager::getInstance()->get(shader_name);
					modify.shader = g_lightShaders[shader_name.getHash()];

					light_batch_face.modify.push_back(modify);
					rt_temp = rt_input;
					rt_input = rt_output;
					rt_output = rt_temp;
				}
			}

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

			Name shader_name(scene.light.shader_publish + shader_type);
			if (g_lightShaders.find(shader_name.getHash()) == g_lightShaders.end())
				g_lightShaders[shader_name.getHash()] = asset::ShaderManager::getInstance()->get(shader_name);
			light_batch.publish.shader = g_lightShaders[shader_name.getHash()];

			light_batch.publish.input  = input;
			light_batch.publish.output = { scene.post_process_manager->getTarget(Name("light_map")) };

			light_batch.faces.push_back(light_batch_face);
			return light_batch;
		}

		LightBatch constructPoint(entity::Entity entity, Scene& scene)
		{
			LightBatch light_batch;
			LightBatch::Face light_batch_faces[6];
			light_batch.full_screen_mesh = scene.light.full_screen_mesh;
			light_batch.is_rh = false;

			auto& data = scene.light.get(entity);

			if (data.shadow_type == components::ShadowType::kGenerateOnce)
				data.shadow_type = (g_generatedOnce[entity] == data.depth_target[0].getTexture().get()) ? components::ShadowType::kGenerated : components::ShadowType::kGenerateOnce;

			static const glm::vec3 g_forwards[6u] = {
				glm::vec3(1.0f, 0.0f, 0.0f),
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
			const bool update = (data.shadow_type == components::ShadowType::kDynamic) ? (++data.dynamic_index >= data.dynamic_frequency) : (data.shadow_type != components::ShadowType::kGenerated);

			if (data.dynamic_index >= data.dynamic_frequency)
				data.dynamic_index = 0u;

			// Setup camera.
			//if (update)
			{
				for (uint32_t i = 0; i < 6; ++i)
				{
					// Set everything up.
					glm::vec3 translation;
					utilities::decomposeMatrix(data.world_matrix, nullptr, nullptr, &translation);

					auto view_size = data.view.size();
					auto view_position_size = data.view_position.size();
					auto projection_size = data.projection.size();

					// Remove shimmering
					{
						float texels_per_unit = (float)shadow_map.getTexture()->getLayer(0u).getWidth() / data.size;
						glm::vec3 scalar(texels_per_unit);

						glm::mat4x4 look_at = glm::lookAtRH(glm::vec3(0.0f), g_forwards[i], glm::vec3(0.0f, 1.0f, 0.0f));
						look_at *= glm::vec4(scalar, 1.0f);
						glm::mat4x4 look_at_inv = glm::inverse(look_at);

						translation = glm::vec3(look_at * glm::vec4(translation, 1.0f));
						translation.x = std::floorf(translation.x);
						translation.y = std::floorf(translation.y);
						translation = glm::vec3(look_at_inv * glm::vec4(translation, 1.0f));
					}

					data.view[i] = glm::lookAtLH(glm::vec3(0.0f), g_forwards[i], g_ups[i]), glm::vec3(-1.0f, 1.0f, 1.0f);
					if (i == 2 || i == 3)   data.view[i] = glm::rotate(data.view[i], 1.5708f * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
					data.view[i] = glm::translate(data.view[i], -translation);
					data.view_position[i] = translation;
					data.projection[i] = glm::perspectiveLH(1.5708f, 1.0f, 0.001f, data.depth[i]); // 1.5708f radians == 90 degrees.
				}
			}


			// Shader variables.
			light_batch.position = data.view_position.back();
			light_batch.colour = data.colour * data.intensity;
			light_batch.near = 0.0f;
			light_batch.far = data.depth.back();

			String shadow_type = data.shadow_type == components::ShadowType::kNone ? "|NO_" : ("|" + scene.light.shader_shadow_type + "_");
			String shader_type = shadow_type + "POINT";

			for (uint32_t i = 0; i < 6; ++i)
			{
				light_batch_faces[i].view_projection = data.projection[i] * data.view[i];
				light_batch_faces[i].direction = -g_forwards[i];
			}

			// Generate shadow maps.
			if (update && data.shadow_type != components::ShadowType::kNone)
			{
				bool statics_only = (data.shadow_type == components::ShadowType::kGenerateOnce);

				if (data.shadow_type == components::ShadowType::kGenerateOnce)
				{
					g_generatedOnce[entity] = data.depth_target[0].getTexture().get();
					data.shadow_type = components::ShadowType::kGenerated;
				}

				// Clear the shadow map.
				platform::RenderTarget depth_map = data.depth_target.at(0u);
				light_batch.clear_queue_texture.push_back(shadow_map);
				light_batch.clear_queue_depth.push_back(depth_map);

				g_lightTextures[depth_map.getTexture().getHash()]  = depth_map.getTexture();
				g_lightTextures[shadow_map.getTexture().getHash()] = shadow_map.getTexture();

				for (uint32_t i = 0; i < 6; ++i)
				{
					// Render to the shadow map.
					shadow_map.setLayer(i);
					depth_map.setLayer(i);

					Name shader_name(scene.light.shader_generate + shader_type);
					if (g_lightShaders.find(shader_name.getHash()) == g_lightShaders.end())
						g_lightShaders[shader_name.getHash()] = asset::ShaderManager::getInstance()->get(shader_name);
					light_batch_faces[i].generate.shader = g_lightShaders[shader_name.getHash()];

					light_batch_faces[i].generate.input = {};
					light_batch_faces[i].generate.output = { shadow_map, depth_map };

					// Handle frustum culling.
					if (data.shadow_type == components::ShadowType::kDynamic)
						data.culler[i].setCullFrequency(std::max(1u, 10u / data.dynamic_frequency));
					else
						data.culler[i].setCullFrequency(1u);
					utilities::Frustum frustum;
					frustum.construct(data.projection[i], data.view[i]);

					// Render to the shadow map.
					if (statics_only)
					{
						components::MeshRenderSystem::createRenderList(data.culler.back(), frustum, scene);
						auto statics = data.culler.back().getStatics();
#if USE_RENDERABLES
						Vector<utilities::Renderable*> opaque;
						Vector<utilities::Renderable*> alpha;
						components::MeshRenderSystem::createSortedRenderList(&statics, opaque, alpha, scene);
						convertRenderableList(opaque, light_batch_faces[i].renderables);
						convertRenderableList(alpha, light_batch_faces[i].renderables);
#else
						components::MeshRenderSystem::createSortedRenderList(&statics, light_batch_faces[i].opaque, light_batch_faces[i].alpha, scene);
#endif
					}
					else
					{
						components::MeshRenderSystem::createRenderList(data.culler.back(), frustum, scene);
						auto statics = data.culler.back().getStatics();
						auto dynamics = data.culler.back().getDynamics();
#if USE_RENDERABLES
						Vector<utilities::Renderable*> opaque;
						Vector<utilities::Renderable*> alpha;
						components::MeshRenderSystem::createSortedRenderList(&statics, opaque, alpha, scene);
						components::MeshRenderSystem::createSortedRenderList(&dynamics, opaque, alpha, scene);
						convertRenderableList(opaque, light_batch_faces[i].renderables);
						convertRenderableList(alpha, light_batch_faces[i].renderables);
#else
						components::MeshRenderSystem::createSortedRenderList(&statics, light_batch_faces[i].opaque, light_batch_faces[i].alpha, scene);
						components::MeshRenderSystem::createSortedRenderList(&dynamics, light_batch_faces[i].opaque, light_batch_faces[i].alpha, scene);
#endif
					}

					String config1 = "__temp_target_cube1_" + toString(shadow_map.getTexture()->getLayer(0).getWidth()) + "_" + toString(shadow_map.getTexture()->getLayer(0).getHeight()) + "__";
					asset::VioletTextureHandle temp1 = asset::TextureManager::getInstance()->create(Name(config1),
						shadow_map.getTexture()->getLayer(0).getWidth(),
						shadow_map.getTexture()->getLayer(0).getHeight(),
						1,
						shadow_map.getTexture()->getLayer(0).getFormat(),
						shadow_map.getTexture()->getLayer(0).getFlags()
					);
					temp1->setKeepInMemory(true);

					String config2 = "__temp_target_cube2_" + toString(shadow_map.getTexture()->getLayer(0).getWidth()) + "_" + toString(shadow_map.getTexture()->getLayer(0).getHeight()) + "__";
					asset::VioletTextureHandle temp2 = asset::TextureManager::getInstance()->create(Name(config2),
						shadow_map.getTexture()->getLayer(0).getWidth(),
						shadow_map.getTexture()->getLayer(0).getHeight(),
						1,
						shadow_map.getTexture()->getLayer(0).getFormat(),
						shadow_map.getTexture()->getLayer(0).getFlags()
					);
					temp2->setKeepInMemory(true);

					platform::RenderTarget rt_input = shadow_map;
					platform::RenderTarget rt_temp1(config1, temp1);
					platform::RenderTarget rt_temp2(config2, temp2);
					platform::RenderTarget rt_output = rt_temp1;
					rt_input.setLayer(i);
					g_lightTextures[rt_input.getTexture().getHash()] = rt_input.getTexture();
					g_lightTextures[rt_temp1.getTexture().getHash()] = rt_temp1.getTexture();
					g_lightTextures[rt_temp2.getTexture().getHash()] = rt_temp2.getTexture();

					// Draw all modify shaders.
					for (uint32_t j = 0; j < scene.light.shader_modify_count; ++j)
					{
						SceneShaderPass modify;
						modify.input = { rt_input };
						modify.output = { rt_output };

						Name shader_name(scene.light.shader_modify + shadow_type + "HORIZONTAL" + (rt_input.getTexture() == shadow_map.getTexture() ? "_CUBE" : ""));
						if (g_lightShaders.find(shader_name.getHash()) == g_lightShaders.end())
							g_lightShaders[shader_name.getHash()] = asset::ShaderManager::getInstance()->get(shader_name);
						modify.shader = g_lightShaders[shader_name.getHash()];

						light_batch_faces[i].modify.push_back(modify);
						
						rt_input  = rt_output;
						rt_output = rt_output.getTexture() == rt_temp1.getTexture() ? rt_temp2 : rt_temp1;

						modify.input = { rt_input };
						modify.output = { rt_output };

						shader_name = Name(scene.light.shader_modify + shadow_type + "VERTICAL");
						if (g_lightShaders.find(shader_name.getHash()) == g_lightShaders.end())
							g_lightShaders[shader_name.getHash()] = asset::ShaderManager::getInstance()->get(shader_name);
						modify.shader = g_lightShaders[shader_name.getHash()];

						light_batch_faces[i].modify.push_back(modify);
						auto rt_temp = rt_input;
						rt_input = rt_output;
						rt_output = rt_temp;
					}

					if (scene.light.shader_modify_count > 0)
					{
						SceneShaderPass modify;
						modify.input = { rt_output };
						modify.output = { shadow_map };
						shader_name = Name(scene.light.shader_modify + "|CUBE");
						if (g_lightShaders.find(shader_name.getHash()) == g_lightShaders.end())
							g_lightShaders[shader_name.getHash()] = asset::ShaderManager::getInstance()->get(shader_name);
						modify.shader = g_lightShaders[shader_name.getHash()];
						light_batch_faces[i].modify.push_back(modify);
					}
				}

				shadow_map.setLayer(-1);
				depth_map.setLayer(-1);
			}

			Name shader_name(scene.light.shader_publish + shader_type);
			if (g_lightShaders.find(shader_name.getHash()) == g_lightShaders.end())
				g_lightShaders[shader_name.getHash()] = asset::ShaderManager::getInstance()->get(shader_name);
			light_batch.publish.shader = g_lightShaders[shader_name.getHash()];

			light_batch.publish.input  = {
				shadow_map,
				platform::RenderTarget(Name("texture"), data.texture),
				scene.post_process_manager->getTarget(Name("position")),
				scene.post_process_manager->getTarget(Name("normal")),
				scene.post_process_manager->getTarget(Name("metallic_roughness"))
			};
			light_batch.publish.output = { scene.post_process_manager->getTarget(Name("light_map")) };

			for (uint32_t i = 0; i < 6; ++i)
				light_batch.faces.push_back(light_batch_faces[i]);

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
						glm::vec3 position;
						utilities::decomposeMatrix(data.world_matrix, nullptr, nullptr, &position);
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
						light_batches.push_back(constructPoint(data.entity, scene));
						break;
					case components::LightType::kCascade:
						//constructCascade(data.entity, scene, light_batches);
						break;
					}
				}
			}

			for (auto& it : g_lightShaders)
				it.second->setKeepInMemory(true);

			return light_batches;
		}

		void renderLight(platform::IRenderer* renderer, platform::PostProcessManager& post_process_manager, const LightBatch* light_batches, uint32_t light_batch_count)
		{
			renderer->beginTimer("Lighting");

			struct CBData
			{
				glm::mat4x4 light_view_projection_matrix;
				glm::vec3 light_position;
				float     light_near;
				glm::vec3 light_direction;
				float     light_far;
				glm::vec3 light_colour;
				float     light_cut_off;
				float     light_outer_cut_off;

			} data;

			platform::IRenderBuffer* cb = renderer->allocRenderBuffer(sizeof(data), platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagTransient | platform::IRenderBuffer::kFlagDynamic, &data);
			renderer->setConstantBuffer(cb, cbPerLightIdx);


			// Prepare the light buffer.
			renderer->pushMarker("Clear Light Buffer");
			renderer->clearRenderTarget(
				post_process_manager.getTarget(Name("light_map")).getTexture(),
				glm::vec4(0.0f)
			);
			renderer->popMarker();

			for (uint32_t i = 0; i < light_batch_count; ++i)
			{
				renderer->pushMarker("Light");

				platform::IRenderBuffer* last_cb = nullptr;

				const auto& light_batch = light_batches[i];

				renderer->pushMarker("Clear");
				for (const auto& to_clear : light_batch.clear_queue_depth)
					renderer->clearRenderTarget(to_clear.getTexture(), glm::vec4(1.0f));

				for (const auto& to_clear : light_batch.clear_queue_texture)
					renderer->clearRenderTarget(to_clear.getTexture(), glm::vec4(FLT_MAX));
				renderer->popMarker();

				for (uint8_t f = 0u; f < light_batch.faces.size(); ++f)
				{
					const auto& face = light_batch.faces[f];
					data = {
						face.view_projection,
						light_batch.position,
						light_batch.near,
						face.direction,
						light_batch.far,
						light_batch.colour,
						0.0f,
						0.0f
					};
					memcpy(cb->lock(), &data, sizeof(data));
					cb->unlock();

					renderer->setUserData(glm::vec4(f, 0.0f, 0.0f, 0.0f), 0);

					// Generate shadow maps.
					if (face.generate.shader)
					{
						renderer->pushMarker("Generate");
						renderer->bindShaderPass(platform::ShaderPass(Name(""), face.generate.shader, face.generate.input, face.generate.output));

#if USE_RENDERABLES
						renderMeshes(renderer, face.renderables, platform::RasterizerState::CullMode::kNone);
#else
						renderMeshes(renderer, face.alpha, platform::RasterizerState::CullMode::kNone);
						renderMeshes(renderer, face.opaque, platform::RasterizerState::CullMode::kNone);
#endif
						renderer->popMarker();

						renderer->pushMarker("Modify");
						// Set up the post processing passes.
						renderer->setMesh(light_batch.full_screen_mesh);
						renderer->setSubMesh(0u);
						renderer->setRasterizerState(platform::RasterizerState::SolidBack());
						renderer->setBlendState(platform::BlendState::Alpha());

						// Draw all modify shaders.
						for (auto modify : face.modify)
						{
							renderer->bindShaderPass(platform::ShaderPass(Name(""), modify.shader, modify.input, modify.output));
							renderer->draw();
						}
						renderer->popMarker();
					}
				}

				renderer->pushMarker("Publish");
				// Render lights to the light map.
				// Set up the post processing passes.
				renderer->setMesh(light_batch.full_screen_mesh);
				renderer->setSubMesh(0u);
				renderer->setRasterizerState(platform::RasterizerState::SolidBack());

				// Render light using the shadow map.
				renderer->bindShaderPass(platform::ShaderPass(Name(""), light_batch.publish.shader, light_batch.publish.input, light_batch.publish.output));

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
				renderer->popMarker();

				renderer->popMarker();
			}

			// Reset states.
			renderer->setRasterizerState(platform::RasterizerState::SolidFront());
			renderer->endTimer("Lighting");
		}

		struct RenderAction_PostProcess : public scene::IRenderAction
		{
			virtual void execute(scene::Scene& scene) override
			{
				scene.renderer->setMesh(asset::MeshManager::getInstance()->getFromCache(Name("__full_screen_quad__")));
				scene.renderer->setSubMesh(0u);
				scene.renderer->setRasterizerState(platform::RasterizerState::SolidBack());
				scene.renderer->setBlendState(platform::BlendState::Default());
				scene.renderer->setDepthStencilState(platform::DepthStencilState::Default());

				scene.renderer->beginTimer("Post Processing");
				scene.renderer->pushMarker("Post Processing");
				for (auto& pass : scene.post_process_manager->getPasses())
				{
					if (pass.getEnabled())
					{
						scene.renderer->pushMarker(pass.getName().getName());
						scene.renderer->bindShaderPass(pass);

						scene.renderer->draw();
						scene.renderer->popMarker();
					}
					else
					{
						scene.renderer->setMarker(pass.getName().getName() + " - Disabled");
					}
				}
				scene.renderer->popMarker();
				scene.renderer->endTimer("Post Processing");
			}

			virtual ~RenderAction_PostProcess() override {};
		};

		struct RenderAction_DebugRenderer : public scene::IRenderAction
		{
			virtual void execute(scene::Scene& scene) override
			{
				scene.renderer->beginTimer("Debug Renderer");
				scene.renderer->pushMarker("Debug Renderer");
				scene.debug_renderer.Render(scene);
				scene.renderer->popMarker();
				scene.renderer->endTimer("Debug Renderer");
			}

			virtual ~RenderAction_DebugRenderer() override {};
		};

		struct RenderAction_MeshRenderDebug : public scene::IRenderAction
		{
			virtual void execute(scene::Scene& scene) override
			{
				scene.renderer->beginTimer("Mesh Renderer Debug");
				scene.renderer->pushMarker("Mesh Renderer Debug");
				scene.mesh_render.static_bvh->draw(&scene.debug_renderer);
				scene.mesh_render.dynamic_bvh->draw(&scene.debug_renderer);
				scene.renderer->popMarker();
				scene.renderer->endTimer("Mesh Renderer Debug");
			}

			virtual ~RenderAction_MeshRenderDebug() override {};
		};

		struct RenderAction_RigidBodyRenderer : public scene::IRenderAction
		{
			virtual void execute(scene::Scene& scene) override
			{
				scene.renderer->beginTimer("RigidBody Renderer");
				scene.renderer->pushMarker("RigidBody Renderer");
				scene.rigid_body.physics_world->render(scene);
				scene.renderer->popMarker();
				scene.renderer->endTimer("RigidBody Renderer");
			}

			virtual ~RenderAction_RigidBodyRenderer() override {};
		};

		struct RenderAction_CopyToScreen : public scene::IRenderAction
		{
			virtual void execute(scene::Scene& scene) override
			{
				scene.renderer->beginTimer("Copy To Screen");
				scene.renderer->pushMarker("Copy To Screen");

				scene.renderer->setBlendState(platform::BlendState::Alpha());

				scene.renderer->copyToScreen(
					scene.post_process_manager->getTarget(
						scene.post_process_manager->getFinalTarget()
					).getTexture()
				);

				if (scene.gui->getEnabled())
					scene.renderer->copyToScreen(scene.post_process_manager->getTarget(Name("gui")).getTexture());

				scene.renderer->popMarker();
				scene.renderer->endTimer("Copy To Screen");
			}

			virtual ~RenderAction_CopyToScreen() override {};
		};

		///////////////////////////////////////////////////////////////////////////
		void flush(scene::Scene& scene, const CameraBatch& camera_batch, const Vector<LightBatch>& light_batches)
		{
			scene.renderer->setOverrideScene(&scene);
			scene.renderer->startFrame();

			renderCamera(scene.renderer, camera_batch);
			renderLight(scene.renderer, *scene.post_process_manager, light_batches.data(), (uint32_t)light_batches.size());

			scene.gui->render(scene);

			for (auto render_action : scene.render_actions)
			{
				render_action->execute(scene);
				render_action->~IRenderAction();
			}

			scene.renderer->endFrame();
			scene.renderer->setOverrideScene(nullptr);
		}

		///////////////////////////////////////////////////////////////////////////
		void construct(scene::Scene& scene, CameraBatch& camera_batch, Vector<LightBatch>& light_batches)
		{
			//Add all render actions.
			Vector<IRenderAction*> render_actions;
			render_actions.push_back(foundation::GetFrameHeap()->construct<RenderAction_PostProcess>());
			render_actions.push_back(foundation::GetFrameHeap()->construct<RenderAction_RigidBodyRenderer>());
			//render_actions.push_back(foundation::GetFrameHeap()->construct<RenderAction_MeshRenderDebug>());
			render_actions.push_back(foundation::GetFrameHeap()->construct<RenderAction_DebugRenderer>());
			render_actions.push_back(foundation::GetFrameHeap()->construct<RenderAction_CopyToScreen>());
			render_actions.insert(render_actions.end(), scene.render_actions.begin(), scene.render_actions.end());
			scene.render_actions = eastl::move(render_actions);

			camera_batch = constructCamera(scene, scene.camera.main_camera);
			light_batches = constructLight(camera_batch, scene);
		}

#if USE_MT
		struct QueueFlushData
		{
			std::atomic<bool> done = true;
			Scene scene;
			CameraBatch camera_batch;
			Vector<LightBatch> light_batches;
		} k_queue_flush_data;

		void queueFlush(void* user_data)
		{
			QueueFlushData& qfd = *(QueueFlushData*)user_data;
			flush(qfd.scene, qfd.camera_batch, qfd.light_batches);
			qfd.done = true;
		}
#endif

		void sceneConstructRender(scene::Scene& scene)
		{
			components::MeshRenderSystem::updateDynamicsBvh(scene);
			components::LightSystem::updateLightTransforms(scene);
			components::CameraSystem::updateCameraTransforms(scene);

#if USE_MT
			while (!k_queue_flush_data.done)
				std::this_thread::sleep_for(std::chrono::microseconds(1));

			construct(scene, k_queue_flush_data.camera_batch, k_queue_flush_data.light_batches);
			k_queue_flush_data.scene.renderer                 = scene.renderer;
			k_queue_flush_data.scene.post_process_manager     = scene.post_process_manager;
			k_queue_flush_data.scene.window                   = scene.window;
			k_queue_flush_data.scene.gui                      = scene.gui;
			k_queue_flush_data.scene.render_actions           = scene.render_actions;
			k_queue_flush_data.scene.rigid_body.physics_world = scene.rigid_body.physics_world;
			k_queue_flush_data.scene.debug_renderer           = scene.debug_renderer;
			k_queue_flush_data.scene.mesh_render.static_bvh   = scene.mesh_render.static_bvh;
			k_queue_flush_data.scene.mesh_render.dynamic_bvh  = scene.mesh_render.dynamic_bvh;

			k_queue_flush_data.done = false;

			platform::TaskScheduler::queue(queueFlush, &k_queue_flush_data, platform::TaskScheduler::kCritical);
#else
			// Create new.

			CameraBatch camera_batch;
			Vector<LightBatch> light_batches;
			construct(scene, camera_batch, light_batches);
			flush(scene, camera_batch, light_batches);
#endif

			scene.render_actions.clear();
			scene.debug_renderer.Clear();
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

			if (scene.do_serialize)
			{
				scene.do_serialize = false;
				sceneSerialize(scene);
				sceneCollectGarbage(scene);
			}

			if (scene.do_deserialize)
			{
				scene.do_deserialize = false;
				sceneDeserialize(scene);
				sceneCollectGarbage(scene);
			}
		}
		void sceneDeinitialize(scene::Scene& scene)
		{
#if USE_MT
			while (!k_queue_flush_data.done)
				std::this_thread::sleep_for(std::chrono::microseconds(1));
#endif
			
			components::ColliderSystem::deinitialize(scene);
			components::RigidBodySystem::deinitialize(scene);
			components::NameSystem::deinitialize(scene);
			components::LODSystem::deinitialize(scene);
			components::CameraSystem::deinitialize(scene);
			components::MeshRenderSystem::deinitialize(scene);
			components::TransformSystem::deinitialize(scene);
			components::MonoBehaviourSystem::deinitialize(scene);
			components::WaveSourceSystem::deinitialize(scene);
			components::LightSystem::deinitialize(scene);
		}
		Serializer k_serializer;
		void sceneSerialize(scene::Scene& scene)
		{
			k_serializer = Serializer();
			components::EntitySystem::serialize(scene, k_serializer);
			components::NameSystem::serialize(scene, k_serializer);
			components::LODSystem::serialize(scene, k_serializer);
			components::CameraSystem::serialize(scene, k_serializer);
			components::RigidBodySystem::serialize(scene, k_serializer);
			components::ColliderSystem::serialize(scene, k_serializer);
			components::MonoBehaviourSystem::serialize(scene, k_serializer);
			components::WaveSourceSystem::serialize(scene, k_serializer);
			components::LightSystem::serialize(scene, k_serializer);
			components::TransformSystem::serialize(scene, k_serializer);
			components::MeshRenderSystem::serialize(scene, k_serializer);
		}
		void sceneDeserialize(scene::Scene& scene)
		{
			components::EntitySystem::deserialize(scene, k_serializer);
			components::NameSystem::deserialize(scene, k_serializer);
			components::LODSystem::deserialize(scene, k_serializer);
			components::CameraSystem::deserialize(scene, k_serializer);
			components::RigidBodySystem::deserialize(scene, k_serializer);
			components::ColliderSystem::deserialize(scene, k_serializer);
			components::MonoBehaviourSystem::deserialize(scene, k_serializer);
			components::WaveSourceSystem::deserialize(scene, k_serializer);
			components::LightSystem::deserialize(scene, k_serializer);
			components::TransformSystem::deserialize(scene, k_serializer);
			components::MeshRenderSystem::deserialize(scene, k_serializer);
		}
	}
}