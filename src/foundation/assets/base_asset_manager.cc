#include "base_asset_manager.h"
#include "utils/file_system.h"
#include "utils/console.h"
#include <lz4.h>

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletBaseAssetManager::SetMagicNumber(String magic_number)
  {
    magic_number_ = magic_number;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletBaseAssetManager::SetGeneratedFilePath(String file_path)
  {
    file_path_generated_ = file_path;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Vector<char> VioletBaseAssetManager::GetData(uint64_t hash) const
  {
    return LoadData(hash);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletBaseAssetManager::Save() const
  {
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletBaseAssetManager::Load()
  {
  }

  void VioletBaseAssetManager::SaveData(Vector<char> data, uint64_t hash)
  {
    Vector<char> compressed_data((uint64_t)LZ4_compressBound((int)data.size()), '\0');
    compressed_data.resize(LZ4_compress_default(data.data(), compressed_data.data(), (int)data.size(), (int)compressed_data.size()) + sizeof(uint32_t));
    //int compression = (int)((1.0f - (float)compressed_data.size() / (float)data.size()) * 100.0f);
    uint32_t size = (uint32_t)data.size();
    memcpy(compressed_data.end() - sizeof(uint32_t), &size, sizeof(uint32_t));

    FileSystem::WriteFile(file_path_generated_ + magic_number_ + "_" + toString(hash), compressed_data.data(), compressed_data.size(), magic_number_.data(), magic_number_.size());
  }

  Vector<char> VioletBaseAssetManager::LoadData(uint64_t hash) const
  {
    Vector<char> data = FileSystem::FileToVector(file_path_generated_ + magic_number_ + "_" + toString(hash), magic_number_.data(), magic_number_.size());

    uint32_t original_size = 0u;
    memcpy(&original_size, data.end() - sizeof(uint32_t), sizeof(uint32_t));

    int size = LZ4_compressBound((int)original_size);
    // Decompress the data.
    Vector<char> decompressed_data(size, '\0');
    int res = LZ4_decompress_safe(data.data(), decompressed_data.data(), (int)data.size() - sizeof(uint32_t), (int)decompressed_data.size());

    if (res < 0)
      LMB_ASSERT(false, "AssetManager: Could not decompress the data.")
    else
      decompressed_data.resize((uint64_t)res);
    
    return decompressed_data;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletBaseAssetManager::RemoveData(uint64_t hash)
  {
    FileSystem::RemoveFile(file_path_generated_ + magic_number_ + "_" + toString(hash));
  }
}
