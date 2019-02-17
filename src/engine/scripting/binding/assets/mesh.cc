#include <scripting/binding/assets/mesh.h>
#include <interfaces/iworld.h>
#include <interfaces/iscript_context.h>
#include <platform/scene.h>
#include <assets/mesh.h>
#include <assets/mesh_io.h>
#include <utils/mesh_decimator.h>
#include <assets/asset_manager.h>

namespace lambda
{
  namespace scripting
  {
    namespace assets
    {
      namespace mesh
      {
        IScriptContext* g_script_context = nullptr;

        Map<uint64_t, int16_t> g_ref_counts;
        UnorderedMap<String, uint64_t> g_mesh_ids;
        Vector<asset::MeshHandle> g_meshes;
        
        uint64_t Add(asset::MeshHandle mesh, const String& name)
        {
          g_mesh_ids.insert(eastl::make_pair(name, g_meshes.size()));
          uint64_t mesh_id = g_meshes.size();
          g_meshes.push_back(mesh);
          return mesh_id;
        }
        uint64_t Load(const String& file_path)
        {
          if (g_mesh_ids.find(file_path) == g_mesh_ids.end())
          {
            io::MeshIO::Mesh mesh = io::MeshIO::load(file_path.c_str());
            asset::Mesh input = io::MeshIO::asAsset(mesh);
            asset::MeshHandle mesh_handle = asset::AssetManager::getInstance().createAsset(
              file_path, foundation::Memory::constructShared<asset::Mesh>(input)
            );
            return Add(mesh_handle, file_path);
          }
          else
            return g_mesh_ids[file_path];
        }
        uint64_t Create()
        {
          static size_t sid = 0u;
          String name = "__created_mesh_" + toString(sid++) + "__";

          asset::MeshHandle mesh_handle = asset::AssetManager::getInstance().createAsset(
            name, foundation::Memory::constructShared<asset::Mesh>()
          );
          return Add(mesh_handle, name);
        }
        uint64_t CreateDefault(const String& type)
        {
          static size_t sid = 0u;
          String name = "__generated_mesh_" + toString(sid++) + "__";

          asset::MeshHandle mesh_handle;
          if (type == "cube")
            mesh_handle = asset::AssetManager::getInstance().createAsset(
              name, foundation::Memory::constructShared<asset::Mesh>(asset::Mesh::createCube())
            );
          else if (type == "cylinder")
            mesh_handle = asset::AssetManager::getInstance().createAsset(
              name, foundation::Memory::constructShared<asset::Mesh>(asset::Mesh::createCylinder())
            );
          else if (type == "sphere")
            mesh_handle = asset::AssetManager::getInstance().createAsset(
              name, foundation::Memory::constructShared<asset::Mesh>(asset::Mesh::createSphere())
            );

          return Add(mesh_handle, name);
        }
        void SetPositions(const uint64_t& id, const void* raw_bytes)
        {
          scripting::ScriptArray script_array = g_script_context->scriptArray(raw_bytes);
          g_meshes[id]->set(asset::MeshElements::kPositions, script_array.vec_vec3);

          Vector<asset::SubMesh> sub_meshes = g_meshes[id]->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kPositions].count = script_array.vec_vec3.size();
          sub_meshes.back().offsets[asset::MeshElements::kPositions].stride = sizeof(float) * 3u;

