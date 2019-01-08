#pragma once
#include "base_asset_manager.h"
#include <containers/containers.h>
#include "enums.h"

namespace lambda
{
  struct VioletTexture
  {
    uint64_t hash   = 0u;
    String   file   = "";
    uint16_t width  = 0u;
    uint16_t height = 0u;
    uint32_t flags  = 0u;
    uint16_t mip_count = 1u;
    TextureFormat format = TextureFormat::kUnknown;
    Vector<uint32_t> data;
  };

  static constexpr uint32_t kTextureFlagDynamicScale   = 1u << 1u;
  static constexpr uint32_t kTextureFlagDynamicData    = 1u << 2u;
  static constexpr uint32_t kTextureFlagClear          = 1u << 3u;
  //static constexpr uint32_t kTextureFlagMipMaps        = 1u << 4u;
  static constexpr uint32_t kTextureFlagIsRenderTarget = 1u << 5u;
  static constexpr uint32_t kTextureFlagFromDDS        = 1u << 6u;
  static constexpr uint32_t kTextureFlagContainsAlpha  = 1u << 7u;
  static constexpr uint32_t kTextureFlagResize         = 1u << 8u;

  class VioletTextureManager : public VioletBaseAssetManager
  {
  public:
    VioletTextureManager();
    
    uint64_t GetHash(String texture_name);

    void AddTexture(VioletTexture texture);
    VioletTexture GetTexture(uint64_t hash);
    void RemoveTexture(uint64_t hash);

  private:
    VioletTexture JSonToTexture(Vector<char> json);
    Vector<char> TextureToJSon(VioletTexture texture);
  };
}