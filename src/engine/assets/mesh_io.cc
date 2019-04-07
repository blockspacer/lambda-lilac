#include "mesh_io.h"
#include <containers/containers.h>
#include <fstream>
#include <unordered_map>
#include <iostream>
#include "utils/decompose_matrix.h"
#include "texture.h"
#include <utils/console.h>
#include <utils/file_system.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#if defined VIOLET_ASSET_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#if VIOLET_WIN32
#pragma warning(push, 0)
#endif
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
//#define TINYGLTF_NO_STB_IMAGE
#include <tiny_gltf.h>
#if VIOLET_WIN32
#pragma warning(pop)
#endif

namespace lambda
{
  namespace io
  {
    const char MeshIO::Format::kMagicHeader[] = { 'L', 'M', 'B' };
    const char MeshIO::Format::kInvalidHeader[] = { 'I', 'N', 'V' };

    String getPathExtension(const String& file)
    {
      if (file.find_last_of(".") != String::npos)
      {
        return file.substr(file.find_last_of(".") + 1);
      }
      return "";
    }

    //--------------------------------------------------------------------
    //|                                                                  |
    //|                                                                  |
    //|                                                                  |
    //|                                                                  |
    //--------------------------------------------------------------------

	void write(Vector<char>& data, const char* t, size_t len)
	{
		data.resize(data.size() + len);
		memcpy((void*)(data.data() + (data.size() - len)), t, len);
	}
	template<typename T>
    void write(Vector<char>& data, const T& t)
    {
	  write(data, (const char*)&t, sizeof(T));
    }
    void writeHeader(Vector<char>& data, const MeshIO::DataInfo& info)
    {
      MeshIO::Format::DataHeader header;
      header.data_size = info.data.size();
      header.segment_count = info.segments.size();
      write(data, header);

      write(data, (const char*)info.data.data(), info.data.size());

      for (const MeshIO::DataSegment& segment : info.segments)
      {
        MeshIO::Format::SegmentHeader segment_header;
        segment_header.count = segment.count;
        segment_header.offset = segment.offset;
        segment_header.stride = segment.stride;
        write(data, segment_header);
      }
    }
    void writeHeader(Vector<char>& data, const Vector<MeshIO::SubMesh>& meshes)
    {
      MeshIO::Format::ModelHeader header;
      header.model_count = meshes.size();
      write(data, header);

      for (const MeshIO::SubMesh& mesh : meshes)
      {
        write(data, (const char*)&mesh, sizeof(MeshIO::SubMesh));
      }
    }
    void startWriting(Vector<char>& data)
    {
      write(data, MeshIO::Format::kInvalidHeader, 3);
    }
    void finalizeWriting(Vector<char>& data)
    {
	  memcpy(data.data(), MeshIO::Format::kMagicHeader, 3);
    }

	Vector<char> MeshIO::save(const Mesh& mesh)
    {
	  Vector<char> data;
      startWriting(data);

      writeHeader(data, mesh.data.pos);
      writeHeader(data, mesh.data.nor);
      writeHeader(data, mesh.data.tan);
      writeHeader(data, mesh.data.col);
      writeHeader(data, mesh.data.tex);
      writeHeader(data, mesh.data.joi);
      writeHeader(data, mesh.data.wei);
      writeHeader(data, mesh.data.idx);
      writeHeader(data, mesh.meshes);

      finalizeWriting(data);
	  return data;
    }

    //--------------------------------------------------------------------
    //|                                                                  |
    //|                                                                  |
    //|                                                                  |
    //|                                                                  |
    //--------------------------------------------------------------------

//#define NEW_METHOD
#ifdef NEW_METHOD
    template<typename T>
    Vector<T> copy(MeshIO::DataInfo& data)
    {
      Vector<T> vector(data.data.size() / sizeof(T));
      memcpy((void*)vector.data(), (void*)data.data.data(), data.data.size());
      data.data.resize(0u);
      return eastl::move(vector);
    }