          glm::vec3 min(FLT_MAX);
          glm::vec3 max(FLT_MIN);
          for (const auto& it : script_array.vec_vec3)
          {
            if (it.x < min.x) min.x = it.x;
            if (it.y < min.y) min.y = it.y;
            if (it.z < min.z) min.z = it.z;
            if (it.x > max.x) max.x = it.x;
            if (it.y > max.y) max.y = it.y;
            if (it.z > max.z) max.z = it.z;
          }
          sub_meshes.back().min = min;
          sub_meshes.back().max = max;
          g_meshes[id]->setSubMeshes(sub_meshes);
        }
        void SetNormals(const uint64_t& id, const void* raw_bytes)
        {
          scripting::ScriptArray script_array = g_script_context->scriptArray(raw_bytes);
          g_meshes[id]->set(asset::MeshElements::kNormals, script_array.vec_vec3);

          Vector<asset::SubMesh> sub_meshes = g_meshes[id]->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kNormals].count = script_array.vec_vec3.size();
          sub_meshes.back().offsets[asset::MeshElements::kNormals].stride = sizeof(float) * 3u;
          g_meshes[id]->setSubMeshes(sub_meshes);
        }
        void SetTexCoords(const uint64_t& id, const void* raw_bytes)
        {
          scripting::ScriptArray script_array = g_script_context->scriptArray(raw_bytes);
          g_meshes[id]->set(asset::MeshElements::kTexCoords, script_array.vec_vec2);

          Vector<asset::SubMesh> sub_meshes = g_meshes[id]->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kTexCoords].count = script_array.vec_vec2.size();
          sub_meshes.back().offsets[asset::MeshElements::kTexCoords].stride = sizeof(float) * 2u;
          g_meshes[id]->setSubMeshes(sub_meshes);
        }
        void SetColours(const uint64_t& id, const void* raw_bytes)
        {
          scripting::ScriptArray script_array = g_script_context->scriptArray(raw_bytes);
          g_meshes[id]->set(asset::MeshElements::kColours, script_array.vec_vec4);
        }
        void SetTangents(const uint64_t& id, const void* raw_bytes)
        {
          scripting::ScriptArray script_array = g_script_context->scriptArray(raw_bytes);
          g_meshes[id]->set(asset::MeshElements::kTangents, script_array.vec_vec3);
        }
        void RecalculateTangents(const uint64_t& id)
        {
          g_meshes[id]->recalculateTangents();
        }
        void SetJoints(const uint64_t& id, const void* raw_bytes)
        {
          scripting::ScriptArray script_array = g_script_context->scriptArray(raw_bytes);
          g_meshes[id]->set(asset::MeshElements::kJoints, script_array.vec_vec4);
        }
        void SetWeights(const uint64_t& id, const void* raw_bytes)
        {
          scripting::ScriptArray script_array = g_script_context->scriptArray(raw_bytes);
          g_meshes[id]->set(asset::MeshElements::kWeights, script_array.vec_vec4);
        }
        void SetIndices(const uint64_t& id, const void* raw_bytes)
        {
          scripting::ScriptArray script_array = g_script_context->scriptArray(raw_bytes);
          g_meshes[id]->set(asset::MeshElements::kIndices, script_array.vec_uint32);

          Vector<asset::SubMesh> sub_meshes = g_meshes[id]->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kIndices].count = script_array.vec_uint32.size();
          sub_meshes.back().offsets[asset::MeshElements::kIndices].stride = sizeof(uint32_t);
          g_meshes[id]->setSubMeshes(sub_meshes);
        }
        uint64_t Decimate(const uint64_t& in_id, const float& reduction, const float& target_error)
        {
          static size_t id = 0u;
          String name = "__decimated_" + toString(id++) + "__";

          asset::Mesh mesh;
          platform::MeshDecimator decimator;
          decimator.decimate(g_meshes[in_id].get(), &mesh, reduction, target_error);
          asset::MeshHandle mesh_handle = asset::AssetManager::getInstance().createAsset(Name(name), foundation::Memory::constructShared<asset::Mesh>(mesh));

          return Add(mesh_handle, name);
        }
        uint16_t GetSubMeshCount(const uint64_t& mesh_id)
        {
          return (uint16_t)g_meshes[mesh_id]->getSubMeshes().size();
        }
        void IncRef(const uint64_t& id)
        {
          auto it = g_ref_counts.find(id);
          if (it == g_ref_counts.end())
          {
            g_ref_counts.insert(eastl::make_pair(id, 0));
            it = g_ref_counts.find(id);
          }
          it->second++;
        }
        void DecRef(const uint64_t& id)
        {
          auto it = g_ref_counts.find(id);
          if (it == g_ref_counts.end())
          {
            g_ref_counts.insert(eastl::make_pair(id, 0));
            it = g_ref_counts.find(id);
          }
          it->second--;
        }

        asset::MeshHandle Get(const uint64_t id)
        {
          return g_meshes[id];
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_script_context = world->getScripting().get();

          return Map<lambda::String, void*> {
            { "uint64 Violet_Assets_Mesh::Load(const String& in)",                                       (void*)Load },
            { "uint64 Violet_Assets_Mesh::Create()",                                                     (void*)Create },
            { "uint64 Violet_Assets_Mesh::CreateDefault(const String& in)",                              (void*)CreateDefault },
            { "uint64 Violet_Assets_Mesh::Decimate(const uint64& in, const float& in, const float& in)", (void*)Decimate },
            { "uint16 Violet_Assets_Mesh::GetSubMeshCount(const uint64& in)",                            (void*)GetSubMeshCount },
            { "void Violet_Assets_Mesh::IncRef(const uint64& in)",                                       (void*)IncRef },
            { "void Violet_Assets_Mesh::DecRef(const uint64& in)",                                       (void*)DecRef },
            { "void Violet_Assets_Mesh::SetPositions(const uint64& in, const Array<Vec3>& in)",          (void*)SetPositions },
            { "void Violet_Assets_Mesh::SetNormals(const uint64& in, const Array<Vec3>& in)",            (void*)SetNormals },
            { "void Violet_Assets_Mesh::SetTexCoords(const uint64& in, const Array<Vec2>& in)",          (void*)SetTexCoords },
            { "void Violet_Assets_Mesh::SetColours(const uint64& in, const Array<Vec4>& in)",            (void*)SetColours },
            { "void Violet_Assets_Mesh::SetTangents(const uint64& in, const Array<Vec3>& in)",           (void*)SetTangents },
            { "void Violet_Assets_Mesh::RecalculateTangents(const uint64& in)",                          (void*)RecalculateTangents },
            { "void Violet_Assets_Mesh::SetJoints(const uint64& in, const Array<Vec4>& in)",             (void*)SetJoints },
            { "void Violet_Assets_Mesh::SetWeights(const uint64& in, const Array<Vec4>& in)",            (void*)SetWeights },
            { "void Violet_Assets_Mesh::SetIndices(const uint64& in, const Array<uint32>& in)",          (void*)SetIndices },
          };
        }

        extern void Unbind()
        {
          g_ref_counts.clear();
          g_mesh_ids.clear();
          g_meshes.clear();

          g_script_context = nullptr;
        }
      }
    }
  }
}
