#pragma once
#include <glm/glm.hpp>
#include <containers/containers.h>
#include "texture.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <memory/memory.h>
#include <assets/mesh_manager.h>

namespace lambda
{
	namespace asset
	{
		enum MeshElements : uint32_t
		{
			kPositions = constexprHash("Positions"),
			kNormals   = constexprHash("Normals"),
			kTexCoords = constexprHash("TexCoords"),
			kColours   = constexprHash("Colours"),
			kTangents  = constexprHash("Tangents"),
			kJoints    = constexprHash("Joints"),
			kWeights   = constexprHash("Weights"),
			kIndices   = constexprHash("Indices"),
		};

		enum class Topology : uint8_t
		{
			kLines,
			kTriangles
		};

		struct SubMesh
		{
			struct Offset
			{
				Offset()
					: offset(0)
					, count(0)
					, stride(0)
				{};
				Offset(size_t offset, size_t count, size_t stride)
					: offset(offset)
					, count(count)
					, stride(stride)
				{};
				size_t offset;
				size_t count;
				size_t stride;

				bool operator==(const Offset& other) const
				{
					return !(*this != other);
				}
				bool operator!=(const Offset& other) const
				{
					return offset != other.offset ||
						count != other.count ||
						stride != other.stride;
				}
			};

			SubMesh() {};
			SubMesh(
				const UnorderedMap<uint32_t, Offset>& offsets,
				const glm::vec3& min,
				const glm::vec3& max)
				: offsets(offsets)
			{
				this->min = min;
				this->max = max;
			}

			Offset& offset(uint32_t hash) { return offsets[hash]; }

			UnorderedMap<uint32_t, Offset> offsets;
			glm::vec3 min;
			glm::vec3 max;
			size_t index_offset = 0;
			size_t vertex_offset = 0;

			struct {
				// Required information.
				int       parent = 0;
				int       topology = 0;
				glm::vec3 translation = glm::vec3(0.0f);
				glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
				glm::vec3 scale = glm::vec3(1.0f);
				// Textures.
				int       tex_alb  = -1;
				int       tex_nor  = -1;
				int       tex_dmra = -1;
				int       tex_emi  = -1;
				// Additional information.
				bool      double_sided = false;
				float     metallic  = 0.0f;
				float     roughness = 1.0f;
				glm::vec3 emissiveness;
				glm::vec4 colour = glm::vec4(1.0f);
			} io;

			bool operator==(const SubMesh& other) const
			{
				return !(*this != other);
			}
			bool operator!=(const SubMesh& other) const
			{
				return (offsets == other.offsets);
			}
		};

		class Mesh
		{
		public:
			struct Buffer
			{
				Buffer() {};
				Buffer(void* data, uint32_t count, uint16_t size)
					: data(data)
					, count(count)
					, size(size)
				{}
				template<typename T>
				Buffer(const Vector<T>& vector)
				{
					if (data)
						foundation::Memory::deallocate(data);

					count = (uint32_t)vector.size();
					size = sizeof(T);
					data = foundation::Memory::allocate(count * size);
					memcpy(data, vector.data(), count * size);
				}
				Buffer(const Buffer& other)
				{
					if (data)
						foundation::Memory::deallocate(data);
					count = other.count;
					size  = other.size;
					data  = foundation::Memory::allocate(count * size);
					memcpy(data, other.data, count * size);
				}
				void operator=(const Buffer& other)
				{
					if (data)
						foundation::Memory::deallocate(data);
					count = other.count;
					size  = other.size;
					data  = foundation::Memory::allocate(count * size);
					memcpy(data, other.data, count * size);
				}
				~Buffer()
				{
					foundation::Memory::deallocate(data);
					data  = nullptr;
					count = 0u;
					size  = 0u;
				}
				void*    data  = nullptr;
				uint32_t count = 0u;
				uint16_t size  = 0u;
			};

			Mesh();
			Mesh(
				const UnorderedMap<uint32_t, Buffer>& buffer,
				const Vector<SubMesh>& sub_meshes);
			Mesh(const Mesh& mesh);
			~Mesh();

