#pragma once
#include <rapidjson/document.h>
#include <containers/containers.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <utils/angle.h>
#include <utils/distance.h>
#include <platform/shader_pass.h>
#include <platform/render_target.h>
#include <systems/light_system.h>
#include <systems/wave_source_system.h>
#pragma warning(disable: 4503)
#pragma warning(disable: 4396)

#if VIOLET_PHYSICS_REACT
#include "physics/react/react_physics_world.h"
#endif

namespace lambda
{
	namespace utilities
	{
		inline float  validate(const float&  v) { return (std::isnan(v) || std::isinf(v)) ? 0.0f : v; }
		inline double validate(const double& v) { return (std::isnan(v) || std::isinf(v)) ? 0.0f : v; }

		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const bool&     t) { return rapidjson::Value(t); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const float&    t) { return rapidjson::Value(validate(t)); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const double&   t) { return rapidjson::Value(validate(t)); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const String&   t) { return rapidjson::Value(rapidjson::StringRef(t.c_str())); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const uint8_t&  t) { return rapidjson::Value(t); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const uint16_t& t) { return rapidjson::Value(t); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const uint32_t& t) { return rapidjson::Value(t); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const uint64_t& t) { return rapidjson::Value(t); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const int8_t&   t) { return rapidjson::Value(t); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const int16_t&  t) { return rapidjson::Value(t); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const int32_t&  t) { return rapidjson::Value(t); }
		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const int64_t&  t) { return rapidjson::Value(t); }

		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const glm::vec2& t)
		{
			rapidjson::Value val(rapidjson::kArrayType);
			val.PushBack(validate(t.x), doc.GetAllocator());
			val.PushBack(validate(t.y), doc.GetAllocator());
			return val;
		}

		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const glm::vec3& t)
		{
			rapidjson::Value val(rapidjson::kArrayType);
			val.PushBack(validate(t.x), doc.GetAllocator());
			val.PushBack(validate(t.y), doc.GetAllocator());
			val.PushBack(validate(t.z), doc.GetAllocator());
			return val;
		}

		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const glm::vec4& t)
		{
			rapidjson::Value val(rapidjson::kArrayType);
			val.PushBack(validate(t.x), doc.GetAllocator());
			val.PushBack(validate(t.y), doc.GetAllocator());
			val.PushBack(validate(t.z), doc.GetAllocator());
			val.PushBack(validate(t.w), doc.GetAllocator());
			return val;
		}

		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const glm::quat& t)
		{
			rapidjson::Value val(rapidjson::kArrayType);
			val.PushBack(validate(t.x), doc.GetAllocator());
			val.PushBack(validate(t.y), doc.GetAllocator());
			val.PushBack(validate(t.z), doc.GetAllocator());
			val.PushBack(validate(t.w), doc.GetAllocator());
			return val;
		}

		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const glm::mat3x3& t)
		{
			rapidjson::Value val(rapidjson::kArrayType);
			val.PushBack(validate(t[0][0]), doc.GetAllocator());
			val.PushBack(validate(t[0][1]), doc.GetAllocator());
			val.PushBack(validate(t[0][2]), doc.GetAllocator());
			val.PushBack(validate(t[1][0]), doc.GetAllocator());
			val.PushBack(validate(t[1][1]), doc.GetAllocator());
			val.PushBack(validate(t[1][2]), doc.GetAllocator());
			val.PushBack(validate(t[2][0]), doc.GetAllocator());
			val.PushBack(validate(t[2][1]), doc.GetAllocator());
			val.PushBack(validate(t[2][2]), doc.GetAllocator());
			return val;
		}

		template <> inline rapidjson::Value serialize(rapidjson::Document& doc, const glm::mat4x4& t)
		{
			rapidjson::Value val(rapidjson::kArrayType);
			val.PushBack(validate(t[0][0]), doc.GetAllocator());
			val.PushBack(validate(t[0][1]), doc.GetAllocator());
			val.PushBack(validate(t[0][2]), doc.GetAllocator());
			val.PushBack(validate(t[0][3]), doc.GetAllocator());
			val.PushBack(validate(t[1][0]), doc.GetAllocator());
			val.PushBack(validate(t[1][1]), doc.GetAllocator());
			val.PushBack(validate(t[1][2]), doc.GetAllocator());
			val.PushBack(validate(t[1][3]), doc.GetAllocator());
			val.PushBack(validate(t[2][0]), doc.GetAllocator());
			val.PushBack(validate(t[2][1]), doc.GetAllocator());
			val.PushBack(validate(t[2][2]), doc.GetAllocator());
			val.PushBack(validate(t[2][3]), doc.GetAllocator());
			val.PushBack(validate(t[3][0]), doc.GetAllocator());
			val.PushBack(validate(t[3][1]), doc.GetAllocator());
			val.PushBack(validate(t[3][2]), doc.GetAllocator());
			val.PushBack(validate(t[3][3]), doc.GetAllocator());
			return val;
		}

		template <typename T>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const Vector<T>& t)
		{
			rapidjson::Value self(rapidjson::kArrayType);

			for (const auto& v : t)
				self.PushBack(serialize(doc, v), doc.GetAllocator());

			return self;
		}

		template <typename T, size_t N>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const Array<T, N>& t)
		{
			rapidjson::Value self(rapidjson::kArrayType);

			for (const auto& v : t)
				self.PushBack(serialize(doc, v), doc.GetAllocator());

			return self;
		}

		template <typename T>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const Set<T>& t)
		{
			rapidjson::Value self(rapidjson::kArrayType);

			for (const auto& v : t)
				self.PushBack(serialize(doc, v), doc.GetAllocator());

			return self;
		}

		template <typename T>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const Queue<T>& t)
		{
			rapidjson::Value self(rapidjson::kArrayType);

			for (const auto& v : t.get_container())
				self.PushBack(serialize(doc, v), doc.GetAllocator());

			return self;
		}

		template <typename T>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const Deque<T>& t)
		{
			rapidjson::Value self(rapidjson::kArrayType);

			for (const auto& v : t)
				self.PushBack(serialize(doc, v), doc.GetAllocator());

			return self;
		}

		template <typename T, typename U>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const Pair<T, U>& t)
		{
			rapidjson::Value pair(rapidjson::kObjectType);
			pair.AddMember("key", serialize(doc, t.first), doc.GetAllocator());
			pair.AddMember("value", serialize(doc, t.second), doc.GetAllocator());
			return pair;
		}

		template <typename T, typename U>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const UnorderedMap<T, U>& t)
		{
			rapidjson::Value self(rapidjson::kArrayType);

			for (const auto& v : t)
				self.PushBack(serialize(doc, v), doc.GetAllocator());

			return self;
		}

		template <typename T, typename U>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const Map<T, U>& t)
		{
			rapidjson::Value self(rapidjson::kArrayType);

			for (const auto& v : t)
				self.PushBack(serialize(doc, v), doc.GetAllocator());

			return self;
		}
		template <> inline void deserialize(const rapidjson::Value& self, bool&     t) { t = self.GetBool(); }
		template <> inline void deserialize(const rapidjson::Value& self, float&    t) { t = self.GetFloat(); }
		template <> inline void deserialize(const rapidjson::Value& self, double&   t) { t = self.GetDouble(); }
		template <> inline void deserialize(const rapidjson::Value& self, String&   t) { t = self.GetString(); }
		template <> inline void deserialize(const rapidjson::Value& self, uint8_t&  t) { t = (uint8_t)self.GetUint(); }
		template <> inline void deserialize(const rapidjson::Value& self, uint16_t& t) { t = (uint16_t)self.GetUint(); }
		template <> inline void deserialize(const rapidjson::Value& self, uint32_t& t) { t = (uint32_t)self.GetUint64(); }
		template <> inline void deserialize(const rapidjson::Value& self, uint64_t& t) { t = (uint64_t)self.GetUint64(); }
		template <> inline void deserialize(const rapidjson::Value& self, int8_t&   t) { t = (int8_t)self.GetInt(); }
		template <> inline void deserialize(const rapidjson::Value& self, int16_t&  t) { t = (int16_t)self.GetInt(); }
		template <> inline void deserialize(const rapidjson::Value& self, int32_t&  t) { t = (int32_t)self.GetInt64(); }
		template <> inline void deserialize(const rapidjson::Value& self, int64_t&  t) { t = (int64_t)self.GetInt64(); }

		template <> inline void deserialize(const rapidjson::Value& self, glm::vec2 &t)
		{
			const auto& a = self.GetArray();
			t = glm::vec2(a[0].GetFloat(), a[1].GetFloat());
		}

		template <> inline void deserialize(const rapidjson::Value& self, glm::vec3 &t)
		{
			const auto& a = self.GetArray();
			t = glm::vec3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());
		}

		template <> inline void deserialize(const rapidjson::Value& self, glm::vec4 &t)
		{
			const auto& a = self.GetArray();
			t = glm::vec4(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat(), a[3].GetFloat());
		}

		template <> inline void deserialize(const rapidjson::Value& self, glm::quat &t)
		{
			const auto& a = self.GetArray();
			t = glm::quat(a[3].GetFloat(), a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());
		}

		template <> inline void deserialize(const rapidjson::Value& self, glm::mat3x3 &t)
		{
			const auto& a = self.GetArray();

			t = glm::mat3x3(
				a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat(),
				a[3].GetFloat(), a[4].GetFloat(), a[5].GetFloat(),
				a[6].GetFloat(), a[7].GetFloat(), a[8].GetFloat()
			);
		}

		template <> inline void deserialize(const rapidjson::Value& self, glm::mat4x4 &t)
		{
			const auto& a = self.GetArray();

			t = glm::mat4x4(
				a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat(), a[3].GetFloat(),
				a[4].GetFloat(), a[5].GetFloat(), a[6].GetFloat(), a[7].GetFloat(),
				a[8].GetFloat(), a[9].GetFloat(), a[10].GetFloat(), a[11].GetFloat(),
				a[12].GetFloat(), a[13].GetFloat(), a[14].GetFloat(), a[15].GetFloat()
			);
		}

		template <typename T>
		inline void deserialize(const rapidjson::Value& self, Vector<T>& t)
		{
			const auto& a = self.GetArray();
			t.resize(a.Size());
			for (size_t i = 0ull; i < a.Size(); ++i)
				deserialize(a[(rapidjson::SizeType)i], t[i]);
		}

		template <typename T, size_t N>
		inline void deserialize(const rapidjson::Value& self, Array<T, N>& t)
		{
			const auto& a = self.GetArray();
			for (size_t i = 0ull; i < a.Size(); ++i)
				deserialize(a[(rapidjson::SizeType)i], t[i]);
		}

		template <typename T>
		inline void deserialize(const rapidjson::Value& self, Set<T>& t)
		{
			t.clear();

			const auto& a = self.GetArray();
			for (size_t i = 0ull; i < a.Size(); ++i)
			{
				T v;
				deserialize(a[(rapidjson::SizeType)i], v);
				t.insert(v);
			}
		}

		template <typename T>
		inline void deserialize(const rapidjson::Value& self, Queue<T>& t)
		{
			while (!t.empty())
				t.pop();

			const auto& a = self.GetArray();
			for (size_t i = 0ull; i < a.Size(); ++i)
			{
				T v;
				deserialize(a[(rapidjson::SizeType)i], v);
				t.push(v);
			}
		}

		template <typename T>
		inline void deserialize(const rapidjson::Value& self, Deque<T>& t)
		{
			t.clear();

			const auto& a = self.GetArray();
			for (size_t i = 0ull; i < a.Size(); ++i)
			{
				T v;
				deserialize(a[(rapidjson::SizeType)i], v);
				t.push_back(v);
			}
		}

		template <typename T, typename U>
		inline void deserialize(const rapidjson::Value& self, Pair<T, U>& t)
		{
			T key;
			U value;
			deserialize(self["key"], key);
			deserialize(self["value"], value);
			t.first = key;
			t.second = value;
		}

		template <typename T, typename U>
		inline void deserialize(const rapidjson::Value& self, UnorderedMap<T, U>& t)
		{
			const auto& a = self.GetArray();
			t.clear();
			for (size_t i = 0ull; i < a.Size(); ++i)
			{
				Pair<T, U> pair;
				deserialize(a[(rapidjson::SizeType)i], pair);
				t[pair.first] = pair.second;
			}
		}

		template <typename T, typename U>
		inline void deserialize(const rapidjson::Value& self, Map<T, U>& t)
		{
			const auto& a = self.GetArray();
			t.clear();
			for (size_t i = 0ull; i < a.Size(); ++i)
			{
				Pair<T, U> pair;
				deserialize(a[(rapidjson::SizeType)i], pair);
				t[pair.first] = pair.second;
			}
		}
		template <typename T>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const lambda::asset::VioletHandle<T>& t)
		{
			return rapidjson::Value(rapidjson::StringRef(t.getName().getName().c_str()));
		}

		template <typename T>
		inline void deserialize(const rapidjson::Value& self, lambda::asset::VioletHandle<T> &t)
		{
			String str = self.GetString();
			if (str.empty())
				t = lambda::asset::VioletHandle<T>();
			else
				t.metaSet(str);
		}

		template <>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const lambda::platform::ShaderPass& t)
		{
			rapidjson::Value self(rapidjson::kObjectType);

			self.AddMember("enabled", serialize(doc, t.getEnabled()), doc.GetAllocator());
			self.AddMember("name",    serialize(doc, t.getName()),    doc.GetAllocator());
			self.AddMember("shader",  serialize(doc, t.getShader()),  doc.GetAllocator());
			self.AddMember("inputs",  serialize(doc, t.getInputs()),  doc.GetAllocator());
			self.AddMember("outputs", serialize(doc, t.getOutputs()), doc.GetAllocator());

			return self;
		}
		template <>
		inline void deserialize(const rapidjson::Value& self, lambda::platform::ShaderPass& t)
		{
			bool enabled;
			Name name;
			asset::VioletShaderHandle shader;
			Vector<platform::RenderTarget> inputs;
			Vector<platform::RenderTarget> outputs;
			deserialize(self["enabled"], enabled);
			deserialize(self["name"], name);
			deserialize(self["shader"], shader);
			deserialize(self["inputs"], inputs);
			deserialize(self["outputs"], outputs);
			t.setEnabled(enabled);
			t.metaSetName(name);
			t.metaSetShader(shader);
			t.metaSetInputs(inputs);
			t.metaSetOutputs(outputs);
		}

		template <>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const lambda::platform::RenderTarget& t)
		{
			rapidjson::Value self(rapidjson::kObjectType);

			self.AddMember("render_scale", serialize(doc, t.getRenderScale()), doc.GetAllocator());
			self.AddMember("texture", serialize(doc, t.getTexture()), doc.GetAllocator());
			self.AddMember("name", serialize(doc, t.getName()), doc.GetAllocator());
			self.AddMember("mip_map", serialize(doc, t.getMipMap()), doc.GetAllocator());
			self.AddMember("layer", serialize(doc, t.getLayer()), doc.GetAllocator());
			self.AddMember("is_back_buffer", serialize(doc, t.isBackBuffer()), doc.GetAllocator());
			self.AddMember("from_texture", serialize(doc, t.fromTexture()), doc.GetAllocator());

			return self;
		}
		template <>
		inline void deserialize(const rapidjson::Value& self, lambda::platform::RenderTarget& t)
		{
			float render_scale;
			asset::VioletTextureHandle texture;
			Name name;
			int mip_map;
			int layer;
			bool is_back_buffer;
			bool from_texture;
			deserialize(self["render_scale"], render_scale);
			deserialize(self["texture"], texture);
			deserialize(self["name"], name);
			deserialize(self["mip_map"], mip_map);
			deserialize(self["layer"], layer);
			deserialize(self["is_back_buffer"], is_back_buffer);
			deserialize(self["from_texture"], from_texture);
			t.metaSetRenderScale(render_scale);
			t.metaSetTexture(texture);
			t.metaSetName(name);
			t.setMipMap(mip_map);
			t.setLayer(layer);
			t.metaSetIsBackBuffer(is_back_buffer);
			t.metaSetFromTexture(from_texture);
		}

		template <>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const components::LightType& t)
		{
			return rapidjson::Value((int64_t)t);
		}
		template <>
		inline void deserialize(const rapidjson::Value& self, components::LightType& t)
		{
			t = (components::LightType)self.GetInt64();
		}

		template <>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const components::ShadowType& t)
		{
			return rapidjson::Value((int64_t)t);
		}
		template <>
		inline void deserialize(const rapidjson::Value& self, components::ShadowType& t)
		{
			t = (components::ShadowType)self.GetInt64();
		}

		template <>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const components::WaveSourceState& t)
		{
			return rapidjson::Value((uint32_t)t);
		}
		template <>
		inline void deserialize(const rapidjson::Value& self, components::WaveSourceState& t)
		{
			t = (components::WaveSourceState)self.GetUint();
		}

		template <>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const lambda::utilities::Angle& t)
		{
			return rapidjson::Value(validate(t.asRad()));
		}
		template <>
		inline void deserialize(const rapidjson::Value& self, lambda::utilities::Angle& t)
		{
			t.setRad(self.GetFloat());
		}
		template <>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const lambda::utilities::Distance& t)
		{
			return rapidjson::Value(validate(t.asMeter()));
		}
		template <>
		inline void deserialize(const rapidjson::Value& self, lambda::utilities::Distance& t)
		{
			t.setMeter(self.GetFloat());
		}
