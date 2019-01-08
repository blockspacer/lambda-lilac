#include <scripting/binding/assets/texture.h>
#include <assets/texture.h>
#include <interfaces/iworld.h>
#include <interfaces/iscript_context.h>

namespace lambda
{
  namespace scripting
  {
    namespace assets
    {
      namespace texture
      {
        IScriptContext* g_script_context = nullptr;

        Map<uint64_t, int16_t> g_ref_counts;
        UnorderedMap<String, uint64_t> g_texture_ids;
        Vector<asset::VioletTextureHandle> g_textures;

        uint64_t Load(const String& file_path)
        {
          if (g_texture_ids.find(file_path) == g_texture_ids.end())
          {
            g_texture_ids.insert(eastl::make_pair(file_path, g_textures.size()));
            g_textures.push_back(asset::TextureManager::getInstance()->get(Name(file_path)));
            //g_textures.back()->getLayer(0u).setFlags(kTextureFlagMipMaps);
          }

          return g_texture_ids.at(file_path);
        }
        uint64_t CreateCubeMap(const String& front, const String& back, const String& top, const String& bottom, const String& left, const String& right)
        {
          String name = front + back + top + bottom + left + right;
          if (g_texture_ids.find(name) == g_texture_ids.end())
          {
            g_texture_ids.insert(eastl::make_pair(name, g_textures.size()));

            asset::VioletTextureHandle texture = asset::TextureManager::getInstance()->create(Name(name));
            texture->addLayer(asset::TextureManager::getInstance()->get(Name(front.c_str())));
            texture->addLayer(asset::TextureManager::getInstance()->get(Name(back.c_str())));
            texture->addLayer(asset::TextureManager::getInstance()->get(Name(top.c_str())));
            texture->addLayer(asset::TextureManager::getInstance()->get(Name(bottom.c_str())));
            texture->addLayer(asset::TextureManager::getInstance()->get(Name(left.c_str())));
            texture->addLayer(asset::TextureManager::getInstance()->get(Name(right.c_str())));

            g_textures.push_back(texture);
          }

          return g_texture_ids.at(name);
        }
        uint64_t Create(const float& width, const float& height, const uint8_t& type)
        {
          g_textures.push_back(
            asset::TextureManager::getInstance()->create(
              Name("__script_generated_texture_" + toString(g_textures.size()) + "__"),
              (uint32_t)width, (uint32_t)height, 1u, (TextureFormat)type,
              kTextureFlagIsRenderTarget // TODO (Hilze): Remove!
            )
          );

          return g_textures.size() - 1u;
        }
        uint64_t CreateFromData(const float& width, const float& height, const void* raw_bytes, const uint8_t& type)
        {
          scripting::ScriptArray script_array = g_script_context->scriptArray(raw_bytes);

          g_textures.push_back(
            asset::TextureManager::getInstance()->create(
              Name("__script_generated_texture_" + toString(g_textures.size()) + "__"),
              (uint32_t)width, (uint32_t)height, 1u, (TextureFormat)type,
              kTextureFlagIsRenderTarget, // TODO (Hilze): Remove!
              script_array.vec_uint8
            )
          );

          return g_textures.size() - 1u;
        }
        ScriptVec2 GetSize(const uint64_t& texture_id)
        {
          const asset::VioletTextureHandle& texture = g_textures[texture_id];
          return ScriptVec2(
            (float)texture->getLayer(0u).getWidth(),
            (float)texture->getLayer(0u).getHeight()
          );
        }
        void SetGenerateMipMaps(const uint64_t& texture_id, const bool& generate_mip_maps)
        {
          /*auto& layer = g_textures[texture_id]->getLayer(0u);
          if (generate_mip_maps)
            layer.setFlags(layer.getFlags() | kTextureFlagMipMaps);
          else
            layer.setFlags(layer.getFlags() & ~kTextureFlagMipMaps);*/
        }
        bool GetGenerateMipMaps(const uint64_t& texture_id)
        {
          return false;// (g_textures[texture_id]->getLayer(0u).getFlags() & kTextureFlagMipMaps) != 0 ? true : false;
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
            g_textures[id] = asset::VioletTextureHandle();
          }
        }

        asset::VioletTextureHandle Get(const uint64_t& id)
        {
          return g_textures[id];
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_script_context = world->getScripting().get();

          return Map<lambda::String, void*>{
            { "uint64 Violet_Assets_Texture::Load(const String& in)",                                                                                                   (void*)Load },
            { "uint64 Violet_Assets_Texture::LoadCubeMap(const String& in, const String& in, const String& in, const String& in, const String& in, const String& in)",  (void*)CreateCubeMap },
            { "uint64 Violet_Assets_Texture::Create(const float& in, const float& in, const uint8& in)",                                                                (void*)Create },
            { "uint64 Violet_Assets_Texture::Create(const float& in, const float& in, const Array<uint8>& in, const uint8& in)",                                        (void*)CreateFromData },
            { "Vec2 Violet_Assets_Texture::GetSize(const uint64& in)",                                                                                                  (void*)GetSize },
            { "void Violet_Assets_Texture::SetGenerateMipMaps(const uint64& in, const bool& in)",                                                                       (void*)SetGenerateMipMaps },
            { "bool Violet_Assets_Texture::GetGenerateMipMaps(const uint64& in)",                                                                                       (void*)GetGenerateMipMaps },
            { "void Violet_Assets_Texture::IncRef(const uint64& in)",                                                                                                   (void*)IncRef },
            { "void Violet_Assets_Texture::DecRef(const uint64& in)",                                                                                                   (void*)DecRef }
          };
        }

        extern void Unbind()
        {
          g_textures.clear();
          g_ref_counts.clear();
          g_texture_ids.clear();
          g_script_context = nullptr;
        }
      }
    }
  }
}
