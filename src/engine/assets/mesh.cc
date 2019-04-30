#include "mesh.h"
#include "utils/angle.h"
#include <glm/gtx/orthonormalize.hpp>
#include "interfaces/irenderer.h"
#include <utils/file_system.h>

namespace lambda
{
  namespace asset
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh::Mesh()
    {
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh::Mesh(const UnorderedMap<uint32_t, Buffer>& buffers, const Vector<SubMesh>& sub_meshes)
    {
			for (const auto& it : buffers)
				set(it.first, it.second);
      setSubMeshes(sub_meshes);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh::Mesh(const Mesh& mesh)
    {
			for (const auto& it : mesh.buffer_)
				set(it.first, it.second);
      setSubMeshes(mesh.sub_meshes_);

      textures_      = mesh.textures_;
      texture_count_ = mesh.texture_count_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh::~Mesh()
    {
      textures_.resize(0u);
			buffer_.clear();
			changed_.clear();
      sub_meshes_.resize(0u);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Mesh::setSubMeshes(const Vector<SubMesh>& sub_meshes)
    {
      sub_meshes_.clear();
      sub_meshes_ = sub_meshes;
    }

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		const Vector<SubMesh>& Mesh::getSubMeshes() const
		{
			return sub_meshes_;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		Vector<SubMesh>& Mesh::getSubMeshes()
		{
			return sub_meshes_;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool Mesh::has(const uint32_t& hash) const
		{
			return buffer_.find(hash) != buffer_.end();
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Mesh::Buffer Mesh::get(const uint32_t& hash)
		//{
		//	return buffer_.at(hash);
		//}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		const Mesh::Buffer& Mesh::get(const uint32_t& hash) const
		{
			return buffer_.at(hash);
		}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Mesh::set(const uint32_t& hash, const Buffer& buffer)
    {
			buffer_[hash] = buffer;
      changed_[hash] = true;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Mesh::clear(bool has_changed)
    {
			buffer_.clear();
			changed_.clear();
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Mesh::recalculateTangents()
    {
      Vector<glm::vec3> positions  = get<glm::vec3>(MeshElements::kPositions);
      Vector<glm::vec3> normals    = get<glm::vec3>(MeshElements::kNormals);
      Vector<glm::vec2> tex_coords = get<glm::vec2>(MeshElements::kTexCoords);
      Vector<uint32_t>  indices    = get<uint32_t>(MeshElements::kIndices);
      Vector<glm::vec3> tangents(positions.size());

      for (uint32_t i = 0; i < indices.size(); i += 3)
      {
        uint32_t i1 = indices[i];
        uint32_t i2 = indices[i + 1];
        uint32_t i3 = indices[i + 2];

        glm::vec3 p1 = positions[i1];
        glm::vec3 p2 = positions[i2];
        glm::vec3 p3 = positions[i3];

        glm::vec2 uv1 = tex_coords[i1];
        glm::vec2 uv2 = tex_coords[i2];
        glm::vec2 uv3 = tex_coords[i3];

				glm::vec3 edge1 = p2 - p1;
				glm::vec3 edge2 = p3 - p1;
				glm::vec2 deltaUV1 = uv2 - uv1;
				glm::vec2 deltaUV2 = uv3 - uv1;

				float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

				glm::vec3 tangent;
				tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
				tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
				tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
				tangent = glm::normalize(tangent);

        tangents[i1] += tangent;
        tangents[i2] += tangent;
        tangents[i3] += tangent;
      }

      for (uint32_t i = 0; i < tangents.size(); ++i)
				if (tangents[i] != glm::vec3(0.0f, 0.0f, 0.0f))
					tangents[i] = glm::normalize(tangents[i]);

      set(MeshElements::kTangents, Buffer(tangents));
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Mesh::changed(const uint32_t& hash) const
    {
      return changed_.at(hash);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Mesh::markAsChanged(const uint32_t& hash)
    {
      changed_.at(hash) = true;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Mesh::updated()
    {
			for (auto& it : changed_)
				it.second = false;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Topology Mesh::getTopology() const
    {
      return topology_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Mesh::setTopology(const Topology& topology)
    {
      topology_ = topology;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Mesh::setAttachedTextures(const Vector<VioletTextureHandle>& textures)
    {
      textures_.resize(0u);
      textures_ = textures;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const Vector<VioletTextureHandle>& Mesh::getAttachedTextures() const
    {
      return textures_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Mesh::setAttachedTextureCount(const glm::uvec4& texture_count)
    {
      texture_count_ = texture_count;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const glm::uvec4& Mesh::getAttachedTextureCount() const
    {
      return texture_count_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh Mesh::createScreenQuad()
    {
      Vector<glm::vec3> positions = {
        glm::vec3(-1.0f, -1.0f, +1.0f),
        glm::vec3(+1.0f, -1.0f, +1.0f),
        glm::vec3(+1.0f, +1.0f, +1.0f),
        glm::vec3(-1.0f, +1.0f, +1.0f),

        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3(+1.0f, -1.0f, -1.0f),
        glm::vec3(+1.0f, +1.0f, -1.0f),
        glm::vec3(-1.0f, +1.0f, -1.0f),
      };
      Vector<glm::vec3> normals = {
        glm::vec3(0.0f),
        glm::vec3(0.0f),
        glm::vec3(0.0f)
      };
      Vector<glm::vec2> tex_coords = {
        glm::vec2(0.0f),
        glm::vec2(0.0f),
        glm::vec2(0.0f)
      };
      Vector<glm::vec4> colours = {
        glm::vec4(1.0f),
        glm::vec4(1.0f),
        glm::vec4(1.0f)
      };
      Vector<uint32_t> indices = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3,
      };
      Vector<glm::vec3> tangents = {
        glm::vec3(1.0f),
        glm::vec3(1.0f),
        glm::vec3(1.0f)
      };

      Vector<SubMesh> sub_meshes = {
				SubMesh{ {
				{ kPositions, SubMesh::Offset(0, 3, sizeof(glm::vec3)) },
				{ kNormals,   SubMesh::Offset(0, 3, sizeof(glm::vec3)) },
				{ kTexCoords, SubMesh::Offset(0, 3, sizeof(glm::vec2)) },
				{ kColours,   SubMesh::Offset(0, 3, sizeof(glm::vec4)) },
				{ kTangents,  SubMesh::Offset(0, 3, sizeof(glm::vec3)) },
				{ kJoints,    SubMesh::Offset(0, 0, sizeof(glm::vec4)) },
				{ kWeights,   SubMesh::Offset(0, 0, sizeof(glm::vec4)) },
				{ kIndices,   SubMesh::Offset(0, 3, sizeof(uint32_t)) } },
        glm::vec3(0.0f),
        glm::vec3(0.0f)
      }
      };
      sub_meshes.at(0u).io.double_sided = true;

      Mesh mesh({
				{ kPositions, positions  },
				{ kNormals,   normals    },
				{ kTexCoords, tex_coords },
				{ kColours,   colours    },
				{ kTangents,  tangents   },
				{ kJoints,    Buffer()   },
				{ kWeights,   Buffer()   },
				{ kIndices,   indices    }
			}, sub_meshes);

			mesh.recalculateTangents();

			return mesh;
    }

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void Mesh::release(Mesh* mesh, const size_t& hash)
		{
			MeshManager::getInstance()->destroy(mesh, hash);
		}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void getMinMax(const Vector<glm::vec3>& positions, glm::vec3& min, glm::vec3& max)
    {
      for (const glm::vec3& pos : positions)
      {
        if      (pos.x < min.x) min.x = pos.x;
        else if (pos.x > max.x) max.x = pos.x;
        if      (pos.y < min.y) min.y = pos.y;
        else if (pos.y > max.y) max.y = pos.y;
        if      (pos.z < min.z) min.z = pos.z;
        else if (pos.z > max.z) max.z = pos.z;
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh Mesh::createCube()
    {
      Vector<glm::vec3> vertices;
      Vector<glm::vec2> uvs;
      Vector<glm::vec3> normals;
      Vector<glm::vec4> colors;
      Vector<uint32_t>  indices;

      //FRONT
      vertices.emplace_back(-0.5f, -0.5f, -0.5f); uvs.emplace_back(0.0f, 1.0f);
      vertices.emplace_back(-0.5f,  0.5f, -0.5f); uvs.emplace_back(0.0f, 0.0f);
      vertices.emplace_back( 0.5f,  0.5f, -0.5f); uvs.emplace_back(1.0f, 0.0f);
      vertices.emplace_back( 0.5f, -0.5f, -0.5f); uvs.emplace_back(1.0f, 1.0f);

      //BACK
      vertices.emplace_back(-0.5f, -0.5f, 0.5f); uvs.emplace_back(1.0f, 1.0f);
      vertices.emplace_back( 0.5f, -0.5f, 0.5f); uvs.emplace_back(0.0f, 1.0f);
      vertices.emplace_back( 0.5f,  0.5f, 0.5f); uvs.emplace_back(0.0f, 0.0f);
      vertices.emplace_back(-0.5f,  0.5f, 0.5f); uvs.emplace_back(1.0f, 0.0f);

      //LEFT
      vertices.emplace_back(-0.5f, -0.5f,  0.5f); uvs.emplace_back(0.0f, 1.0f);
      vertices.emplace_back(-0.5f,  0.5f,  0.5f); uvs.emplace_back(0.0f, 0.0f);
      vertices.emplace_back(-0.5f,  0.5f, -0.5f); uvs.emplace_back(1.0f, 0.0f);
      vertices.emplace_back(-0.5f, -0.5f, -0.5f); uvs.emplace_back(1.0f, 1.0f);

      //RIGHT
      vertices.emplace_back(0.5f, -0.5f, -0.5f); uvs.emplace_back(0.0f, 1.0f);
      vertices.emplace_back(0.5f,  0.5f, -0.5f); uvs.emplace_back(0.0f, 0.0f);
      vertices.emplace_back(0.5f,  0.5f,  0.5f); uvs.emplace_back(1.0f, 0.0f);
      vertices.emplace_back(0.5f, -0.5f,  0.5f); uvs.emplace_back(1.0f, 1.0f);

      //TOP
      vertices.emplace_back(-0.5f, 0.5f, -0.5f); uvs.emplace_back(0.0f, 1.0f);
      vertices.emplace_back(-0.5f, 0.5f,  0.5f); uvs.emplace_back(0.0f, 0.0f);
      vertices.emplace_back( 0.5f, 0.5f,  0.5f); uvs.emplace_back(1.0f, 0.0f);
      vertices.emplace_back( 0.5f, 0.5f, -0.5f); uvs.emplace_back(1.0f, 1.0f);

      //BOTTOM
      vertices.emplace_back(-0.5f, -0.5f, -0.5f); uvs.emplace_back(1.0f, 1.0f);
      vertices.emplace_back( 0.5f, -0.5f, -0.5f); uvs.emplace_back(0.0f, 1.0f);
      vertices.emplace_back( 0.5f, -0.5f,  0.5f); uvs.emplace_back(0.0f, 0.0f);
      vertices.emplace_back(-0.5f, -0.5f,  0.5f); uvs.emplace_back(1.0f, 0.0f);

      //Generate normals
      for (int i = 0; i < 4; ++i)
        normals.emplace_back(glm::vec3(0.0f, 0.0f, -1.0f));
      for (int i = 0; i < 4; ++i)
        normals.emplace_back(glm::vec3(0.0f, 0.0f, 1.0f));
      for (int i = 0; i < 4; ++i)
        normals.emplace_back(glm::vec3(-1.0f, 0.0f, 0.0f));
      for (int i = 0; i < 4; ++i)
        normals.emplace_back(glm::vec3(1.0f, 0.0f, 0.0f));
      for (int i = 0; i < 4; ++i)
        normals.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));
      for (int i = 0; i < 4; ++i)
        normals.emplace_back(glm::vec3(0.0f, -1.0f, 0.0f));

      //Generate indices
      uint32_t index = 0;
      uint32_t faceCount = (uint32_t)vertices.size() / 4;
      for (uint32_t i = 0; i < faceCount; i++) {
        indices.emplace_back(index);     //0
        indices.emplace_back(++index);   //1
        indices.emplace_back(++index);   //2
        indices.emplace_back(index);     //2
        indices.emplace_back(++index);   //3
        indices.emplace_back(index - 3); //0
        index++;
      }

      // Add colors
      for (uint32_t i = 0; i < vertices.size(); ++i)
        colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

      glm::vec3 min(FLT_MAX), max(FLT_MIN);
      getMinMax(vertices, min, max);
      Mesh cube({
				{ kPositions, vertices },
				{ kNormals,   normals  },
				{ kTexCoords, uvs      },
				{ kColours,   colors   },
				{ kTangents,  Buffer() },
				{ kJoints,    Buffer() },
				{ kWeights,   Buffer() },
				{ kIndices,   indices  }
			},
        Vector<SubMesh>{ SubMesh{
        { 
					{ kPositions, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kNormals,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kTexCoords, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec2)) },
					{ kColours,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec4)) },
					{ kTangents,  SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kJoints,    SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
					{ kWeights,   SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
					{ kIndices,   SubMesh::Offset(0, indices.size(), sizeof(uint32_t))   }
					},
        min, max }}
      );

      cube.recalculateTangents();

      return cube;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh Mesh::createPoint()
    {
      Mesh point;
      Vector<glm::vec3> vertices;
      Vector<glm::vec2> uvs;
      Vector<glm::vec3> normals;
      Vector<glm::vec3> tangents;
      Vector<glm::vec4> colors;
      Vector<uint32_t>  indices;

      vertices.emplace_back(0.0f, 0.0f, 0.0f);
      uvs.emplace_back(0.0f, 0.0f);
      normals.emplace_back(glm::vec3(0.0f, 0.0f, -1.0f));
      tangents.emplace_back(glm::vec3(1.0f, 0.0f, 0.0f));
      indices.emplace_back(0);
      colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

      point.set(MeshElements::kPositions, eastl::move(vertices));
      point.set(MeshElements::kIndices,   eastl::move(indices));
      point.set(MeshElements::kTexCoords, eastl::move(uvs));
      point.set(MeshElements::kNormals,   eastl::move(normals));
      point.set(MeshElements::kTangents,  eastl::move(tangents));
      point.set(MeshElements::kColours,   eastl::move(colors));

      return point;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh Mesh::createQuad(const glm::vec2& min, const glm::vec2& max)
    {
      Vector<glm::vec3> vertices;
      Vector<glm::vec2> uvs;
      Vector<glm::vec3> normals;
      Vector<glm::vec4> colors;
      Vector<uint32_t>  indices;

      vertices.emplace_back(min.x, min.y, 1.0f); uvs.emplace_back(0.0f, 0.0f);
      vertices.emplace_back(min.x, max.y, 1.0f); uvs.emplace_back(0.0f, 1.0f);
      vertices.emplace_back(max.x, max.y, 1.0f); uvs.emplace_back(1.0f, 1.0f);
      vertices.emplace_back(max.x, min.y, 1.0f); uvs.emplace_back(1.0f, 0.0f);

      for (int i = 0; i < 4; ++i)
        normals.emplace_back(glm::vec3(0.0f, 0.0f, -1.0f));

      //Generate indices
      indices.emplace_back(0);
      indices.emplace_back(1);
      indices.emplace_back(2);
      indices.emplace_back(2);
      indices.emplace_back(3);
      indices.emplace_back(0);

      // Add colors
      for (size_t i = 0; i < vertices.size(); ++i)
        colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

      glm::vec3 mmin(FLT_MAX), mmax(FLT_MIN);
      getMinMax(vertices, mmin, mmax);
      Mesh quad({ 
				{ kPositions, vertices },
				{ kNormals,   normals  },
				{ kTexCoords, uvs      },
				{ kColours,   colors   },
				{ kTangents,  Buffer() },
				{ kJoints,    Buffer() },
				{ kWeights,   Buffer() },
				{ kIndices,   indices  }
			},
        Vector<SubMesh>{ SubMesh{
        { 
					{ kPositions, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kNormals,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kTexCoords, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec2)) },
					{ kColours,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec4)) },
					{ kTangents,  SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kJoints,    SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
					{ kWeights,   SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
					{ kIndices,   SubMesh::Offset(0, indices.size(), sizeof(uint32_t))   } },
        mmin, mmax }}
      );

      quad.recalculateTangents();

      return quad;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh Mesh::createPlane(uint32_t subDivisions)
    {
      Vector<glm::vec3> vertices;
      Vector<glm::vec2> uvs;
      Vector<glm::vec3> normals;
      Vector<glm::vec4> colors;
      Vector<uint32_t>  indices;

      float size = 1.0f / subDivisions;

      for (uint32_t x = 0; x <= subDivisions; ++x) {
        for (uint32_t z = 0; z <= subDivisions; ++z) {
          float _x = x * size - 0.5f;
          float _z = z * size - 0.5f;

          vertices.emplace_back(_x, 0.0f, _z);
          uvs.emplace_back(_x, _z);
        }
      }

      for (size_t i = 0; i < vertices.size(); ++i)
        normals.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));

      //Generate indices
      for (uint32_t r = 0; r < subDivisions; ++r) {
        for (uint32_t c = 0; c < subDivisions; ++c) {
          indices.emplace_back(r * (subDivisions + 1) + c);
          indices.emplace_back(r * (subDivisions + 1) + (c + 1));
          indices.emplace_back((r + 1) * (subDivisions + 1) + c);

          indices.emplace_back((r + 1) * (subDivisions + 1) + c);
          indices.emplace_back((r * (subDivisions + 1) + (c + 1)));
          indices.emplace_back((r + 1) * (subDivisions + 1) + (c + 1));
        }
      }

      // Add colors
      for (size_t i = 0; i < vertices.size(); ++i)
        colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

      glm::vec3 min(FLT_MAX), max(FLT_MIN);
      getMinMax(vertices, min, max);
      Mesh plane({ 
				{ kPositions, vertices },
				{ kNormals,   normals  },
				{ kTexCoords, uvs      },
				{ kColours,   colors   },
				{ kTangents,  Buffer() },
				{ kJoints,    Buffer() },
				{ kWeights,   Buffer() },
				{ kIndices,   indices  }
			},
        Vector<SubMesh>{ SubMesh{
        {
					{ kPositions, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kNormals,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kTexCoords, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec2)) },
					{ kColours,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec4)) },
					{ kTangents,  SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kJoints,    SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
					{ kWeights,   SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
					{ kIndices,   SubMesh::Offset(0, indices.size(), sizeof(uint32_t))   } },
        min, max }}
      );

      plane.recalculateTangents();

      return plane;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh Mesh::createPyramid()
    {
      return createCone(1.0f, 1.0f, 4);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh Mesh::createCone(float height, float bottomRadius, uint32_t sliceCount, uint32_t stackCount)
    {
      return createCylinder(height, 0.0f, bottomRadius, sliceCount, stackCount);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh Mesh::createCylinder(float height, float topRadius, float bottomRadius, uint32_t sliceCount, uint32_t stackCount)
    {
      Vector<glm::vec3> vertices;
      Vector<glm::vec2> uvs;
      Vector<glm::vec3> normals;
      Vector<glm::vec3> tangents;
      Vector<glm::vec4> colors;
      Vector<uint32_t>  indices;

      float stackHeight = height / stackCount;
      float radiusStep = (topRadius - bottomRadius) / stackCount;
      uint32_t ringCount = stackCount + 1;
      float dTheta = utilities::Angle::tau / sliceCount;

      for (uint32_t i = 0; i < ringCount; ++i)
      {
        float y = -0.5f*height + i*stackHeight;
        float r = bottomRadius + i*radiusStep;

        // vertices of ring
        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
          float c = cosf(j*dTheta);
          float s = sinf(j*dTheta);
          vertices.emplace_back(glm::vec3(r*c, y, r*s));
          colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
          uvs.emplace_back(glm::vec2((float)j / sliceCount, 1.0f - (float)i / stackCount));

          glm::vec3 T(-s, 0.0f, c);
          float dr = bottomRadius - topRadius;
          glm::vec3 B(dr*c, -height, dr*s);
          glm::vec3 N = glm::normalize(glm::cross(T, B));

          tangents.emplace_back(T);
          normals.emplace_back(N);
        }
      }

      // Add one because we duplicate the first and last vertex per ring
      // since the texture coordinates are different.
      uint32_t ringVertexCount = sliceCount + 1;
      // Compute indices for each stack.
      for (uint32_t i = 0; i < stackCount; ++i)
      {
        for (uint32_t j = 0; j < sliceCount; ++j)
        {
          indices.push_back(i*ringVertexCount + j);
          indices.push_back((i + 1)*ringVertexCount + j);
          indices.push_back((i + 1)*ringVertexCount + j + 1);
          indices.push_back(i*ringVertexCount + j);
          indices.push_back((i + 1)*ringVertexCount + j + 1);
          indices.push_back(i*ringVertexCount + j + 1);
        }
      }

      // Build top cap
      uint32_t baseIndex = (uint32_t)vertices.size();
      float y = 0.5f*height;

      // Duplicate cap ring vertices because the texture coordinates and normals differ.
      for (uint32_t i = 0; i <= sliceCount; ++i)
      {
        float x = topRadius*cosf(i*dTheta);
        float z = topRadius*sinf(i*dTheta);
        // Scale down by the height to try and make top cap texture coord area
        // proportional to base.
        float u = x / height + 0.5f;
        float v = z / height + 0.5f;
        vertices.emplace_back(glm::vec3(x, y, z));
        colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        normals.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));
        tangents.emplace_back(glm::vec3(1.0f, 0.0f, 0.0f));
        uvs.emplace_back(glm::vec2(u, v));
      }
      // Cap center vertex.
      vertices.emplace_back(glm::vec3(0.0f, y, 0.0f));
      colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
      normals.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));
      tangents.emplace_back(glm::vec3(1.0f, 0.0f, 0.0f));
      uvs.emplace_back(glm::vec2(0.5f, 0.5f));

      // Index of center vertex.
      uint32_t centerIndex = (uint32_t)vertices.size() - 1;
      for (uint32_t i = 0; i < sliceCount; ++i)
      {
        indices.push_back(centerIndex);
        indices.push_back(baseIndex + i + 1);
        indices.push_back(baseIndex + i);
      }

      // Build bottom cap
      baseIndex = (uint32_t)vertices.size();
      y = -0.5f*height;

      for (uint32_t i = 0; i <= sliceCount; ++i)
      {
        float x = bottomRadius*cosf(i*dTheta);
        float z = bottomRadius*sinf(i*dTheta);
        // Scale down by the height to try and make top cap texture coord area
        // proportional to base.
        float u = x / height + 0.5f;
        float v = z / height + 0.5f;
        vertices.emplace_back(glm::vec3(x, y, z));
        colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        normals.emplace_back(glm::vec3(0.0f, -1.0f, 0.0f));
        tangents.emplace_back(glm::vec3(1.0f, 0.0f, 0.0f));
        uvs.emplace_back(glm::vec2(u, v));
      }
      // Cap center vertex.
      vertices.emplace_back(glm::vec3(0.0f, y, 0.0f));
      colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
      normals.emplace_back(glm::vec3(0.0f, -1.0f, 0.0f));
      tangents.emplace_back(glm::vec3(1.0f, 0.0f, 0.0f));
      uvs.emplace_back(glm::vec2(0.5f, 0.5f));

      // Cache the index of center vertex.
      centerIndex = (uint32_t)vertices.size() - 1;
      for (uint32_t i = 0; i < sliceCount; ++i)
      {
        indices.push_back(centerIndex);
        indices.push_back(baseIndex + i);
        indices.push_back(baseIndex + i + 1);
      }

      glm::vec3 min(FLT_MAX), max(FLT_MIN);
      getMinMax(vertices, min, max);
      Mesh cylinder({ 
				{ kPositions, vertices },
				{ kNormals,   normals  },
				{ kTexCoords, uvs      },
				{ kColours,   colors   },
				{ kTangents,  Buffer() },
				{ kJoints,    Buffer() },
				{ kWeights,   Buffer() },
				{ kIndices,   indices  }
			},
        Vector<SubMesh>{ SubMesh{
        {
					{ kPositions, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kNormals,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kTexCoords, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec2)) },
					{ kColours,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec4)) },
					{ kTangents,  SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kJoints,    SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
					{ kWeights,   SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
					{ kIndices,   SubMesh::Offset(0, indices.size(), sizeof(uint32_t))   } },
        min, max }}
      );

      return cylinder;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh Mesh::createCircle(uint32_t segments)
    {
      Vector<glm::vec3> vertices;
      Vector<glm::vec2> uvs;
      Vector<glm::vec3> normals;
      Vector<glm::vec3> tangents;
      Vector<glm::vec4> colors;
      Vector<uint32_t>  indices;

      float dTheta = utilities::Angle::tau / segments;

      for (uint32_t i = 0; i <= segments; ++i)
      {
        float x = cosf(i*dTheta);
        float z = sinf(i*dTheta);

        float u = x + 0.5f;
        float v = z + 0.5f;
        vertices.emplace_back(glm::vec3(x, z, 0.0f));
        colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        normals.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));
        tangents.emplace_back(glm::vec3(1.0f, 0.0f, 0.0f));
        uvs.emplace_back(glm::vec2(u, v));
      }
      // Center vertex.
      vertices.emplace_back(glm::vec3(0.0f, 0.0f, 0.0f));
      colors.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
      normals.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));
      tangents.emplace_back(glm::vec3(1.0f, 0.0f, 0.0f));
      uvs.emplace_back(glm::vec2(0.5f, 0.5f));

      uint32_t centerIndex = (uint32_t)vertices.size() - 1;
      for (uint32_t i = 0; i < segments; ++i)
      {
        indices.push_back(centerIndex);
        indices.push_back(i + 1);
        indices.push_back(i);
      }

      glm::vec3 min(FLT_MAX), max(FLT_MIN);
      getMinMax(vertices, min, max);
      Mesh circle({ 
				{ kPositions, vertices },
				{ kNormals,   normals  },
				{ kTexCoords, uvs      },
				{ kColours,   colors   },
				{ kTangents,  Buffer() },
				{ kJoints,    Buffer() },
				{ kWeights,   Buffer() },
				{ kIndices,   indices  }
			},
        Vector<SubMesh>{ SubMesh{
          { 
						{ kPositions, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
						{ kNormals,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
						{ kTexCoords, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec2)) },
						{ kColours,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec4)) },
						{ kTangents,  SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
						{ kJoints,    SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
						{ kWeights,   SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
						{ kIndices,   SubMesh::Offset(0, indices.size(), sizeof(uint32_t))   } },
        min, max }}
      );

      circle.recalculateTangents();

      return circle;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mesh Mesh::createSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
    {
      Vector<glm::vec3> vertices;
      Vector<glm::vec2> uvs;
      Vector<glm::vec3> normals;
      Vector<glm::vec4> colors;
      Vector<uint32_t>  indices;

      vertices.push_back(glm::vec3(0.0f, radius, 0.0f));
      normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
      uvs.push_back(glm::vec2(1.0f, 0.0f));
      colors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

      float phiStep = utilities::Angle::pi / stackCount;
      float thetaStep = utilities::Angle::tau / sliceCount;

      //! Vertices.
      for (uint32_t i = 1; i <= stackCount - 1; ++i)
      {
        float phi = i * phiStep;

        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
          float theta = j * thetaStep;

          glm::vec3 p;
          p.x = radius*sinf(phi)*cosf(theta);
          p.y = radius*cosf(phi);
          p.z = radius*sinf(phi)*sinf(theta);

          glm::vec3 n;
          n = glm::normalize(p);

          glm::vec2 uv;
          uv.x = theta / utilities::Angle::tau;
          uv.y = phi / utilities::Angle::pi;

          vertices.push_back(p);
          normals.push_back(n);
          uvs.push_back(uv);
          colors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
      }

      vertices.push_back(glm::vec3(0.0f, -radius, 0.0f));
      normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
      uvs.push_back(glm::vec2(1.0f, 0.0f));
      colors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

      //! Indices.
      for (uint32_t i = 1; i <= sliceCount; ++i)
      {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i);
      }

      uint32_t baseIndex = 1;
      uint32_t ringVertexCount = sliceCount + 1;
      for (uint32_t i = 0; i < stackCount - 2; ++i)
      {
        for (uint32_t j = 0; j < sliceCount; ++j)
        {
          indices.push_back(baseIndex + i*ringVertexCount + j);
          indices.push_back(baseIndex + i*ringVertexCount + j + 1);
          indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

          indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
          indices.push_back(baseIndex + i*ringVertexCount + j + 1);
          indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
        }
      }

      uint32_t southPoleIndex = (uint32_t)vertices.size() - 1;
      baseIndex = southPoleIndex - ringVertexCount;
      for (uint32_t i = 0; i < sliceCount; ++i)
      {
        indices.push_back(southPoleIndex);
        indices.push_back(baseIndex + i);
        indices.push_back(baseIndex + i + 1);
      }

      //! Combine.
      glm::vec3 min(FLT_MAX), max(FLT_MIN);
      getMinMax(vertices, min, max);
      Mesh sphere({ 
				{ kPositions, vertices },
				{ kNormals,   normals },
				{ kTexCoords, uvs },
				{ kColours,   colors },
				{ kTangents,  Buffer() },
				{ kJoints,    Buffer() },
				{ kWeights,   Buffer() },
				{ kIndices,   indices }
			},
        Vector<SubMesh>{ SubMesh{
        { 
					{ kPositions, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kNormals,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kTexCoords, SubMesh::Offset(0, vertices.size(), sizeof(glm::vec2)) },
					{ kColours,   SubMesh::Offset(0, vertices.size(), sizeof(glm::vec4)) },
					{ kTangents,  SubMesh::Offset(0, vertices.size(), sizeof(glm::vec3)) },
					{ kJoints,    SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
					{ kWeights,   SubMesh::Offset(0, 0, sizeof(glm::vec4))               },
					{ kIndices,   SubMesh::Offset(0, indices.size(), sizeof(uint32_t))   } },
        min, max }}
      );

      sphere.recalculateTangents();

      return sphere;
    }

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		VioletMeshHandle lambda::asset::MeshManager::create(Name name)
		{
			VioletMeshHandle handle;

			auto it = mesh_cache_.find(name.getHash());
			if (it == mesh_cache_.end())
			{
				handle = VioletMeshHandle(foundation::Memory::construct<Mesh>(), name);
				mesh_cache_.insert(eastl::make_pair(name.getHash(), handle.get()));
			}
			else
				handle = VioletMeshHandle(it->second, name);

			return handle;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		VioletMeshHandle lambda::asset::MeshManager::create(Name name, Mesh mesh)
		{
			VioletMeshHandle handle;

			auto it = mesh_cache_.find(name.getHash());
			if (it == mesh_cache_.end())
			{
				handle = VioletMeshHandle(foundation::Memory::construct<Mesh>(mesh), name);
				mesh_cache_.insert(eastl::make_pair(name.getHash(), handle.get()));
			}
			else
				handle = VioletMeshHandle(it->second, name);

			return handle;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		VioletMeshHandle MeshManager::create(Name name, VioletMesh mesh)
		{
			Vector<glm::vec3> pos(mesh.data.pos.data.size() / sizeof(glm::vec3));
			Vector<glm::vec3> nor(mesh.data.nor.data.size() / sizeof(glm::vec3));
			Vector<glm::vec2> tex(mesh.data.tex.data.size() / sizeof(glm::vec2));
			Vector<glm::vec4> col(mesh.data.col.data.size() / sizeof(glm::vec4));
			Vector<glm::vec4> ltn(mesh.data.tan.data.size() / sizeof(glm::vec4));
			Vector<glm::vec4> joi(mesh.data.joi.data.size() / sizeof(glm::vec4));
			Vector<glm::vec4> wei(mesh.data.wei.data.size() / sizeof(glm::vec4));
			Vector<uint32_t>  idx(mesh.data.idx.data.size() / sizeof(uint32_t));
			Vector<glm::vec3> tan(ltn.size());

			memcpy((void*)pos.data(), mesh.data.pos.data.data(), mesh.data.pos.data.size());
			memcpy((void*)nor.data(), mesh.data.nor.data.data(), mesh.data.nor.data.size());
			memcpy((void*)tex.data(), mesh.data.tex.data.data(), mesh.data.tex.data.size());
			memcpy((void*)col.data(), mesh.data.col.data.data(), mesh.data.col.data.size());
			memcpy((void*)ltn.data(), mesh.data.tan.data.data(), mesh.data.tan.data.size());
			memcpy((void*)joi.data(), mesh.data.joi.data.data(), mesh.data.joi.data.size());
			memcpy((void*)wei.data(), mesh.data.wei.data.data(), mesh.data.wei.data.size());
			memcpy((void*)idx.data(), mesh.data.idx.data.data(), mesh.data.idx.data.size());
			for (uint64_t i = 0u; i < tan.size(); ++i)
				memcpy((glm::vec3*)tan.data() + i, (glm::vec4*)ltn.data() + i, sizeof(float) * 3u);

			Vector<asset::SubMesh> sub_meshes;
			for (const VioletSubMesh& m : mesh.meshes)
			{
				asset::SubMesh sm;
				sm.io.parent = m.parent;
				sm.io.topology = m.topology;
				sm.io.translation = m.translation;
				sm.io.rotation = m.rotation;
				sm.io.scale = m.scale;
				sm.io.tex_alb = m.tex_alb;
				sm.io.tex_nor = m.tex_nor;
				sm.io.tex_dmra = m.tex_dmra;
				sm.io.tex_emi = m.tex_emi;
				sm.io.double_sided = m.double_sided;
				sm.io.metallic = m.metallic;
				sm.io.roughness = m.roughness;
				sm.io.emissiveness = m.emissiveness;
				sm.io.colour = m.colour;

				if (m.pos >= 0) sm.offsets[asset::MeshElements::kPositions] = asset::SubMesh::Offset{
					mesh.data.pos.segments.at(m.pos).offset,
					mesh.data.pos.segments.at(m.pos).count,
					mesh.data.pos.segments.at(m.pos).stride
				};
				if (m.nor >= 0) sm.offsets[asset::MeshElements::kNormals] = asset::SubMesh::Offset{
					mesh.data.nor.segments.at(m.nor).offset,
					mesh.data.nor.segments.at(m.nor).count,
					mesh.data.nor.segments.at(m.nor).stride
				};
				if (m.col >= 0) sm.offsets[asset::MeshElements::kColours] = asset::SubMesh::Offset{
					mesh.data.col.segments.at(m.col).offset,
					mesh.data.col.segments.at(m.col).count,
					mesh.data.col.segments.at(m.col).stride
				};
				if (m.tan >= 0) sm.offsets[asset::MeshElements::kTangents] = asset::SubMesh::Offset{
					mesh.data.tan.segments.at(m.tan).offset / mesh.data.tan.segments.at(m.tan).stride * sizeof(glm::vec3),
					//mesh.data.tan.segments.at(m.tan].offset,
					mesh.data.tan.segments.at(m.tan).count,
					sizeof(glm::vec3) // mesh.data.tan.segments.at(m.tan).stride
				};
				if (m.tex >= 0) sm.offsets[asset::MeshElements::kTexCoords] = asset::SubMesh::Offset{
					mesh.data.tex.segments.at(m.tex).offset,
					mesh.data.tex.segments.at(m.tex).count,
					mesh.data.tex.segments.at(m.tex).stride
				};
				if (m.joi >= 0) sm.offsets[asset::MeshElements::kJoints] = asset::SubMesh::Offset{
					mesh.data.joi.segments.at(m.joi).offset,
					mesh.data.joi.segments.at(m.joi).count,
					mesh.data.joi.segments.at(m.joi).stride
				};
				if (m.wei >= 0) sm.offsets[asset::MeshElements::kWeights] = asset::SubMesh::Offset{
					mesh.data.wei.segments.at(m.wei).offset,
					mesh.data.wei.segments.at(m.wei).count,
					mesh.data.wei.segments.at(m.wei).stride
				};
				if (m.idx >= 0) sm.offsets[asset::MeshElements::kIndices] = asset::SubMesh::Offset{
					mesh.data.idx.segments.at(m.idx).offset,
					mesh.data.idx.segments.at(m.idx).count,
					mesh.data.idx.segments.at(m.idx).stride
				};
				sm.min = m.aabb_min;
				sm.max = m.aabb_max;
				sub_meshes.push_back(sm);
			}

			Vector<asset::VioletTextureHandle> textures;
			for (const String& texture : mesh.data.tex_alb)
				textures.push_back(asset::TextureManager::getInstance()->get(texture));
			for (const String& texture : mesh.data.tex_nrm)
				textures.push_back(asset::TextureManager::getInstance()->get(texture));
			for (const String& texture : mesh.data.tex_dmra)
				textures.push_back(asset::TextureManager::getInstance()->get(texture));
			for (const String& texture : mesh.data.tex_emi)
				textures.push_back(asset::TextureManager::getInstance()->get(texture));

			asset::Mesh m;
			m.set(asset::MeshElements::kPositions, eastl::move(pos));
			m.set(asset::MeshElements::kNormals, eastl::move(nor));
			m.set(asset::MeshElements::kTexCoords, eastl::move(tex));
			m.set(asset::MeshElements::kColours, eastl::move(col));
			m.set(asset::MeshElements::kTangents, eastl::move(tan));
			m.set(asset::MeshElements::kJoints, eastl::move(joi));
			m.set(asset::MeshElements::kWeights, eastl::move(wei));
			m.set(asset::MeshElements::kIndices, eastl::move(idx));
			m.setSubMeshes(eastl::move(sub_meshes));
			m.setAttachedTextures(eastl::move(textures));
			m.setAttachedTextureCount(glm::uvec4(
				mesh.data.tex_alb.size(),
				mesh.data.tex_nrm.size(),
				mesh.data.tex_dmra.size(),
				mesh.data.tex_emi.size()
			));

			return create(name, m);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		VioletMeshHandle lambda::asset::MeshManager::get(Name name)
		{
			uint64_t hash = manager_.GetHash(FileSystem::MakeRelative(name.getName()));
			LMB_ASSERT(manager_.HasHeader(hash), "Could not find mesh: %s", name.getName().c_str());
			return get(hash);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		VioletMeshHandle MeshManager::get(uint64_t hash)
		{
			LMB_ASSERT(manager_.HasHeader(hash), "Could not find mesh: %llu", hash);
			VioletMesh mesh = manager_.GetMesh(hash, true);
			return create(mesh.file, mesh);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		VioletMeshHandle MeshManager::getFromCache(Name name)
		{
			auto it = mesh_cache_.find(name.getHash());
			LMB_ASSERT(it != mesh_cache_.end(), "Could not find mseh in cache: %s", name.getName().c_str());
			return VioletMeshHandle(it->second, name);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void lambda::asset::MeshManager::destroy(Mesh* mesh, const size_t& hash)
		{
			if (mesh_cache_.empty())
				return;

			auto it = mesh_cache_.find(hash);
			if (it != mesh_cache_.end())
				mesh_cache_.erase(it);

			foundation::Memory::destruct<Mesh>(mesh);
			renderer_->destroyMesh(hash);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		MeshManager* lambda::asset::MeshManager::getInstance()
		{
			static MeshManager* s_instance =
				foundation::Memory::construct<MeshManager>();

			return s_instance;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void lambda::asset::MeshManager::setRenderer(platform::IRenderer* renderer)
		{
			getInstance()->renderer_ = renderer;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		MeshManager::~MeshManager()
		{
			while (!mesh_cache_.empty())
				destroy(mesh_cache_.begin()->second, mesh_cache_.begin()->first);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		VioletMeshManager& MeshManager::getManager()
		{
			return manager_;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		const VioletMeshManager& MeshManager::getManager() const
		{
			return manager_;
		}
  }
}