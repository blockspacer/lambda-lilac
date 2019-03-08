#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include "mesh.h"
#include <sstream>

namespace lambda
{
  namespace io
  {
    class MeshIO
    {
    public:
      struct DataSegment
      {
        size_t offset;
        size_t count;
        size_t stride;
      };
      struct TextureSegment
      {
        size_t offset;
        size_t width;
        size_t height;
        size_t bpp;
      };
      struct DataInfo
      {
        Vector<unsigned char> data;
        Vector<DataSegment> segments;
      };
      struct TextureInfo
      {
        Vector<unsigned char> data;
        Vector<TextureSegment> segments;
      };
      struct SubMesh
      {
        // Required information.
        int parent   = 0;
        int topology = 0;
        glm::vec3 translation = glm::vec3(0.0f);
        glm::quat rotation    = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scale       = glm::vec3(1.0f);
        // Bounding volume.
        glm::vec3 aabb_min = glm::vec3(0.0f);
        glm::vec3 aabb_max = glm::vec3(0.0f);
        // Buffers.
        int pos = -1;
        int nor = -1;
        int tan = -1;
        int col = -1;
        int tex = -1;
        int idx = -1;
        int joi = -1;
        int wei = -1;
        // Textures.
        int tex_alb = -1;
        int tex_nor = -1;
        int tex_mrt = -1;
        // Additional information.
        bool double_sided = false;
        float metallic = 0.0f;
        float roughness = 1.0f;
        glm::vec4 colour = glm::vec4(1.0f);
      };
      struct MeshData
      {
        DataInfo pos;
        DataInfo nor;
        DataInfo tan;
        DataInfo col;
        DataInfo tex;
        DataInfo joi;
        DataInfo wei;
        DataInfo idx;
        TextureInfo tex_alb;
        TextureInfo tex_nor;
        TextureInfo tex_mrt;
      };
      struct Mesh
      {
        MeshData data;
        Vector<MeshIO::SubMesh> meshes;
      };

    public:
      static Mesh load(const String& path);
      static Vector<char> save(const Mesh& mesh);

      static asset::Mesh asAsset(Mesh& mesh);

    private:
      static bool loadMeshCustom(const String& path, Mesh& mesh);
      static Mesh loadMeshGLTF(const String& path);
      static Mesh loadMeshAssimp(const String& path);

    public:
      struct Format
      {
        static const char kMagicHeader[3];
        static const char kInvalidHeader[3];

        struct DataHeader
        {
          size_t data_size;
          size_t segment_count;
        };
        struct SegmentHeader
        {
          size_t offset;
          size_t count;
          size_t stride;
        };
        struct TextureHeader
        {
          size_t offset;
          size_t width;
          size_t height;
          size_t bpp;
        };
        struct ModelHeader
        {
          size_t model_count;
        };
      };
    };
  }
}