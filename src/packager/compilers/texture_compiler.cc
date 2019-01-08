#include "texture_compiler.h"
#include <utils/file_system.h>
#include <utils/utilities.h>
#include <utils/console.h>
#include <memory/memory.h>
//#define STBI_MALLOC(sz)           lambda::foundation::Memory::allocate(sz)
//#define STBI_REALLOC(p,newsz)     lambda::foundation::Memory::reallocate(p,newsz)
//#define STBI_FREE(p)              lambda::foundation::Memory::deallocate(p)
#define STBI_NO_STDIO
//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <DirectXTex.h>
#include <wrl.h>

#define VIOLET_GENERATE_MIPS 1

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletTextureCompiler::VioletTextureCompiler() :
    VioletTextureManager()
  {
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletTextureCompiler::Compile(TextureCompileInfo texture_info)
  {
    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    LMB_ASSERT(SUCCEEDED(hr), "TextureCompiler: Could not initialize texture compiler.");

    // Flags.
    bool compress = true;
    bool generate_mips = true;
    TextureFormat format = TextureFormat::kBC3;

    // Load file.
    Vector<char> raw_texture = FileSystem::FileToVector(texture_info.file);
    DirectX::TexMetadata info;
    DirectX::ScratchImage* src_image = foundation::Memory::construct<DirectX::ScratchImage>();
    if (FileSystem::GetExtension(texture_info.file) == "hdr")
    {
      hr = DirectX::LoadFromHDRMemory(raw_texture.data(), raw_texture.size(), &info, *src_image);
      format = TextureFormat::kBC6;
      generate_mips = false;
    }
    else
      hr = DirectX::LoadFromWICMemory(raw_texture.data(), raw_texture.size(), DirectX::DDS_FLAGS_NONE, &info, *src_image);
    LMB_ASSERT(SUCCEEDED(hr), "TextureCompiler: Could not load file.");

    // Generate mip maps.
    if (generate_mips)
    {
      DirectX::ScratchImage* dst_image = foundation::Memory::construct<DirectX::ScratchImage>();
      hr = DirectX::GenerateMipMaps(src_image->GetImages(), src_image->GetImageCount(), src_image->GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0u, *dst_image);
      foundation::Memory::destruct(src_image);
      src_image = dst_image;
      LMB_ASSERT(SUCCEEDED(hr), "TextureCompiler: Could not generate mip maps.");
    }

    // Convert file.
    if (compress)
    {
      DXGI_FORMAT compression_format;
      switch (format)
      {
      case TextureFormat::kBC1: compression_format = DXGI_FORMAT_BC1_UNORM; break;
      case TextureFormat::kBC2: compression_format = DXGI_FORMAT_BC2_UNORM; break;
      case TextureFormat::kBC3: compression_format = DXGI_FORMAT_BC3_UNORM; break;
      case TextureFormat::kBC4: compression_format = DXGI_FORMAT_BC4_UNORM; break;
      case TextureFormat::kBC5: compression_format = DXGI_FORMAT_BC5_UNORM; break;
      case TextureFormat::kBC6: compression_format = DXGI_FORMAT_BC6H_UF16; break;
      case TextureFormat::kBC7: compression_format = DXGI_FORMAT_BC7_UNORM; break;
      }

      DirectX::ScratchImage* dst_image = foundation::Memory::construct<DirectX::ScratchImage>();
      hr = DirectX::Compress(src_image->GetImages(), src_image->GetImageCount(), src_image->GetMetadata(),
        compression_format, DirectX::TEX_COMPRESS_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, *dst_image);
      foundation::Memory::destruct(src_image);
      src_image = dst_image;
      LMB_ASSERT(SUCCEEDED(hr), "TextureCompiler: Could not compress a texture.");
    }

    // Get DDS data.
    DirectX::Blob blob;
    DirectX::SaveToDDSMemory(src_image->GetImages(), src_image->GetImageCount(), src_image->GetMetadata(), DirectX::DDS_FLAGS_NONE, blob);

    VioletTexture texture;
    texture.hash   = GetHash(texture_info.file);
    texture.file   = texture_info.file;
    texture.format = format;
    texture.width  = (uint16_t)src_image->GetImage(0u, 0u, 0u)->width;
    texture.height = (uint16_t)src_image->GetImage(0u, 0u, 0u)->height;
    texture.mip_count = (uint16_t)src_image->GetImageCount(); // TODO (Hilze): Verify.
    texture.flags  = kTextureFlagFromDDS;
    if (!src_image->IsAlphaAllOpaque())
      texture.flags |= kTextureFlagContainsAlpha;
    texture.data.resize((uint32_t)((blob.GetBufferSize() + (sizeof(uint32_t) - 1u)) / sizeof(uint32_t)), 0u);
    memcpy(texture.data.data(), blob.GetBufferPointer(), blob.GetBufferSize());
    AddTexture(texture);

    foundation::Memory::destruct(src_image);
    src_image = nullptr;

    /*Vector<char> raw_texture = FileSystem::FileToVector(texture_info.file);
    int w, h, bpp;
    unsigned char* data = stbi_load_from_memory((unsigned char*)raw_texture.data(), (int)raw_texture.size(), &w, &h, &bpp, STBI_rgb_alpha);

    if (data == nullptr)
    {
      foundation::Error("Failed to load texture: " + texture_info.file + "\n");
      foundation::Error("STBI Error: " + String(stbi_failure_reason()) + "\n");
      LMB_ASSERT(false, "Texture compiler failed");
      return;
    }

    Vector<unsigned char> raw_data(w * h * 4);
    memcpy(raw_data.data(), data, raw_data.size());
    stbi_image_free(data);

    VioletTexture texture;
    texture.hash   = GetHash(texture_info.file);
    texture.file   = texture_info.file;
    texture.format = TextureFormat::kR8G8B8A8;
    texture.width  = (uint32_t)w;
    texture.height = (uint32_t)h;
    texture.data   = utilities::convertVec<unsigned char, uint32_t>(raw_data);
    AddTexture(texture);*/
  }
}