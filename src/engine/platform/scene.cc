#include "scene.h"
#include "systems/light_system.h"
#include "systems/wave_source_system.h"

#include "platform/depth_stencil_state.h"
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

#define USE_MT 0

#if USE_MT
		std::atomic<int> k_can_read;
		std::atomic<int> k_can_write;
		scene::Scene k_scene;

		///////////////////////////////////////////////////////////////////////////
		void flush()
		{
			while (true)
			{
				while (k_can_read.load() == 0) {
					std::this_thread::sleep_for(std::chrono::microseconds(1u));
				}

				k_can_read = 0;
				k_scene.renderer->setOverrideScene(&k_scene);
				k_scene.renderer->startFrame();

				k_scene.gui->render(k_scene);

				components::CameraSystem::onRender(k_scene);
				components::LightSystem::onRender(k_scene);

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
				std::this_thread::sleep_for(std::chrono::microseconds(1u));
			}

			k_can_write = 0;

			// Required by other things..
			k_scene.camera                  = scene.camera;
			k_scene.light                   = scene.light;
			k_scene.transform               = scene.transform;
			k_scene.mesh_render             = scene.mesh_render;
			k_scene.debug_renderer          = scene.debug_renderer;
			
			// Required by flush.
			k_scene.renderer                = scene.renderer;
			// Required by the renderer.
			k_scene.post_process_manager    = scene.post_process_manager;
			k_scene.shader_variable_manager = scene.shader_variable_manager;
			k_scene.window                  = scene.window;
			k_scene.gui                     = scene.gui;

			scene.debug_renderer.Clear();

			k_can_read = 1;
#else
			scene.renderer->setOverrideScene(&scene);
			scene.renderer->startFrame();

			scene.gui->render(scene);

			components::CameraSystem::onRender(scene);
			components::LightSystem::onRender(scene);

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