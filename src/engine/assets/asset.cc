#include "asset.h"
#include "asset_manager.h"

namespace lambda
{
  namespace asset
  {
    void AssetHandleDestroyAsset(AssetManager* asset_manager, size_t asset)
    {
      if (AssetManager::k_initialized_ == true)
      {
        asset_manager->destroyAsset(asset);
      }
    }
  }
}