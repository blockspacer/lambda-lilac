#pragma once
#include <Meta.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <systems/wave_source_system.h>
#include <systems/light_system.h>
#include <systems/camera_system.h>
#include <systems/lod_system.h>
#include <systems/collider_system.h>
#include <systems/mesh_render_system.h>
#include <systems/mono_behaviour_system.h>
#include <systems/name_system.h>
#include <systems/rigid_body_system.h>
#include <systems/transform_system.h>
#include <platform/render_target.h>
#include <platform/shader_pass.h>
#include <utils/angle.h>
#include <utils/distance.h>

namespace meta
{
	template <>
	inline auto registerMembers<glm::vec2>()
	{
		return members(
			member("x", &glm::vec2::x),
			member("y", &glm::vec2::y)
		);
	}

	template <>
	inline auto registerMembers<glm::vec3>()
	{
		return members(
			member("x", &glm::vec3::x),
			member("y", &glm::vec3::y),
			member("z", &glm::vec3::z)
		);
	}

	template <>
	inline auto registerMembers<glm::vec4>()
	{
		return members(
			member("x", &glm::vec4::x),
			member("y", &glm::vec4::y),
			member("z", &glm::vec4::z),
			member("w", &glm::vec4::w)
		);
	}

	template <>
	inline auto registerMembers<glm::quat>()
	{
		return members(
			member("x", &glm::quat::x),
			member("y", &glm::quat::y),
			member("z", &glm::quat::z),
			member("w", &glm::quat::w)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::WaveSourceSystem::Data>()
	{
		return members(
			member("entity", &lambda::components::WaveSourceSystem::Data::entity),
			member("state", &lambda::components::WaveSourceSystem::Data::state),
			member("buffer", &lambda::components::WaveSourceSystem::Data::buffer),
			member("handle", &lambda::components::WaveSourceSystem::Data::handle),
			member("in_world", &lambda::components::WaveSourceSystem::Data::in_world),
			member("loop", &lambda::components::WaveSourceSystem::Data::loop),
			member("gain", &lambda::components::WaveSourceSystem::Data::gain),
			member("pitch", &lambda::components::WaveSourceSystem::Data::pitch),
			member("radius", &lambda::components::WaveSourceSystem::Data::radius),
			member("last_position", &lambda::components::WaveSourceSystem::Data::last_position),
			member("valid", &lambda::components::WaveSourceSystem::Data::valid)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::WaveSourceSystem::SystemData>()
	{
		return members(
			member("entity_to_data", &lambda::components::WaveSourceSystem::SystemData::entity_to_data),
			member("data_to_entity", &lambda::components::WaveSourceSystem::SystemData::data_to_entity),
			member("marked_for_delete", &lambda::components::WaveSourceSystem::SystemData::marked_for_delete),
			member("unused_data_entries", &lambda::components::WaveSourceSystem::SystemData::unused_data_entries),
			member("listener", &lambda::components::WaveSourceSystem::SystemData::listener),
			member("last_listener_position", &lambda::components::WaveSourceSystem::SystemData::last_listener_position),
			member("data", &lambda::components::WaveSourceSystem::SystemData::data)
		);
	}

	template <>
	inline auto registerMembers<lambda::platform::ShaderPass>()
	{
		return members(
			member("enabled", &lambda::platform::ShaderPass::getEnabled, &lambda::platform::ShaderPass::setEnabled),
			member("name", &lambda::platform::ShaderPass::getName, &lambda::platform::ShaderPass::metaSetName),
			member("shader", &lambda::platform::ShaderPass::getShader, &lambda::platform::ShaderPass::metaSetShader),
			member("inputs", &lambda::platform::ShaderPass::getInputs, &lambda::platform::ShaderPass::metaSetInputs),
			member("outputs", &lambda::platform::ShaderPass::getOutputs, &lambda::platform::ShaderPass::metaSetOutputs)
		);
	}

	template <>
	inline auto registerMembers<lambda::platform::RenderTarget>()
	{
		return members(
			member("render_scale", &lambda::platform::RenderTarget::getRenderScale, &lambda::platform::RenderTarget::metaSetRenderScale),
			member("texture", &lambda::platform::RenderTarget::getTexture, &lambda::platform::RenderTarget::metaSetTexture),
			member("name", &lambda::platform::RenderTarget::getName, &lambda::platform::RenderTarget::metaSetName),
			member("mip_map", &lambda::platform::RenderTarget::getMipMap, &lambda::platform::RenderTarget::setMipMap),
			member("layer", &lambda::platform::RenderTarget::getLayer, &lambda::platform::RenderTarget::setLayer),
			member("is_back_buffer", &lambda::platform::RenderTarget::isBackBuffer, &lambda::platform::RenderTarget::metaSetIsBackBuffer),
			member("from_texture", &lambda::platform::RenderTarget::fromTexture, &lambda::platform::RenderTarget::metaSetFromTexture)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::LightSystem::Data>()
	{
		return members(
			member("type", &lambda::components::LightSystem::Data::type),
			member("shadow_type", &lambda::components::LightSystem::Data::shadow_type),
			member("colour", &lambda::components::LightSystem::Data::colour),
			member("ambient", &lambda::components::LightSystem::Data::ambient),
			member("intensity", &lambda::components::LightSystem::Data::intensity),
			member("texture", &lambda::components::LightSystem::Data::texture),
			member("culler", &lambda::components::LightSystem::Data::culler),
			member("entity", &lambda::components::LightSystem::Data::entity),
			member("cut_off", &lambda::components::LightSystem::Data::cut_off),
			member("outer_cut_off", &lambda::components::LightSystem::Data::outer_cut_off),
			member("size", &lambda::components::LightSystem::Data::size),
			member("enabled", &lambda::components::LightSystem::Data::enabled),
			member("rsm", &lambda::components::LightSystem::Data::rsm),
			member("dynamic_frequency", &lambda::components::LightSystem::Data::dynamic_frequency),
			member("dynamic_index", &lambda::components::LightSystem::Data::dynamic_index),
			member("valid", &lambda::components::LightSystem::Data::valid),
			member("depth", &lambda::components::LightSystem::Data::depth),
			member("projection", &lambda::components::LightSystem::Data::projection),
			member("view", &lambda::components::LightSystem::Data::view),
			member("view_position", &lambda::components::LightSystem::Data::view_position),
			member("shadow_map_size_px", &lambda::components::LightSystem::Data::shadow_map_size_px),
			member("render_target_texture", &lambda::components::LightSystem::Data::render_target_texture),
			member("depth_target_texture", &lambda::components::LightSystem::Data::depth_target_texture),
			member("render_target", &lambda::components::LightSystem::Data::render_target),
			member("depth_target", &lambda::components::LightSystem::Data::depth_target),
			member("world_matrix", &lambda::components::LightSystem::Data::world_matrix)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::LightSystem::SystemData>()
	{
		return members(
			member("data", &lambda::components::LightSystem::SystemData::data),
			member("entity_to_data", &lambda::components::LightSystem::SystemData::entity_to_data),
			member("data_to_entity", &lambda::components::LightSystem::SystemData::data_to_entity),
			member("marked_for_delete", &lambda::components::LightSystem::SystemData::marked_for_delete),
			member("unused_data_entries", &lambda::components::LightSystem::SystemData::unused_data_entries),
			member("full_screen_mesh", &lambda::components::LightSystem::SystemData::full_screen_mesh),
			member("shader_generate", &lambda::components::LightSystem::SystemData::shader_generate),
			member("shader_modify", &lambda::components::LightSystem::SystemData::shader_modify),
			member("shader_modify_count", &lambda::components::LightSystem::SystemData::shader_modify_count),
			member("shader_publish", &lambda::components::LightSystem::SystemData::shader_publish),
			member("shader_shadow_type", &lambda::components::LightSystem::SystemData::shader_shadow_type),
			member("default_shadow_map", &lambda::components::LightSystem::SystemData::default_shadow_map),
			member("default_texture", &lambda::components::LightSystem::SystemData::default_texture)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::CameraSystem::Data>()
	{
		return members(
			member("fov", &lambda::components::CameraSystem::Data::fov),
			member("near_plane", &lambda::components::CameraSystem::Data::near_plane),
			member("far_plane", &lambda::components::CameraSystem::Data::far_plane),
			member("shader_passes", &lambda::components::CameraSystem::Data::shader_passes),
			member("world_matrix", &lambda::components::CameraSystem::Data::world_matrix),
			member("entity", &lambda::components::CameraSystem::Data::entity),
			member("valid", &lambda::components::CameraSystem::Data::valid)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::CameraSystem::SystemData>()
	{
		return members(
			member("data", &lambda::components::CameraSystem::SystemData::data),
			member("entity_to_data", &lambda::components::CameraSystem::SystemData::entity_to_data),
			member("data_to_entity", &lambda::components::CameraSystem::SystemData::data_to_entity),
			member("marked_for_delete", &lambda::components::CameraSystem::SystemData::marked_for_delete),
			member("unused_data_entries", &lambda::components::CameraSystem::SystemData::unused_data_entries),
			member("main_camera", &lambda::components::CameraSystem::SystemData::main_camera)
		);
	}

	template <>
	inline auto registerMembers<lambda::utilities::Angle>()
	{
		return members(
			member("rad", &lambda::utilities::Angle::asRad, &lambda::utilities::Angle::setRad)
		);
	}

	template <>
	inline auto registerMembers<lambda::utilities::Distance>()
	{
		return members(
			member("meter", &lambda::utilities::Distance::asMeter, &lambda::utilities::Distance::setMeter)
		);
	}

	template <>
	inline auto registerMembers<lambda::asset::VioletHandle<lambda::asset::Mesh>>()
	{
		return members(
			member("from_name", &lambda::asset::VioletHandle<lambda::asset::Mesh>::metaGet, &lambda::asset::VioletHandle<lambda::asset::Mesh>::metaSet)
		);
	}

	template <>
	inline auto registerMembers<lambda::asset::VioletHandle<lambda::asset::Shader>>()
	{
		return members(
			member("from_name", &lambda::asset::VioletHandle<lambda::asset::Shader>::metaGet, &lambda::asset::VioletHandle<lambda::asset::Shader>::metaSet)
		);
	}

	template <>
	inline auto registerMembers<lambda::asset::VioletHandle<lambda::asset::Texture>>()
	{
		return members(
			member("from_name", &lambda::asset::VioletHandle<lambda::asset::Texture>::metaGet, &lambda::asset::VioletHandle<lambda::asset::Texture>::metaSet)
		);
	}

	template <>
	inline auto registerMembers<lambda::asset::VioletHandle<lambda::asset::Wave>>()
	{
		return members(
			member("from_name", &lambda::asset::VioletHandle<lambda::asset::Wave>::metaGet, &lambda::asset::VioletHandle<lambda::asset::Wave>::metaSet)
		);
	}

	template <>
	inline auto registerMembers<lambda::Name>()
	{
		return members(
			member("name", &lambda::Name::name_),
			member("hash", &lambda::Name::hash_)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::TransformSystem::Data>()
	{
		return members(
			member("children", &lambda::components::TransformSystem::Data::children),
			member("entity", &lambda::components::TransformSystem::Data::entity),
			member("translation", &lambda::components::TransformSystem::Data::translation),
			member("rotation", &lambda::components::TransformSystem::Data::rotation),
			member("scale", &lambda::components::TransformSystem::Data::scale),
			member("local", &lambda::components::TransformSystem::Data::local),
			member("world", &lambda::components::TransformSystem::Data::world),
			member("dirty", &lambda::components::TransformSystem::Data::dirty),
			member("valid", &lambda::components::TransformSystem::Data::valid),
			member("parent", &lambda::components::TransformSystem::Data::parent)
		);
	}
	template <>
	inline auto registerMembers<lambda::components::TransformSystem::SystemData>()
	{
		return members(
			member("data", &lambda::components::TransformSystem::SystemData::data),
			member("entity_to_data", &lambda::components::TransformSystem::SystemData::entity_to_data),
			member("data_to_entity", &lambda::components::TransformSystem::SystemData::data_to_entity),
			member("marked_for_delete", &lambda::components::TransformSystem::SystemData::marked_for_delete),
			member("unused_data_entries", &lambda::components::TransformSystem::SystemData::unused_data_entries)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::RigidBodySystem::Data>()
	{
		return members(
			member("valid", &lambda::components::RigidBodySystem::Data::valid),
			member("entity", &lambda::components::RigidBodySystem::Data::entity)
		);
	}
	template <>
	inline auto registerMembers<lambda::components::RigidBodySystem::SystemData>()
	{
		return members(
			member("data", &lambda::components::RigidBodySystem::SystemData::data),
			member("entity_to_data", &lambda::components::RigidBodySystem::SystemData::entity_to_data),
			member("data_to_entity", &lambda::components::RigidBodySystem::SystemData::data_to_entity),
			member("marked_for_delete", &lambda::components::RigidBodySystem::SystemData::marked_for_delete),
			member("unused_data_entries", &lambda::components::RigidBodySystem::SystemData::unused_data_entries)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::NameSystem::Data>()
	{
		return members(
			member("name", &lambda::components::NameSystem::Data::name),
			member("tags", &lambda::components::NameSystem::Data::tags),
			member("entity", &lambda::components::NameSystem::Data::entity),
			member("valid", &lambda::components::NameSystem::Data::valid)
		);
	}
	template <>
	inline auto registerMembers<lambda::components::NameSystem::SystemData>()
	{
		return members(
			member("data", &lambda::components::NameSystem::SystemData::data),
			member("entity_to_data", &lambda::components::NameSystem::SystemData::entity_to_data),
			member("data_to_entity", &lambda::components::NameSystem::SystemData::data_to_entity),
			member("marked_for_delete", &lambda::components::NameSystem::SystemData::marked_for_delete),
			member("unused_data_entries", &lambda::components::NameSystem::SystemData::unused_data_entries)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::MonoBehaviourSystem::Data>()
	{
		return members(
			member("object", &lambda::components::MonoBehaviourSystem::Data::object),
			member("initialize", &lambda::components::MonoBehaviourSystem::Data::initialize),
			member("deinitialize", &lambda::components::MonoBehaviourSystem::Data::deinitialize),
			member("update", &lambda::components::MonoBehaviourSystem::Data::update),
			member("fixed_update", &lambda::components::MonoBehaviourSystem::Data::fixed_update),
			member("on_collision_enter", &lambda::components::MonoBehaviourSystem::Data::on_collision_enter),
			member("on_collision_exit", &lambda::components::MonoBehaviourSystem::Data::on_collision_exit),
			member("on_trigger_enter", &lambda::components::MonoBehaviourSystem::Data::on_trigger_enter),
			member("on_trigger_exit", &lambda::components::MonoBehaviourSystem::Data::on_trigger_exit),
			member("valid", &lambda::components::MonoBehaviourSystem::Data::valid),
			member("entity", &lambda::components::MonoBehaviourSystem::Data::entity)
		);
	}
	template <>
	inline auto registerMembers<lambda::components::MonoBehaviourSystem::SystemData>()
	{
		return members(
			member("data", &lambda::components::MonoBehaviourSystem::SystemData::data),
			member("entity_to_data", &lambda::components::MonoBehaviourSystem::SystemData::entity_to_data),
			member("data_to_entity", &lambda::components::MonoBehaviourSystem::SystemData::data_to_entity),
			member("marked_for_delete", &lambda::components::MonoBehaviourSystem::SystemData::marked_for_delete),
			member("unused_data_entries", &lambda::components::MonoBehaviourSystem::SystemData::unused_data_entries)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::MeshRenderSystem::Data>()
	{
		return members(
			member("mesh", &lambda::components::MeshRenderSystem::Data::mesh),
			member("sub_mesh", &lambda::components::MeshRenderSystem::Data::sub_mesh),
			member("albedo_texture", &lambda::components::MeshRenderSystem::Data::albedo_texture),
			member("normal_texture", &lambda::components::MeshRenderSystem::Data::normal_texture),
			member("dmra_texture", &lambda::components::MeshRenderSystem::Data::dmra_texture),
			member("emissive_texture", &lambda::components::MeshRenderSystem::Data::emissive_texture),
			member("metallicness", &lambda::components::MeshRenderSystem::Data::metallicness),
			member("roughness", &lambda::components::MeshRenderSystem::Data::roughness),
			member("emissiveness", &lambda::components::MeshRenderSystem::Data::emissiveness),
			member("visible", &lambda::components::MeshRenderSystem::Data::visible),
			member("cast_shadows", &lambda::components::MeshRenderSystem::Data::cast_shadows),
			member("valid", &lambda::components::MeshRenderSystem::Data::valid),
			member("entity", &lambda::components::MeshRenderSystem::Data::entity),
			member("renderable", &lambda::components::MeshRenderSystem::Data::renderable)
		);
	}
	template <>
	inline auto registerMembers<lambda::components::MeshRenderSystem::SystemData>()
	{
		return members(
			member("data", &lambda::components::MeshRenderSystem::SystemData::data),
			member("entity_to_data", &lambda::components::MeshRenderSystem::SystemData::entity_to_data),
			member("data_to_entity", &lambda::components::MeshRenderSystem::SystemData::data_to_entity),
			member("marked_for_delete", &lambda::components::MeshRenderSystem::SystemData::marked_for_delete),
			member("unused_data_entries", &lambda::components::MeshRenderSystem::SystemData::unused_data_entries),
			member("dynamic_renderables", &lambda::components::MeshRenderSystem::SystemData::dynamic_renderables),
			member("static_renderables", &lambda::components::MeshRenderSystem::SystemData::static_renderables),
			member("default_albedo", &lambda::components::MeshRenderSystem::SystemData::default_albedo),
			member("default_normal", &lambda::components::MeshRenderSystem::SystemData::default_normal),
			member("default_dmra", &lambda::components::MeshRenderSystem::SystemData::default_dmra),
			member("default_emissive", &lambda::components::MeshRenderSystem::SystemData::default_emissive)
		);
	}
	template <>
	inline auto registerMembers<lambda::utilities::Renderable>()
	{
		return members(
			member("entity", &lambda::utilities::Renderable::entity),
			member("model_matrix", &lambda::utilities::Renderable::model_matrix),
			member("mesh", &lambda::utilities::Renderable::mesh),
			member("sub_mesh", &lambda::utilities::Renderable::sub_mesh),
			member("albedo_texture", &lambda::utilities::Renderable::albedo_texture),
			member("normal_texture", &lambda::utilities::Renderable::normal_texture),
			member("dmra_texture", &lambda::utilities::Renderable::dmra_texture),
			member("emissive_texture", &lambda::utilities::Renderable::emissive_texture),
			member("metallicness", &lambda::utilities::Renderable::metallicness),
			member("roughness", &lambda::utilities::Renderable::roughness),
			member("emissiveness", &lambda::utilities::Renderable::emissiveness),
			member("min", &lambda::utilities::Renderable::min),
			member("max", &lambda::utilities::Renderable::max),
			member("center", &lambda::utilities::Renderable::center),
			member("radius", &lambda::utilities::Renderable::radius)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::LOD>()
	{
		return members(
			member("distance", &lambda::components::LOD::getDistance, &lambda::components::LOD::setDistance),
			member("mesh", &lambda::components::LOD::getMesh, &lambda::components::LOD::setMesh)
		);
	}
	template <>
	inline auto registerMembers<lambda::components::LODSystem::Data>()
	{
		return members(
			member("lods", &lambda::components::LODSystem::Data::lods),
			member("base_lod", &lambda::components::LODSystem::Data::base_lod),
			member("entity", &lambda::components::LODSystem::Data::entity),
			member("valid", &lambda::components::LODSystem::Data::valid)
		);
	}
	template <>
	inline auto registerMembers<lambda::components::LODSystem::SystemData>()
	{
		return members(
			member("data", &lambda::components::LODSystem::SystemData::data),
			member("entity_to_data", &lambda::components::LODSystem::SystemData::entity_to_data),
			member("data_to_entity", &lambda::components::LODSystem::SystemData::data_to_entity),
			member("marked_for_delete", &lambda::components::LODSystem::SystemData::marked_for_delete),
			member("unused_data_entries", &lambda::components::LODSystem::SystemData::unused_data_entries)
		);
	}

	template <>
	inline auto registerMembers<lambda::components::ColliderSystem::Data>()
	{
		return members(
			member("type", &lambda::components::ColliderSystem::Data::type),
			member("is_trigger", &lambda::components::ColliderSystem::Data::is_trigger),
			member("valid", &lambda::components::ColliderSystem::Data::valid),
			member("entity", &lambda::components::ColliderSystem::Data::entity)
		);
	}
	template <>
	inline auto registerMembers<lambda::components::ColliderSystem::SystemData>()
	{
		return members(
			member("data", &lambda::components::ColliderSystem::SystemData::data),
			member("entity_to_data", &lambda::components::ColliderSystem::SystemData::entity_to_data),
			member("data_to_entity", &lambda::components::ColliderSystem::SystemData::data_to_entity),
			member("marked_for_delete", &lambda::components::ColliderSystem::SystemData::marked_for_delete),
			member("unused_data_entries", &lambda::components::ColliderSystem::SystemData::unused_data_entries)
		);
	}
	/*template <>
	inline auto registerMembers<lambda::physics::ICollisionBody>()
	{
	return members(
	member("angular_constraints", &lambda::physics::ICollisionBody::getAngularConstraints, &lambda::physics::ICollisionBody::setAngularConstraints),
	member("angular_velocity", &lambda::physics::ICollisionBody::getAngularVelocity, &lambda::physics::ICollisionBody::setAngularVelocity),
	member("entity", &lambda::physics::ICollisionBody::getEntity, &lambda::physics::ICollisionBody::setEntity),
	member("friction", &lambda::physics::ICollisionBody::getFriction, &lambda::physics::ICollisionBody::setFriction),
	member("layers", &lambda::physics::ICollisionBody::getLayers, &lambda::physics::ICollisionBody::setLayers),
	member("mass", &lambda::physics::ICollisionBody::getMass, &lambda::physics::ICollisionBody::setMass),
	member("position", &lambda::physics::ICollisionBody::getPosition, &lambda::physics::ICollisionBody::setPosition),
	member("rotation", &lambda::physics::ICollisionBody::getRotation, &lambda::physics::ICollisionBody::setRotation),
	member("velocity", &lambda::physics::ICollisionBody::getVelocity, &lambda::physics::ICollisionBody::setVelocity),
	member("velocity_constraints", &lambda::physics::ICollisionBody::getVelocityConstraints, &lambda::physics::ICollisionBody::setVelocityConstraints)
	);
	}*/
}