    asset::Mesh MeshIO::asAsset(Mesh& mesh)
    {
      Vector<glm::vec3> pos = eastl::move(copy<glm::vec3>(mesh.data.pos));
      Vector<glm::vec3> nor = eastl::move(copy<glm::vec3>(mesh.data.nor));
      Vector<glm::vec2> tex = eastl::move(copy<glm::vec2>(mesh.data.tex));
      Vector<glm::vec4> col = eastl::move(copy<glm::vec4>(mesh.data.col));
      Vector<glm::vec4> ltn = eastl::move(copy<glm::vec4>(mesh.data.tan));
      Vector<glm::vec4> joi = eastl::move(copy<glm::vec4>(mesh.data.joi));
      Vector<glm::vec4> wei = eastl::move(copy<glm::vec4>(mesh.data.wei));
      Vector<uint32_t>  idx = eastl::move(copy<uint32_t>(mesh.data.idx));
      Vector<glm::vec3> tan(ltn.size());
      for (uint64_t i = 0u; i < tan.size(); ++i)
        memcpy((glm::vec3*)tan.data() + i, (glm::vec4*)ltn.data() + i, sizeof(float) * 3u);
      ltn.resize(0u);
#else
    asset::Mesh MeshIO::asAsset(Mesh& mesh)
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
#endif

      Vector<asset::SubMesh> sub_meshes;
      for (const MeshIO::SubMesh& m : mesh.meshes)
      {
        asset::SubMesh sm;
        sm.io.parent       = m.parent;
        sm.io.topology     = m.topology;
        sm.io.translation  = m.translation;
        sm.io.rotation     = m.rotation;
        sm.io.scale        = m.scale;
        sm.io.tex_alb      = m.tex_alb;
        sm.io.tex_nor      = m.tex_nor;
        sm.io.tex_mrt      = m.tex_mrt;
        sm.io.double_sided = m.double_sided;
        sm.io.metallic     = m.metallic;
        sm.io.roughness    = m.roughness;
        sm.io.colour       = m.colour;

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
			textures.insert(textures.end(), mesh.data.tex_alb.begin(),  mesh.data.tex_alb.end());
			textures.insert(textures.end(), mesh.data.tex_nrm.begin(),  mesh.data.tex_nrm.end());
			textures.insert(textures.end(), mesh.data.tex_dmra.begin(), mesh.data.tex_dmra.end());

      asset::Mesh m;
      m.set(asset::MeshElements::kPositions, eastl::move(pos));
      m.set(asset::MeshElements::kNormals,   eastl::move(nor));
      m.set(asset::MeshElements::kTexCoords, eastl::move(tex));
      m.set(asset::MeshElements::kColours,   eastl::move(col));
      m.set(asset::MeshElements::kTangents,  eastl::move(tan));
      m.set(asset::MeshElements::kJoints,    eastl::move(joi));
      m.set(asset::MeshElements::kWeights,   eastl::move(wei));
      m.set(asset::MeshElements::kIndices,   eastl::move(idx));
      m.setSubMeshes(eastl::move(sub_meshes));
      m.setAttachedTextures(eastl::move(textures));
      m.setAttachedTextureCount(glm::uvec3(
        mesh.data.tex_alb.size(),
        mesh.data.tex_nrm.size(),
        mesh.data.tex_dmra.size()
      ));

      return m;
    }

    //--------------------------------------------------------------------
    //|                                                                  |
    //|                                                                  |
    //|                                                                  |
    //|                                                                  |
    //--------------------------------------------------------------------

    MeshIO::Mesh MeshIO::load(const String& path)
    {
      String extension(getPathExtension(path));
      eastl::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
      String full_path = FileSystem::FullFilePath(path);

      if (extension == "gltf" || extension == "glb")
      {
#if defined USE_ASSIMP and defined VIOLET_ASSET_ASSIMP
        return loadMeshAssimp(full_path.c_str());
#else
        return loadMeshGLTF(full_path.c_str());
#endif
      }
      else
      {
		  Mesh mesh;
		  loadMeshCustom(path, mesh);
		  return mesh;
      }
    }

    //--------------------------------------------------------------------
    //|                                                                  |
    //|                                                                  |
    //|                                                                  |
    //|                                                                  |
    //--------------------------------------------------------------------

	void read(const Vector<char>& data, size_t& offset, char* t, size_t len)
	{
		memcpy(t, data.data() + offset, len);
		offset += len;
	}

    template<typename T>
    void read(const Vector<char>& data, size_t& offset, T& t)
    {
		read(data, offset, (char*)&t, sizeof(T));
    }
    void readHeader(const Vector<char>& data, size_t& offset, MeshIO::DataInfo& info)
    {
      MeshIO::Format::DataHeader header;
      read(data, offset, header);

      info.data.resize(header.data_size);
      read(data, offset, (char*)info.data.data(), header.data_size);

      for (size_t i = 0; i < header.segment_count; ++i)
      {
        MeshIO::Format::SegmentHeader segment_header;
        read(data, offset, segment_header);

        MeshIO::DataSegment segment;
        segment.count = segment_header.count;
        segment.offset = segment_header.offset;
        segment.stride = segment_header.stride;

        info.segments.push_back(segment);
      }
    }
    void readHeader(const Vector<char>& data, size_t& offset, Vector<MeshIO::SubMesh>& meshes)
    {
      MeshIO::Format::ModelHeader header;
      read(data, offset, header);

      for (size_t i = 0; i < header.model_count; ++i)
      {
        MeshIO::SubMesh mesh;
        read(data, offset, mesh);
        meshes.push_back(mesh);
      }
    }

    bool validateFile(const Vector<char>& data, size_t& offset)
    {
	  char magic_header[3];
	  read(data, offset, magic_header, 3);

      return
        magic_header[0] == MeshIO::Format::kMagicHeader[0] &&
        magic_header[1] == MeshIO::Format::kMagicHeader[1] &&
        magic_header[2] == MeshIO::Format::kMagicHeader[2];
    }

    bool MeshIO::loadMeshCustom(const String& path, Mesh& mesh)
    {
	  mesh = Mesh{};

	  size_t offset = 0;
	  Vector<char> data = FileSystem::FileToVector(path);

      if (!validateFile(data, offset))
      {
        foundation::Error("[MESH] Failed to load mesh\n");
        return false;
      }

      readHeader(data, offset, mesh.data.pos);
      readHeader(data, offset, mesh.data.nor);
      readHeader(data, offset, mesh.data.tan);
      readHeader(data, offset, mesh.data.col);
      readHeader(data, offset, mesh.data.tex);
      readHeader(data, offset, mesh.data.joi);
      readHeader(data, offset, mesh.data.wei);
      readHeader(data, offset, mesh.data.idx);
      readHeader(data, offset, mesh.meshes);

      return true;
    }

    //--------------------------------------------------------------------
    //|                                                                  |
    //|                                                                  |
    //|                                                                  |
    //|                                                                  |
    //--------------------------------------------------------------------

    struct AccessorMemory
    {
      Vector<int> pos;
      Vector<int> nor;
      Vector<int> tan;
      Vector<int> col;
      Vector<int> tex;
      Vector<int> joi;
      Vector<int> wei;
      Vector<int> idx;
      Vector<int> tex_alb;
      Vector<int> tex_nor;
      Vector<int> tex_mrt;
    };
    struct AccessorMemoryConverter
    {
      UnorderedMap<int, int> pos;
      UnorderedMap<int, int> nor;
      UnorderedMap<int, int> tan;
      UnorderedMap<int, int> col;
      UnorderedMap<int, int> tex;
      UnorderedMap<int, int> idx;
      UnorderedMap<int, int> joi;
      UnorderedMap<int, int> wei;
      UnorderedMap<int, int> tex_alb;
      UnorderedMap<int, int> tex_nor;
      UnorderedMap<int, int> tex_mrt;
    };

    size_t getElementType(int type, int component_type)
    {
      size_t elem = 1;
      size_t scalar = 1;
      switch (type)
      {
      case TINYGLTF_TYPE_SCALAR:
        scalar = 1;
        break;
      case TINYGLTF_TYPE_VEC2:
        scalar = 2;
        break;
      case TINYGLTF_TYPE_VEC3:
        scalar = 3;
        break;
      case TINYGLTF_TYPE_VEC4:
        scalar = 4;
        break;
      case TINYGLTF_TYPE_MAT2:
        scalar = 4;
        break;
      case TINYGLTF_TYPE_MAT3:
        scalar = 9;
        break;
      case TINYGLTF_TYPE_MAT4:
        scalar = 16;
        break;
      case TINYGLTF_TYPE_VECTOR:
        foundation::Error("MeshIO: scalar type is vector. How do you handle this?\n");
        break;
      case TINYGLTF_TYPE_MATRIX:
        foundation::Error("MeshIO: scalar type is matrix. How do you handle this?\n");
        break;
      default:
        foundation::Error("MeshIO: No scalar was specified!\n");
        break;
      }
      switch (component_type)
      {
      case TINYGLTF_COMPONENT_TYPE_BYTE:
        elem = sizeof(char);
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        elem = sizeof(unsigned char);
        break;
      case TINYGLTF_COMPONENT_TYPE_SHORT:
        elem = sizeof(short);
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        elem = sizeof(unsigned short);
        break;
      case TINYGLTF_COMPONENT_TYPE_INT:
        elem = sizeof(int);
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        elem = sizeof(unsigned int);
        break;
      case TINYGLTF_COMPONENT_TYPE_FLOAT:
        elem = sizeof(float);
        break;
      case TINYGLTF_COMPONENT_TYPE_DOUBLE:
        elem = sizeof(double);
        break;
      default:
        foundation::Error("MeshIO: No type was specified!\n");
        break;
      }
      return elem * scalar;
    }
    template<typename T>
    void insert(Vector<T>& v, const T& t)
    {
      if (eastl::find(v.begin(), v.end(), t) == v.end())
      {
        v.push_back(t);
      }
    }
    template<typename V>
    V contains(const std::map<std::string, V>& m, const String& v)
    {
      if (m.empty())
      {
        return -1;
      }
			std::string std_str = stlString(v);
      auto it = m.find(std_str);
      if (it != m.end())
      {
        return it->second;
      }
      else
      {
        return -1;
      }
    }
    template<typename V>
    bool containsRaw(const std::map<std::string, V>& m, const String& v, V& ret)
    {
      if (m.empty())
      {
        return false;
      }
			std::string std_str = stlString(v);
      auto it = m.find(std_str);
      if (it != m.end())
      {
        ret = it->second;
        return true;
      }
      else
      {
        return false;
      }
    }
    template<typename T>
    void erase(Vector<T>& v, const T& t)
    {
      v.erase(eastl::find(v.begin(), v.end(), t));
    }
    template<typename T>
    void eraseIf(Vector<T>& v, const T& t)
    {
      const auto& it = eastl::find(v.begin(), v.end(), t);
      if (it != v.end())
      {
        v.erase(it);
      }
    }

    tinygltf::Model loadWorld(const String& path);
    void addNode(const tinygltf::Model& model, AccessorMemory& memory, const int& n);
    void addNode(const tinygltf::Model& model, const int& n, const int& current_mesh, Vector<MeshIO::SubMesh>& meshes, const AccessorMemoryConverter& converter);
    void getData(const tinygltf::Model& model, const Vector<int>& accessors, MeshIO::DataInfo& info);
    void getDataIdx(const tinygltf::Model& model, const Vector<int>& accessors, MeshIO::DataInfo& info);
    void getTexture(const String& base_path, const tinygltf::Model& model, const Vector<int>& textures, Vector<asset::VioletTextureHandle>& handles);

#ifdef ANIMATIONS
    struct Channel
    {
      enum class Type
      {
        kTranslation,
        kRotation,
        kScale,
        kWeights
      } type;

      int node;
      int sampler;
    };
    struct Sampler
    {
      enum class Interpolation
      {
        kLinear,
        kStep,
        kCatmullromspline,
        kCubicSpline
      } interpolation;
      int input;
      int output;
    };
    struct Animation
    {
      String name;
      Vector<Channel> channels;
      Vector<Sampler> samplers;
    };

    void fillAnimations(tinygltf::Model model)
    {
      Vector<Animation> animations;
      for (const auto& animation : model.animations)
      {
        Animation a;
        a.name = animation.name;
        for (const auto& channel : animation.channels)
        {
          Channel c;
          c.sampler = channel.sampler;
          c.node = channel.target_node;
          if (channel.target_path == "translation") c.type = Channel::Type::kTranslation;
          if (channel.target_path == "rotation")    c.type = Channel::Type::kRotation;
          if (channel.target_path == "scale")       c.type = Channel::Type::kScale;
          if (channel.target_path == "weights")     c.type = Channel::Type::kWeights;
          a.channels.push_back(c);
        }
        for (const auto& sampler : animation.samplers)
        {
          Sampler s;
          s.input  = sampler.input;
          s.output = sampler.output;
          if (sampler.interpolation == "LINEAR")           s.interpolation = Sampler::Interpolation::kLinear;
          if (sampler.interpolation == "STEP")             s.interpolation = Sampler::Interpolation::kStep;
          if (sampler.interpolation == "CATMULLROMSPLINE") s.interpolation = Sampler::Interpolation::kCatmullromspline;
          if (sampler.interpolation == "CUBICSPLINE")      s.interpolation = Sampler::Interpolation::kCubicSpline;
          a.samplers.push_back(s);
        }
      }
    }
#endif

    MeshIO::Mesh MeshIO::loadMeshGLTF(const String& path)
    {
      Mesh converted_mesh;
      AccessorMemory memory;
      AccessorMemoryConverter converter;

      // Get the world.
      tinygltf::Model model = loadWorld(path);
      
      // Fill the accessor memory by looping over all nodes.
			LMB_ASSERT(model.scenes.size() > 0, "GLTF file did not contain a scene");
      for (tinygltf::Scene scene : model.scenes)
        for (int node : scene.nodes)
          addNode(model, memory, node);

			String base_path = path;
			eastl_size_t last_slash = base_path.find_last_of("/");
			if (last_slash != String::npos)
				base_path = base_path.substr(0, last_slash + 1);

			{
				loadWorld(path);
			}

      // Erase the invalid accessors.
      eraseIf(memory.pos, -1);
      eraseIf(memory.nor, -1);
      eraseIf(memory.tan, -1);
      eraseIf(memory.col, -1);
      eraseIf(memory.tex, -1);
      eraseIf(memory.idx, -1);
      eraseIf(memory.joi, -1);
      eraseIf(memory.wei, -1);
      eraseIf(memory.tex_alb, -1);
      eraseIf(memory.tex_nor, -1);
      eraseIf(memory.tex_mrt, -1);

      // Get the used accessors.
      getData(model, memory.pos, converted_mesh.data.pos);
      getData(model, memory.nor, converted_mesh.data.nor);
      getData(model, memory.tan, converted_mesh.data.tan);
      getData(model, memory.col, converted_mesh.data.col);
      getData(model, memory.tex, converted_mesh.data.tex);
      getData(model, memory.joi, converted_mesh.data.joi);
      getData(model, memory.wei, converted_mesh.data.wei);
      getDataIdx(model, memory.idx, converted_mesh.data.idx);
      getTexture(base_path, model, memory.tex_alb, converted_mesh.data.tex_alb);
      getTexture(base_path, model, memory.tex_nor, converted_mesh.data.tex_nrm);
      getTexture(base_path, model, memory.tex_mrt, converted_mesh.data.tex_dmra);

      auto convert = [](const Vector<int>& v, UnorderedMap<int, int>& m) { m.insert(eastl::make_pair(-1, -1)); for (int i = 0; i < (int)v.size(); ++i) { m.insert(eastl::make_pair(v.at(i), i)); } };
      convert(memory.pos, converter.pos);
      convert(memory.nor, converter.nor);
      convert(memory.tan, converter.tan);
      convert(memory.col, converter.col);
      convert(memory.tex, converter.tex);
      convert(memory.joi, converter.joi);
      convert(memory.wei, converter.wei);
      convert(memory.idx, converter.idx);
      convert(memory.tex_alb, converter.tex_alb);
      convert(memory.tex_nor, converter.tex_nor);
      convert(memory.tex_mrt, converter.tex_mrt);

      converted_mesh.meshes.push_back(MeshIO::SubMesh());
      for (tinygltf::Scene scene : model.scenes)
      {
        for (int node : scene.nodes)
        {
          converted_mesh.meshes.push_back(MeshIO::SubMesh());
          addNode(model, node, (int)converted_mesh.meshes.size() - 1, converted_mesh.meshes, converter);
        }
      }

      return converted_mesh;
    }

#if defined VIOLET_ASSET_ASSIMP
    void processMesh(MeshIO::Mesh& final_mesh, const aiMesh* mesh, const aiScene* scene)
    {
      Vector<glm::vec3> positions(mesh->mNumVertices);
      Vector<glm::vec3> normals(mesh->HasNormals() ? mesh->mNumVertices : 0u);
      Vector<glm::vec3> tangents(mesh->HasTangentsAndBitangents() ? mesh->mNumVertices : 0u);
      Vector<glm::vec4> colours(mesh->HasVertexColors(0u) ? mesh->mNumVertices : 0u);
      Vector<glm::vec2> tex_coords(mesh->HasTextureCoords(0u) ? mesh->mNumVertices : 0u);
      Vector<unsigned int> indices;

      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
        const auto& position = mesh->mVertices[i];
        positions[i] = glm::vec3(position.x, position.y, position.z);
        if (mesh->HasNormals())
        {
          const auto& normal = mesh->mNormals[i];
          normals[i] = glm::vec3(normal.x, normal.y, normal.z);
        }
        if (mesh->HasTangentsAndBitangents())
        {
          const auto& tangent = mesh->mTangents[i];
          tangents[i] = glm::vec3(tangent.x, tangent.y, tangent.z);
        }
        if (mesh->HasVertexColors(0u))
        {
          const auto& colour = mesh->mColors[0u][i];
          colours[i] = glm::vec4(colour.r, colour.g, colour.b, colour.a);
        }
        if (mesh->HasTextureCoords(0u))
        {
          const auto& tex_coord = mesh->mTextureCoords[0u][i];
          tex_coords[i] = glm::vec2(tex_coord.x, tex_coord.y);
        }
      }

      for (size_t i = 0u; i < mesh->mNumFaces; ++i)
      {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0u; j < face.mNumIndices; ++j)
        {
          indices.push_back(face.mIndices[j]);
        }
      }

