#include "texture.h"
#include "utils/utilities.h"
#include "utils/console.h"
#include "utils/file_system.h"
#include <memory/memory.h>

namespace lambda
{
  namespace asset
  {
    ///////////////////////////////////////////////////////////////////////////
    TextureLayer::TextureLayer() :
      data_(),
      dirty_(false)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    TextureLayer::TextureLayer(const TextureLayer& layer) :
      data_(layer.data_),
      dirty_(layer.dirty_)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    TextureLayer::TextureLayer(const VioletTexture& data) :
      data_(data),
      dirty_(true)
    {
      if (!data_.data.empty())
        checkForAlpha();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    const Vector<char>& TextureLayer::getData() const
    {
      return data_.data;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void TextureLayer::setData(const Vector<char>& data)
    {
      if ((data_.flags & kTextureFlagDynamicData))
      {
        data_.data = data;
        makeDirty();

        if (!data.empty())
          checkForAlpha();
      }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    uint32_t TextureLayer::getWidth() const
    {
      return data_.width;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    uint32_t TextureLayer::getHeight() const
    {
      return data_.height;
    }

    ///////////////////////////////////////////////////////////////////////////
    uint32_t TextureLayer::getMipCount() const
    {
      return data_.mip_count;
    }

    ///////////////////////////////////////////////////////////////////////////
    TextureFormat TextureLayer::getFormat() const
    {
      return data_.format;
    }

    ///////////////////////////////////////////////////////////////////////////
    void TextureLayer::setFormat(TextureFormat format)
    {
      if (data_.format != format)
      {
        data_.format = format;
        makeDirty();
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void TextureLayer::resize(uint32_t width, uint32_t height)
    {
      if (0 == (data_.flags & kTextureFlagFromDDS))
      {
        data_.width  = width;
        data_.height = height;
        data_.flags |= kTextureFlagRecreate;
        makeDirty();
      }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void TextureLayer::clean()
    {
      dirty_ = false;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void TextureLayer::makeDirty()
    {
      dirty_ = true;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    bool TextureLayer::isDirty() const
    {
      return dirty_;
    }

    ///////////////////////////////////////////////////////////////////////////
    uint32_t TextureLayer::getFlags() const
    {
      return data_.flags;
    }

    ///////////////////////////////////////////////////////////////////////////
    void TextureLayer::setFlags(uint32_t flags)
    {
      if (data_.flags == flags)
        return;

      data_.flags = flags;
      makeDirty();
    }

    ///////////////////////////////////////////////////////////////////////////
    bool TextureLayer::containsAlpha() const
    {
      return (data_.flags & kTextureFlagContainsAlpha) != 0u;
    }

    ///////////////////////////////////////////////////////////////////////////
    void TextureLayer::checkForAlpha()
    {
      setFlags(getFlags() & ~kTextureFlagContainsAlpha);

      if ((data_.flags & kTextureFlagFromDDS))
        return;

      // TODO (Hilze): Support other formats.
      if (data_.format != TextureFormat::kR8G8B8A8)
        return;

      // It skips every other pixel to optimize this function a bit.
      for (unsigned int i = 3u; i < data_.data.size(); i += 4u * 2u)
      {
        if (*(data_.data.data() + i) < 127u)
        {
          data_.flags |= kTextureFlagContainsAlpha;
          return;
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void TextureLayer::operator=(const TextureLayer& layer)
    {
      data_  = layer.data_;
      dirty_ = layer.dirty_;
    }










    ///////////////////////////////////////////////////////////////////////////
    Texture::Texture()
			: keep_in_memory_(false)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    Texture::Texture(const Texture& texture)
			: layers_(texture.layers_)
			, keep_in_memory_(texture.keep_in_memory_)
		{
    }
    
    ///////////////////////////////////////////////////////////////////////////
    Texture::Texture(const VioletTexture& texture, uint32_t layer_count)
			: keep_in_memory_(false)
		{
      for (uint32_t i = 0u; i < layer_count; ++i)
        addLayer(texture);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void Texture::clean()
    {
      for (auto& layer : layers_)
        layer.clean();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void Texture::makeDirty()
    {
      for (auto& layer : layers_)
        layer.makeDirty();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    bool Texture::isDirty() const
    {
      for (const auto& layer : layers_)
        if (layer.isDirty())
          return true;

      return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    void Texture::resize(uint32_t width, uint32_t height)
    {
      for (auto& layer : layers_)
        layer.resize(width, height);
    }

    ///////////////////////////////////////////////////////////////////////////
    void Texture::addLayer(const VioletHandle<Texture>& texture)
    {
      for (uint32_t i = 0u; i < texture->getLayerCount(); ++i)
        addLayer(texture->getLayer(i));
    }

    ///////////////////////////////////////////////////////////////////////////
    void Texture::addLayer(const TextureLayer& layer)
    {
      layers_.push_back(layer);
    }

    ///////////////////////////////////////////////////////////////////////////
    void Texture::addLayer(const VioletTexture& texture)
    {
      layers_.push_back(TextureLayer(texture));
    }

    ///////////////////////////////////////////////////////////////////////////
    void Texture::removeLayer(uint32_t layer)
    {
      layers_.erase(layers_.begin() + layer);
      layers_.resize(layers_.size());
      // Remove the dangling layers. Slower but might save ram space.
    }

    ///////////////////////////////////////////////////////////////////////////
    uint32_t Texture::getLayerCount() const
    {
      return (uint32_t)layers_.size();
    }

    ///////////////////////////////////////////////////////////////////////////
    const TextureLayer& Texture::getLayer(uint32_t layer) const
    {
      LMB_ASSERT(
        layer < layers_.size(), 
        "TEXTURE LAYER: Tried to get a layer which is out of bounds."
      );
      return layers_[layer];
    }

    ///////////////////////////////////////////////////////////////////////////
    TextureLayer& Texture::getLayer(uint32_t layer)
    {
      LMB_ASSERT(
        layer < layers_.size(), 
        "TEXTURE LAYER: Tried to get a layer which is out of bounds."
      );
      return layers_[layer];
    }

		///////////////////////////////////////////////////////////////////////////
		bool Texture::getKeepInMemory() const
		{
			return keep_in_memory_;
		}

		///////////////////////////////////////////////////////////////////////////
		void Texture::setKeepInMemory(bool keep_in_memory)
		{
			keep_in_memory_ = keep_in_memory;
		}










    ///////////////////////////////////////////////////////////////////////////
    VioletTextureHandle TextureManager::create(Name name)
    {
      return VioletTextureHandle(
        foundation::Memory::construct<Texture>(), 
        name
      );
    }

    ///////////////////////////////////////////////////////////////////////////
    VioletTextureHandle TextureManager::create(Name name, Texture texture)
    {
      return VioletTextureHandle(
        foundation::Memory::construct<Texture>(texture), 
        name
      );
    }

    ///////////////////////////////////////////////////////////////////////////
    VioletTextureHandle TextureManager::create(Name name, VioletTexture texture)
    {
      return VioletTextureHandle(
        foundation::Memory::construct<Texture>(texture), 
        name
      );
    }
    
    ///////////////////////////////////////////////////////////////////////////
    VioletTextureHandle TextureManager::create(
      Name name, 
      uint32_t width, 
      uint32_t height, 
      uint32_t layers, 
      TextureFormat format, 
      uint32_t flags, 
      const Vector<char>& data)
    {
      VioletTexture layer;
      layer.hash   = name.getHash();
      layer.file   = name.getName();
      layer.width  = width;
      layer.height = height;
      layer.flags  = flags;
      layer.format = format;
      layer.data   = data;

      return VioletTextureHandle(
        foundation::Memory::construct<Texture>(layer, layers),
        name
      );
    }

		///////////////////////////////////////////////////////////////////////////
		VioletTextureHandle TextureManager::create(
			Name name,
			uint32_t width,
			uint32_t height,
			uint32_t layers,
			TextureFormat format,
			uint32_t flags,
			const Vector<unsigned char>& data)
		{
			return create(
				name, 
				width, 
				height, 
				layers, 
				format, 
				flags, 
				utilities::convertVec<unsigned char, char>(data)
			);
		}

    ///////////////////////////////////////////////////////////////////////////
    VioletTextureHandle TextureManager::get(Name name)
    {
      return get(manager_.GetHash(FileSystem::MakeRelative(name.getName())));
    }
    
    ///////////////////////////////////////////////////////////////////////////
    VioletTextureHandle TextureManager::get(uint64_t hash)
    {
      VioletTexture texture = manager_.GetTexture(hash);
      return create(texture.file, texture);
    }

		///////////////////////////////////////////////////////////////////////////
		Vector<char> TextureManager::getData(VioletTextureHandle texture)
		{
			return eastl::move(manager_.GetData(texture.getHash()));
		}
    
    ///////////////////////////////////////////////////////////////////////////
    void TextureManager::destroy(VioletTextureHandle texture)
    {
      foundation::Memory::destruct<Texture>(texture.get());
    }
    
    ///////////////////////////////////////////////////////////////////////////
    TextureManager* TextureManager::getInstance()
    {
      static TextureManager* s_instance = 
        foundation::Memory::construct<TextureManager>();
      
      return s_instance;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    VioletTextureManager& TextureManager::getManager()
    {
      return manager_;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    const VioletTextureManager& TextureManager::getManager() const
    {
      return manager_;
    }
  }
}