#if VIOLET_PHYSICS_REACT
		template <>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const lambda::physics::ReactCollisionBodyType& t)
		{
			return rapidjson::Value((int32_t)t);
		}
		template <>
		inline void deserialize(const rapidjson::Value& self, lambda::physics::ReactCollisionBodyType& t)
		{
			t = (lambda::physics::ReactCollisionBodyType)self.GetInt();
		}

		template <>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const lambda::physics::ReactCollisionColliderType& t)
		{
			return rapidjson::Value((int32_t)t);
		}
		template <>
		inline void deserialize(const rapidjson::Value& self, lambda::physics::ReactCollisionColliderType& t)
		{
			t = (lambda::physics::ReactCollisionColliderType)self.GetInt();
		}

		template <>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const lambda::physics::ReactCollisionBody& t)
		{
			rapidjson::Value self(rapidjson::kObjectType);

			self.AddMember("position",             serialize(doc, t.getPosition()),            doc.GetAllocator());
			self.AddMember("rotation",             serialize(doc, t.getRotation()),            doc.GetAllocator());
			self.AddMember("entity",               serialize(doc, t.getEntity()),              doc.GetAllocator());
			self.AddMember("friction",             serialize(doc, t.getFriction()),            doc.GetAllocator());
			self.AddMember("mass",                 serialize(doc, t.getMass()),                doc.GetAllocator());
			self.AddMember("layers",               serialize(doc, t.getLayers()),              doc.GetAllocator());
			self.AddMember("velocity_constraints", serialize(doc, t.getVelocityConstraints()), doc.GetAllocator());
			self.AddMember("angular_constraints",  serialize(doc, t.getAngularConstraints()),  doc.GetAllocator());
			self.AddMember("velocity",             serialize(doc, t.getVelocity()),            doc.GetAllocator());
			self.AddMember("angular_velocity",     serialize(doc, t.getAngularVelocity()),     doc.GetAllocator());
			self.AddMember("mesh",                 serialize(doc, t.metaGetMesh()),            doc.GetAllocator());
			self.AddMember("sub_mesh_id",          serialize(doc, t.metaGetSubMeshId()),       doc.GetAllocator());
			self.AddMember("type",                 serialize(doc, t.metaGetType()),            doc.GetAllocator());
			self.AddMember("collider_type",        serialize(doc, t.metaGetColliderType()),    doc.GetAllocator());

			return self;
		}
		template <>
		inline void deserialize(const rapidjson::Value& self, lambda::physics::ReactCollisionBody& t)
		{
			entity::Entity entity;
			deserialize(self["entity"], entity);

			t = lambda::physics::ReactCollisionBody(
				lambda::physics::k_reactScene,
				lambda::physics::k_reactDynamicsWorld,
				lambda::physics::k_reactPhysicsWorld,
				entity
			);

			glm::vec3 position;
			glm::vec3 rotation;
			float friction;
			float mass;
			uint16_t layers;
			uint8_t velocity_constraints;
			uint8_t angular_constraints;
			glm::vec3 velocity;
			glm::vec3 angular_velocity;
			asset::VioletMeshHandle mesh;
			uint32_t sub_mesh_id;
			lambda::physics::ReactCollisionBodyType type;
			lambda::physics::ReactCollisionColliderType collider_type;
			deserialize(self["position"], position);
			deserialize(self["rotation"], rotation);
			deserialize(self["friction"], friction);
			deserialize(self["mass"], mass);
			deserialize(self["layers"], layers);
			deserialize(self["velocity_constraints"], velocity_constraints);
			deserialize(self["angular_constraints"], angular_constraints);
			deserialize(self["velocity"], velocity);
			deserialize(self["angular_velocity"], angular_velocity);
			deserialize(self["mesh"], mesh);
			deserialize(self["sub_mesh_id"], sub_mesh_id);
			deserialize(self["type"], type);
			deserialize(self["collider_type"], collider_type);

			t.setPosition(position);
			t.setRotation(rotation);
			t.setFriction(friction);
			t.setMass(mass);
			t.setLayers(layers);
			t.setVelocityConstraints(velocity_constraints);
			t.setAngularConstraints(angular_constraints);
			t.setVelocity(velocity);
			t.setAngularVelocity(angular_velocity);

			if (type == lambda::physics::ReactCollisionBodyType::kCollider)
				t.makeCollider();
			else if (type == lambda::physics::ReactCollisionBodyType::kRigidBody)
				t.makeRigidBody();

			if (collider_type == lambda::physics::ReactCollisionColliderType::kBox)
				t.makeBoxCollider();
			else if (collider_type == lambda::physics::ReactCollisionColliderType::kSphere)
				t.makeSphereCollider();
			else if (collider_type == lambda::physics::ReactCollisionColliderType::kCapsule)
				t.makeCapsuleCollider();
			else if (collider_type == lambda::physics::ReactCollisionColliderType::kMesh)
				t.makeMeshCollider(mesh, sub_mesh_id);
		}
#endif
	}
}