      if (mesh->mMaterialIndex >= 0)
      {
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        MeshIO::TextureInfo tex_alb;
        MeshIO::TextureInfo tex_nor;
        MeshIO::TextureInfo tex_mrt;

        for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); ++i)
        {
          aiString str;
          material->GetTexture(aiTextureType_DIFFUSE, i,&str);
          int texture_index = std::stoi(str.C_Str() + 1u);
          const aiTexture* texture = scene->mTextures[texture_index];
          if(texture != nullptr)
          {
            size_t size = tex_alb.data.size();

            unsigned char* data;
            MeshIO::TextureSegment segment;
            segment.offset = size;
            
            if (texture->mHeight == 0u)
            {
              int width, height, bpp;
              data = stbi_load_from_memory((const stbi_uc*)texture->pcData, texture->mWidth,&width,&height,&bpp, STBI_rgb_alpha);
              segment.bpp    = bpp;
              segment.width  = width;
              segment.height = height;
            }
            else
            {
              data = (unsigned char*)texture->pcData;
              segment.bpp    = 4u;
              segment.height = texture->mHeight;
              segment.width  = texture->mWidth;
            }

            tex_alb.data.insert(tex_alb.data.end(), segment.width * segment.height * segment.bpp, '\0');
            memcpy(tex_alb.data.data() + size, data, segment.width * segment.height * segment.bpp);
            tex_alb.segments.push_back(segment);
          }
        }
      }
    }

    void processNode(MeshIO::Mesh& final_mesh, const aiNode* node, const aiScene* scene)
    {
      // process all the node's meshes (if any)
      for (unsigned int i = 0; i < node->mNumMeshes; i++)
      {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(final_mesh, mesh, scene);
      }
      // then do the same for each of its children
      for (unsigned int i = 0; i < node->mNumChildren; i++)
      {
        processNode(final_mesh, node->mChildren[i], scene);
      }
    }

    MeshIO::Mesh MeshIO::loadMeshAssimp(const String& path)
    {
      Assimp::Importer importer;
      const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate /*| aiProcess_FlipUVs*/);

      if (!scene || scene->mFlags& AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
      {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
      }
      
      Mesh mesh;
      processNode(mesh, scene->mRootNode, scene);

      return mesh;
    }
