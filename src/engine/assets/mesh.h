#pragma once
#include "asset.h"
#include <glm/glm.hpp>
#include <containers/containers.h>
#include "texture.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>

namespace lambda
{
  namespace asset
  {
    enum MeshElements : uint8_t
    {
      kPositions = 0u,
      kNormals   = kPositions + 1u,
      kTexCoords = kNormals   + 1u,
      kColours   = kTexCoords + 1u,
      kTangents  = kColours   + 1u,
      kJoints    = kTangents  + 1u,
      kWeights   = kJoints    + 1u,
      kIndices   = kWeights   + 1u,
      kCount     = kIndices   + 1u,
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
        Offset() : offset(0), count(0), stride(0) {};
        Offset(size_t offset, size_t count, size_t stride) :
          offset(offset), count(count), stride(stride) {};
        size_t offset;
        size_t count;
        size_t stride;

        bool operator==(const Offset& other) const
        {
          return !(*this != other);
        }
        bool operator!=(const Offset& other) const
        {
          return offset != other.offset || count != other.count || stride != other.stride;
        }
      };

      SubMesh() {};
      SubMesh(const Array<Offset, MeshElements::kCount>& offset, const glm::vec3& min, const glm::vec3& max)
      {
        this->offset = offset;
        this->min = min;
        this->max = max;
      }

      Array<Offset, MeshElements::kCount> offset;
      glm::vec3 min;
      glm::vec3 max;
      size_t index_offset  = 0;
      size_t vertex_offset = 0;

      struct {
        // Required information.
        int parent   = 0;
        int topology = 0;
        glm::vec3 translation = glm::vec3(0.0f);
        glm::quat rotation    = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scale       = glm::vec3(1.0f);
        // Textures.
        int tex_alb = -1;
        int tex_nor = -1;
        int tex_mrt = -1;
        // Additional information.
        bool double_sided = false;
        float metallic    = 0.0f;
        float roughness   = 1.0f;
        glm::vec4 colour  = glm::vec4(1.0f);
      } io;

      bool operator==(const SubMesh& other) const
      {
        return !(*this != other);
      }
      bool operator!=(const SubMesh& other) const
      {
        if (offset.size() != other.offset.size())
        {
          return false;
        }
        for (uint8_t i = 0u; i < MeshElements::kCount; ++i)
        {
          if (offset.at(i) != other.offset.at(i))
          {
            return false;
          }
        }
        return true;
      }
    };

    class Mesh : public IAsset
    {
    public:
      struct Buffer
      {
        Buffer() {};
        Buffer(void* data, uint32_t count, uint16_t size) :
          data(data), count(count), size(size) {}
        template<typename T>
        Buffer(const Vector<T>& vector)
        {
          count = (uint32_t)vector.size();
          size  = sizeof(T);
          data  = foundation::Memory::allocate(count * size);
          memcpy(data, vector.data(), count * size);
        }
        void*    data  = nullptr;
        uint32_t count = 0u;
        uint16_t size  = 0u;
      };

      Mesh();
      Mesh(const Array<Buffer, MeshElements::kCount>& buffer, const Vector<SubMesh>& sub_meshes);
      Mesh(const Mesh& mesh);
      ~Mesh();

      // Get
      Buffer get(const uint8_t& index) const;
      template<typename T>
      Vector<T> get(const uint8_t& index) const;
      template<typename T>
      Vector<T> get(const uint8_t& index, const uint64_t& sub_mesh) const;
      // Set
      void set(const uint8_t& index, const Buffer& buffer);
      template<typename T>
      void set(const uint8_t& index, const Vector<T>& vector);
      // Sub Meshes
      void setSubMeshes(const Vector<SubMesh>& sub_meshes);
      const Vector<SubMesh>& getSubMeshes() const;
      // Clearing / recalculating
      void clear(bool has_changed = true);
      void recalculateTangents();
      // Changed
      bool changed(const uint8_t& index) const;
      void markAsChanged(const uint8_t& index);
      void updated();
      // Topology
      Topology getTopology() const;
      void setTopology(const Topology& topology);
      // Textures
      void setAttachedTextures(const Vector<VioletTextureHandle>& textures);
      const Vector<VioletTextureHandle>& getAttachedTextures() const;
      void setAttachedTextureCount(const glm::uvec3& texture_count);
      const glm::uvec3& getAttachedTextureCount() const;
      // Creators
      static Mesh createPoint();
      static Mesh createQuad(const glm::vec2& min = glm::vec2(-1.0f), const glm::vec2& max = glm::vec2(1.0f));
      static Mesh createCube();
      static Mesh createPlane(uint32_t sub_divisions = 1);
      static Mesh createPyramid();
      static Mesh createCone(float height = 1.5f, float bottom_radius = 0.5f, uint32_t slice_count = 24, uint32_t stack_count = 1);
      static Mesh createCylinder(float height = 1.5f, float top_radius = 0.5f, float bottom_radius = 0.5f, uint32_t slice_count = 24, uint32_t stack_count = 1);
      static Mesh createCircle(uint32_t segments = 24);
      static Mesh createSphere(float radius = 0.5f, uint32_t slice_count = 8, uint32_t stack_count = 8);
      static Mesh createScreenQuad();

    private:
      Array<Buffer, MeshElements::kCount> buffer_;
      Array<bool,   MeshElements::kCount> changed_;

      Vector<VioletTextureHandle> textures_;
      glm::uvec3 texture_count_;

      Vector<SubMesh> sub_meshes_;
      Topology topology_ = Topology::kTriangles;
    };
    typedef AssetHandle<Mesh> MeshHandle;

    template<typename T>
    inline Vector<T> Mesh::get(const uint8_t& index) const
    {
      Buffer buffer = get(index);
      Vector<T> vector(buffer.count);
      memcpy(vector.data(), buffer.data, buffer.count * buffer.size);
      return vector;
    }
    template<typename T>
    inline Vector<T> Mesh::get(const uint8_t& index, const uint64_t& sub_mesh) const
    {
      Buffer buffer = get(index);
      SubMesh sm = sub_meshes_.at(sub_mesh);
      size_t count = sm.offset.at(index).count;
      size_t offset = sm.offset.at(index).offset;
      size_t stride = sm.offset.at(index).stride;
      Vector<T> vector(count);
      memcpy(vector.data(), (char*)buffer.data + offset, count * stride);
      return vector;
    }
    template<typename T>
    inline void Mesh::set(const uint8_t& index, const Vector<T>& vector)
    {
      Buffer buffer((void*)vector.data(), (uint32_t)vector.size(), sizeof(T));
      set(index, buffer);
    }
}
}