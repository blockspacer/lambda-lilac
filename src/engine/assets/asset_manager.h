#pragma once
#include "asset.h"
#include "interfaces/irenderer.h"
#include <unordered_map>
#include <memory/memory.h>

namespace lambda
{
  namespace asset
  {
    class AssetManager
    {
    public:
      ~AssetManager();
      void initialize(foundation::SharedPointer<platform::IRenderer> renderer);

      friend void AssetHandleDestroyAsset(AssetManager* asset_manager, size_t asset);

      template<class T>
      AssetHandle<T> createAsset(const Name& name, foundation::SharedPointer<T> asset);
      template<class T>
      AssetHandle<T> createIfNotExists(const Name& name, foundation::SharedPointer<T> asset);
      template<class T>
      AssetHandle<T> getAsset(const Name& name);
      
      static AssetManager& getInstance();
      void destroyAllGPUAssets();

    protected:
      void destroyAsset(size_t asset);
      static bool k_initialized_;

    private:
      static AssetManager* k_asset_manager_;
      UnorderedMap<size_t, foundation::SharedPointer<IAsset>> assets_;
      foundation::SharedPointer<platform::IRenderer> renderer_;
    };

    template<class T>
    inline AssetHandle<T> AssetManager::createAsset(const Name& name, foundation::SharedPointer<T> asset)
    {
      assets_.insert(eastl::make_pair(name.getHash(), asset));
      
      return AssetHandle<T>(asset.get(), this, name.getHash());
    }

    template<class T>
    inline AssetHandle<T> AssetManager::createIfNotExists(const Name& name, foundation::SharedPointer<T> asset)
    {
      if (assets_.find(name.getHash()) == assets_.end())
      {
        assets_.insert(eastl::make_pair(name.getHash(), asset));
      }

      return getAsset<T>(name);
    }

    template<class T>
    inline AssetHandle<T> AssetManager::getAsset(const Name& name)
    {
      foundation::SharedPointer<T> ptr(assets_.at(name.getHash()));
      return AssetHandle<T>(ptr.get(), this, name.getHash());
    }
    
    inline AssetManager& AssetManager::getInstance()
    {
      return *k_asset_manager_;
    }

    inline void AssetManager::destroyAllGPUAssets()
    {
      for(auto& it : assets_)
      {
        renderer_->destroyAsset(it.second);
        it.second->gpuData().reset();
      }
    }

    inline void AssetManager::destroyAsset(size_t asset)
    {
      if (assets_.empty())
      {
        return;
      }
      auto it = assets_.find(asset);
      if (it != assets_.end())
      {
        renderer_->destroyAsset(it->second);
        assets_.erase(asset);
      }
    }
  }
}