#endif

    void getData(const tinygltf::Model& model, const Vector<int>& accessors, MeshIO::DataInfo& info)
    {
      for (const int& a : accessors)
      {
        const tinygltf::Accessor&   accessor    = model.accessors.at(a);
        const tinygltf::BufferView& buffer_view = model.bufferViews.at(accessor.bufferView);
        const tinygltf::Buffer&     buffer      = model.buffers.at(buffer_view.buffer);

        size_t offset = accessor.byteOffset + buffer_view.byteOffset;
        size_t count  = accessor.count;
        size_t stride = getElementType(accessor.type, accessor.componentType);

        info.segments.push_back({
          /* offset */ info.data.size(),
          /* count  */ count,
          /* stride */ stride
        });

        Vector<unsigned char> data(count * stride);
        memcpy(data.data(), buffer.data.data() + offset, count * stride);
        info.data.insert(info.data.end(), data.begin(), data.end());
      }
    }
    void getDataIdx(const tinygltf::Model& model, const Vector<int>& accessors, MeshIO::DataInfo& info)
    {
      for (const int& a : accessors)
      {
        const tinygltf::Accessor&   accessor    = model.accessors.at(a);
        const tinygltf::BufferView& buffer_view = model.bufferViews.at(accessor.bufferView);
        const tinygltf::Buffer&     buffer      = model.buffers.at(buffer_view.buffer);

        size_t offset = accessor.byteOffset + buffer_view.byteOffset;
        size_t count  = accessor.count;
        size_t stride = getElementType(accessor.type, accessor.componentType);

        info.segments.push_back({
          /* offset */ info.data.size(),
          /* count  */ count,
          /* stride */ sizeof(uint32_t)
        });

        if (stride == sizeof(uint32_t))
        {
          Vector<unsigned char> data(count * stride);
          memcpy(data.data(), buffer.data.data() + offset, count * stride);
          info.data.insert(info.data.end(), data.begin(), data.end());
        }
        else
        {
          Vector<unsigned char> data(count * stride);
          memcpy(data.data(), buffer.data.data() + offset, count * stride);

          if (stride == sizeof(uint16_t))
          {
            Vector<uint32_t> converted(data.size() / sizeof(uint16_t));

            for (size_t i = 0; i < data.size(); i += sizeof(uint16_t))
            {
              converted.at(i / sizeof(uint16_t)) = (data.at(i + 1) << 8) + data.at(i);
            }

            data.resize(converted.size() * sizeof(uint32_t));
            memcpy((void*)data.data(), converted.data(), data.size());

            info.data.insert(info.data.end(), data.begin(), data.end());
          }
          else if (stride == sizeof(uint8_t))
          {
            Vector<uint32_t> converted(data.size());

            for (size_t i = 0; i < data.size(); ++i)
            {
              converted.at(i) = (uint32_t)data.at(i);
            }

            data.resize(converted.size() * sizeof(uint32_t));
            memcpy((void*)data.data(), converted.data(), data.size());

            info.data.insert(info.data.end(), data.begin(), data.end());
          }
        }
      }
    }

    void getTexture(const String& base_path, const tinygltf::Model& model, const Vector<int>& textures, Vector<asset::VioletTextureHandle>& handles)
    {
      for (const int& t : textures)
      {
        const tinygltf::Texture& texture = model.textures.at(t);
        const tinygltf::Image&   image   = model.images.at(texture.source);

        size_t width  = image.width;
        size_t height = image.height;
        size_t bpp    = image.component;

				String uri = lmbString(image.uri);

				asset::VioletTextureHandle handle;

				if (uri.empty())
				{
					VioletTexture tex;
					tex.width  = (uint16_t)width;
					tex.height = (uint16_t)height;
					tex.format = TextureFormat::kR8G8B8A8;

					size_t total_size = width * height * 4;

					tex.data.resize(total_size);

					if (bpp == 4)
						memcpy(tex.data.data(), image.image.data(), total_size);
					else if (bpp == 3)
					{
						for (uint32_t i = 0, idx = 0; i < width * height * 3; i += 3, idx += 4)
						{
							memcpy(tex.data.data() + idx, &image.image.at(i), sizeof(tex.data[0] * 3u));
							*((unsigned char*)&tex.data[idx + 3]) = 255;
						}
					}
					else
						LMB_ASSERT(false, "BPP is neither 3 or 4");
					
					static int s_index = 0;
					handle = asset::TextureManager::getInstance()->create(Name("__mesh_io_" + toString(s_index) + "__"), tex);
				}
				else
				{
					handle = asset::TextureManager::getInstance()->get(Name(base_path + uri));
				}

				handles.push_back(handle);
      }
    }

    void addNode(const tinygltf::Model& model, const int& n, const int& current_mesh, Vector<MeshIO::SubMesh>& meshes, const AccessorMemoryConverter& converter)
    {
      const tinygltf::Node& node = model.nodes.at(n);
      MeshIO::SubMesh& mesh = meshes.at(current_mesh);

      // Get the matrix.
      if (node.matrix.size())
      {
        //glm::mat4 m = glm::make_mat4(node.matrix.data());
        lambda::utilities::decomposeMatrix(
          glm::make_mat4(node.matrix.data()),
          mesh.scale,
          mesh.rotation,
          mesh.translation
        );
        glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), mesh.translation);
        model_mat = model_mat * glm::mat4_cast(mesh.rotation);
        model_mat = glm::scale(model_mat, mesh.scale);
        model_mat = model_mat;
      }
      else
      {
        mesh.translation = (node.translation.size() > 0 ? glm::vec3((float)node.translation.at(0), (float)node.translation.at(1), (float)node.translation.at(2)) : glm::vec3(0.0f));
        mesh.scale       = (node.scale.size()       > 0 ? glm::vec3((float)node.scale.at(0),       (float)node.scale.at(1),       (float)node.scale.at(2))       : glm::vec3(1.0f));
        mesh.rotation    = (node.rotation.size()    > 0 ? glm::quat((float)node.rotation.at(3),    (float)node.rotation.at(0),    (float)node.rotation.at(1), (float)node.rotation.at(2)) : glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      }

      if (node.mesh >= 0)
      {
        const tinygltf::Mesh& gltf_mesh = model.meshes.at(node.mesh);

        // For each primitive, find each attribute and insert it.
        for (const tinygltf::Primitive& primitive : gltf_mesh.primitives)
        {
          meshes.push_back(MeshIO::SubMesh());
          MeshIO::SubMesh& sub_mesh = meshes.back();
          sub_mesh.parent = current_mesh;

          sub_mesh.pos = converter.pos.at(contains(primitive.attributes, "POSITION"));
          sub_mesh.nor = converter.nor.at(contains(primitive.attributes, "NORMAL"));
          sub_mesh.tan = converter.tan.at(contains(primitive.attributes, "TANGENT"));
          sub_mesh.col = converter.col.at(contains(primitive.attributes, "COLOR"));
          sub_mesh.tex = converter.tex.at(contains(primitive.attributes, "TEXCOORD_0"));
          sub_mesh.joi = converter.joi.at(contains(primitive.attributes, "JOINTS_0"));
          sub_mesh.wei = converter.wei.at(contains(primitive.attributes, "WEIGHTS_0"));
          sub_mesh.idx = converter.idx.at(primitive.indices);
          sub_mesh.topology = primitive.mode;
          
          const tinygltf::Accessor& position_accessor = model.accessors.at(primitive.attributes.at("POSITION"));
          sub_mesh.aabb_min = glm::vec3(
            (float)position_accessor.minValues.at(0u),
            (float)position_accessor.minValues.at(1u),
            (float)position_accessor.minValues.at(2u)
          ); 
          sub_mesh.aabb_max = glm::vec3(
            (float)position_accessor.maxValues.at(0u),
            (float)position_accessor.maxValues.at(1u),
            (float)position_accessor.maxValues.at(2u)
          );

          if (primitive.material < (const int)model.materials.size())
          {
            tinygltf::Parameter param;
            const tinygltf::Material& material = model.materials.at(primitive.material);
            if (true == containsRaw(material.values, "baseColorTexture", param)) {
              sub_mesh.tex_alb = converter.tex_alb.at((int)param.json_double_value.at("index"));
            }
            if (true == containsRaw(material.additionalValues, "normalTexture", param)) {
              sub_mesh.tex_nor = converter.tex_nor.at((int)param.json_double_value.at("index"));
            }
            if (true == containsRaw(material.values, "metallicRoughnessTexture", param)) {
              sub_mesh.tex_mrt = converter.tex_mrt.at((int)param.json_double_value.at("index"));
            }
            if (true == containsRaw(material.values, "doubleSided", param)) {
              sub_mesh.double_sided = param.bool_value;
            }
            if (true == containsRaw(material.values, "metallicFactor", param)) {
              sub_mesh.metallic = param.number_array.empty() ? 0.0f : (float)param.number_array.at(0);
            }
            if (true == containsRaw(material.values, "roughnessFactor", param)) {
              sub_mesh.roughness = param.number_array.empty() ? 0.0f : (float)param.number_array.at(0);
            }
            if (true == containsRaw(material.values, "baseColorFactor", param)) {
              sub_mesh.colour = param.number_array.empty() ? glm::vec4(0.0f) : glm::vec4(
                (float)param.number_array.at(0), (float)param.number_array.at(1),
                (float)param.number_array.at(2), (float)param.number_array.at(3)
              );
            }
          }
        }
      }

      // Add all children.
      for (const int& child : node.children)
      {
        meshes.push_back(MeshIO::SubMesh());
        meshes.back().parent = current_mesh;
        addNode(model, child, (int)meshes.size() - 1, meshes, converter);
      }
    }
    
    void addNode(const tinygltf::Model& model, AccessorMemory& memory, const int& n)
    {
      // Get the current node.
      const tinygltf::Node& node = model.nodes.at(n);

      // Check if this node has a mesh.
      if (node.mesh >= 0)
      {
        const tinygltf::Mesh& mesh = model.meshes.at(node.mesh);

        // For each primitive, find each attribute and insert it.
        for (const tinygltf::Primitive& primitive : mesh.primitives)
        {
          insert(memory.pos, contains(primitive.attributes, "POSITION"));
          insert(memory.nor, contains(primitive.attributes, "NORMAL"));
          insert(memory.tan, contains(primitive.attributes, "TANGENT"));
          insert(memory.col, contains(primitive.attributes, "COLOR"));
          insert(memory.tex, contains(primitive.attributes, "TEXCOORD_0"));
          insert(memory.joi, contains(primitive.attributes, "JOINTS_0"));
          insert(memory.wei, contains(primitive.attributes, "WEIGHTS_0"));
          insert(memory.idx, primitive.indices);

          if (primitive.material < (const int)model.materials.size())
          {
            tinygltf::Parameter param;
            if (true == containsRaw(model.materials[primitive.material].values, "baseColorTexture", param)) {
              insert(memory.tex_alb, (int)param.json_double_value.at("index"));
            }
            if (true == containsRaw(model.materials[primitive.material].additionalValues, "normalTexture", param)) {
              insert(memory.tex_nor, (int)param.json_double_value.at("index"));
            }
            if (true == containsRaw(model.materials[primitive.material].values, "metallicRoughnessTexture", param)) {
              insert(memory.tex_mrt, (int)param.json_double_value.at("index"));
            }
          }
        }
      }

      // Add all children.
      for (const int& child : node.children)
      {
        addNode(model, memory, child);
      }
    }

    tinygltf::Model loadWorld(const String& path)
    {
      tinygltf::Model model;
      tinygltf::TinyGLTF loader;
      std::string error_message;
      std::string warning_message;
      String extension = getPathExtension(path);
      bool return_value = false;

      if (extension.compare("glb") == 0)
      {
        // assume binary glTF.
        return_value = loader.LoadBinaryFromFile(&model, &error_message, &warning_message, path.c_str());
      }
      else
      {
        // assume ascii glTF.
        return_value = loader.LoadASCIIFromFile(&model,&error_message, &warning_message, path.c_str());
      }

      if (false == warning_message.empty())
      {
        foundation::Error("MeshIO: Warning " + lmbString(warning_message) + "\n");
      }
      if (false == error_message.empty())
      {
        foundation::Error("MeshIO: Error " + lmbString(error_message) + "\n");
      }
      if (false == return_value)
      {
        foundation::Error("MeshIO: Failed to load ." + extension + " : " + String(path) + "\n");
        exit(-1);
      }

      return model;
    }
  }
}