			// Has
			bool has(const uint32_t& hash) const;
			// Get
			//Buffer get(const uint32_t& hash);
			const Buffer& get(const uint32_t& hash) const;
			template<typename T>
			Vector<T> get(const uint32_t& hash);
			template<typename T>
			Vector<T> get(const uint32_t& hash, const uint64_t& sub_mesh);
			// Set
			void set(const uint32_t& hash, const Buffer& buffer);
			// Sub Meshes
			void setSubMeshes(const Vector<SubMesh>& sub_meshes);
			const Vector<SubMesh>& getSubMeshes() const;
			Vector<SubMesh>& getSubMeshes();
			// Clearing / recalculating
			void clear(bool has_changed = true);
			void recalculateTangents();
			// Changed
			bool changed(const uint32_t& hash) const;
			void markAsChanged(const uint32_t& hash);
			void updated();
			// Topology
			Topology getTopology() const;
			void setTopology(const Topology& topology);
			// Textures
			void setAttachedTextures(const Vector<VioletTextureHandle>& textures);
			const Vector<VioletTextureHandle>& getAttachedTextures() const;
			void setAttachedTextureCount(const glm::uvec4& texture_count);
			const glm::uvec4& getAttachedTextureCount() const;
			// Creators
			static Mesh createPoint();
			static Mesh createQuad(const glm::vec2& min = glm::vec2(-1.0f), const glm::vec2& max = glm::vec2(1.0f));
			static Mesh createCube(glm::vec3 min = glm::vec3(-0.5f), glm::vec3 max = glm::vec3(0.5f));
			static Mesh createPlane(uint32_t sub_divisions = 1);
			static Mesh createPyramid();
			static Mesh createCone(float height = 1.5f, float bottom_radius = 0.5f, uint32_t slice_count = 24, uint32_t stack_count = 1);
			static Mesh createCylinder(float height = 1.5f, float top_radius = 0.5f, float bottom_radius = 0.5f, uint32_t slice_count = 24, uint32_t stack_count = 1);
			static Mesh createCircle(uint32_t segments = 24);
			static Mesh createSphere(float radius = 0.5f, uint32_t slice_count = 8, uint32_t stack_count = 8);
			static Mesh createScreenQuad();

			static void release(Mesh* mesh, const size_t& hash);
			static VioletHandle<Mesh> privMetaSet(const String& name);

		private:
			UnorderedMap<uint32_t, Buffer> buffer_;
			UnorderedMap<uint32_t, bool> changed_;

			Vector<VioletTextureHandle> textures_;
			glm::uvec4 texture_count_;

			Vector<SubMesh> sub_meshes_;
			Topology topology_ = Topology::kTriangles;
		};

		template<typename T>
		inline Vector<T> Mesh::get(const uint32_t& hash)
		{
			Buffer buffer = get(hash);
			Vector<T> vector(buffer.count);
			memcpy(vector.data(), buffer.data, buffer.count * buffer.size);
			return vector;
		}
		template<typename T>
		inline Vector<T> Mesh::get(const uint32_t& hash, const uint64_t& sub_mesh)
		{
			Buffer buffer  = get(hash);
			SubMesh sm     = sub_meshes_.at(sub_mesh);
			auto sm_offset = sm.offsets[hash];
			size_t count   = sm_offset.count;
			size_t offset  = sm_offset.offset;
			size_t stride  = sm_offset.stride;
			Vector<T> vector(count);
			memcpy(vector.data(), (char*)buffer.data + offset, count * stride);
			return vector;
		}

		using VioletMeshHandle = VioletHandle<Mesh>;

		///////////////////////////////////////////////////////////////////////////
		class MeshManager
		{
		public:
			VioletMeshHandle create(Name name);
			VioletMeshHandle create(Name name, Mesh mesh);
			VioletMeshHandle create(Name name, VioletMesh mesh);
			VioletMeshHandle get(Name name);
			VioletMeshHandle get(uint64_t hash);
			VioletMeshHandle getFromCache(Name name);
			void destroy(Mesh* mesh, const size_t& hash);

		public:
			static MeshManager* getInstance();
			static void setRenderer(platform::IRenderer* renderer);
			~MeshManager();

		protected:
			VioletMeshManager& getManager();
			const VioletMeshManager& getManager() const;

		private:
			platform::IRenderer* renderer_;
			VioletMeshManager manager_;
			UnorderedMap<uint64_t, Mesh*> mesh_cache_;
		};
	}
}
