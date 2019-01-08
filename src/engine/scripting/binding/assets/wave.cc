#include <scripting/binding/assets/wave.h>
#include <soloud_wav.h>

namespace lambda
{
  namespace scripting
  {
    namespace assets
    {
      namespace wave
      {
        Map<uint64_t, int16_t> g_ref_counts;
        UnorderedMap<String, uint64_t> g_wave_ids;
        Vector<asset::VioletWaveHandle> g_waves;
        
        uint64_t Load(const String& file_path)
        {
          if (g_wave_ids.find(file_path) == g_wave_ids.end())
          {
            g_wave_ids.insert(eastl::make_pair(file_path, g_waves.size()));
            g_waves.push_back(asset::WaveManager::getInstance()->get(Name(file_path.c_str())));
          }

          return g_wave_ids.at(file_path);
        }
        float GetLength(const uint64_t& id)
        {
          return (float)g_waves[id]->getBuffer()->getLength();
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
            g_waves.at(id) = asset::VioletWaveHandle();
          }
        }

        extern asset::VioletWaveHandle Get(const uint64_t& id)
        {
          return g_waves[id];
        }

        uint64_t Count()
        {
          return g_waves.size();
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          return Map<lambda::String, void*>{
            { "uint64 Violet_Assets_Wave::Load(const String& in)",     (void*)Load },
            { "float Violet_Assets_Wave::GetLength(const uint64& in)", (void*)GetLength },
            { "void Violet_Assets_Wave::IncRef(const uint64& in)",     (void*)IncRef },
            { "void Violet_Assets_Wave::DecRef(const uint64& in)",     (void*)DecRef },
          };
        }

        void Unbind()
        {
          g_waves.clear();
          g_wave_ids.clear();
          g_ref_counts.clear();
        }
      }
    }
  }
}
