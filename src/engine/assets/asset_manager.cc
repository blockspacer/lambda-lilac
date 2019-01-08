#include "asset_manager.h"

namespace lambda
{
  namespace asset
  {
    AssetManager::~AssetManager()
    {
      while (assets_.empty() == false)
      {
        destroyAsset(assets_.begin()->first);
      }
      assets_.clear();
      renderer_.reset();
      k_initialized_ = false;
    }
    void AssetManager::initialize(foundation::SharedPointer<platform::IRenderer> renderer)
    {
      k_initialized_ = true;
      k_asset_manager_ = this;
      renderer_ = renderer;
    }
    AssetManager* AssetManager::k_asset_manager_ = nullptr;
    bool AssetManager::k_initialized_ = false;
  }
}