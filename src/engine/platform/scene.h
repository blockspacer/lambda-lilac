#pragma once
#include <systems/entity_system.h>
#include <systems/name_system.h>
#include <systems/lod_system.h>
#include <systems/rigid_body_system.h>
#include <systems/collider_system.h>
#include <systems/mono_behaviour_system.h>
#include <systems/wave_source_system.h>
#include <systems/light_system.h>
#include <systems/camera_system.h>
#include <systems/transform_system.h>
#include <systems/mesh_render_system.h>
#include <platform/shader_variable_manager.h>
#include <platform/post_process_manager.h>
#include <platform/debug_renderer.h>
#include <interfaces/iscript_context.h>

namespace lambda
{
	namespace gui
	{
		class GUI;
	}

	namespace scene
	{
		///////////////////////////////////////////////////////////////////////////
		struct Scene
		{
			components::EntitySystem::SystemData        entity;
			components::NameSystem::SystemData          name;
			components::LODSystem::SystemData           lod;
			components::CameraSystem::SystemData        camera;
			components::RigidBodySystem::SystemData     rigid_body;
			components::ColliderSystem::SystemData      collider;
			components::MonoBehaviourSystem::SystemData mono_behaviour;
			components::WaveSourceSystem::SystemData    wave_source;
			components::LightSystem::SystemData         light;
			components::TransformSystem::SystemData     transform;
			components::MeshRenderSystem::SystemData    mesh_render;
			platform::ShaderVariableManager shader_variable_manager;
			platform::DebugRenderer         debug_renderer;
			platform::PostProcessManager    post_process_manager;
			scripting::IScriptContext* scripting = nullptr;
			platform::IRenderer*       renderer  = nullptr;
			platform::IWindow*         window    = nullptr;
			gui::GUI*                  gui       = nullptr;
		};

		///////////////////////////////////////////////////////////////////////////
		void sceneInitialize(scene::Scene& scene);
		void sceneUpdate(const float& delta_time, scene::Scene& scene);
		void sceneFixedUpdate(const float& delta_time, scene::Scene& scene);
		void sceneConstructRender(scene::Scene& scene);
		void sceneOnRender(scene::Scene& scene);
		void sceneCollectGarbage(scene::Scene& scene);
		void sceneDeinitialize(scene::Scene& scene);
	}
}