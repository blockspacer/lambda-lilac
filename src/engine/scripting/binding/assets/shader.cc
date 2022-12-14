#include <scripting/binding/assets/shader.h>
#include <platform/scene.h>
#include <assets/shader.h>
#include <assets/shader_io.h>
#include <scripting/script_vector.h>

namespace lambda
{
  namespace scripting
  {
    namespace assets
    {
      namespace shader
      {
        Map<uint64_t, int16_t> g_ref_counts;
        UnorderedMap<String, uint64_t> g_shader_ids;
        Vector<asset::VioletShaderHandle> g_shaders;

        uint64_t Load(const String& file_path)
        {
          //auto it = g_shader_ids.find(file_path);
          //if (it == g_shader_ids.end())
          {
            //if (g_shader_ids.find(file_path) == g_shader_ids.end())
            {
              String name = "__script_generated_shader_" + toString(g_shaders.size()) + "__";
              g_shader_ids.insert(eastl::make_pair(name, g_shaders.size()));
							g_shaders.push_back(asset::ShaderManager::getInstance()->get(Name(name)));
            }

            //return g_shader_ids.at(file_path);
          }

          return g_shaders.size() - 1u;
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
          if (it->second <= 0)
          {
            g_ref_counts.erase(it);
            g_shaders[id] = asset::VioletShaderHandle();
          }
        }

        asset::VioletShaderHandle Get(const uint64_t& id)
        {
          return g_shaders[id];
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          return Map<lambda::String, void*> {
            { "uint64 Violet_Assets_Shader::Load(const String&in)",                                               (void*)Load },
            { "void Violet_Assets_Shader::IncRef(const uint64& in)",                                              (void*)IncRef },
            { "void Violet_Assets_Shader::DecRef(const uint64& in)",                                              (void*)DecRef }
          };
        }

        extern void Unbind()
        {
          g_ref_counts.clear();
          g_shader_ids.clear();
          g_shaders.clear();
        }
      }
    }
  }
}
