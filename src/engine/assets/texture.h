#pragma once
#include "assets/asset_handle.h"
#include "utils/bitset.h"
#include <glm/glm.hpp>
#include <containers/containers.h>
#include <assets/texture_manager.h>

namespace lambda
{
  namespace asset
  {
    ///////////////////////////////////////////////////////////////////////////
    class TextureLayer
    {
    public:
      TextureLayer();
      TextureLayer(const TextureLayer& layer);
      TextureLayer(const VioletTexture& data);
      const Vector<char>& getData() const;
      void setData(const Vector<char>& data);
      uint32_t getWidth() const;
      uint32_t getHeight() const;
      uint32_t getMipCount() const;
      TextureFormat getFormat() const;
      void setFormat(TextureFormat format);
      void resize(uint32_t width, uint32_t height);
      void clean();
      void makeDirty();
      bool isDirty() const;
      uint32_t getFlags() const;
      void setFlags(uint32_t flags);
      bool containsAlpha() const;
      void checkForAlpha();

      void operator=(const TextureLayer& layer);

    private:
      VioletTexture data_;
      bool dirty_;
    };

    ///////////////////////////////////////////////////////////////////////////
    class Texture
    {
    public:
      Texture();
      Texture(const Texture& texture);
      Texture(const VioletTexture& texture, uint32_t layer_count = 1u);
      void clean();
      void makeDirty();
      bool isDirty() const;
      void resize(uint32_t width, uint32_t height);

      void addLayer(const VioletHandle<Texture>& texture);
      void addLayer(const TextureLayer& layer);
      void addLayer(const VioletTexture& texture);
      void removeLayer(uint32_t layer);
      uint32_t getLayerCount() const;
      const TextureLayer& getLayer(uint32_t layer) const;
      TextureLayer& getLayer(uint32_t layer);
			bool getKeepInMemory() const;
			void setKeepInMemory(bool keep_in_memory);

    private:
      Vector<TextureLayer> layers_;
			bool keep_in_memory_;
    };
    using VioletTextureHandle = VioletHandle<Texture>;

    ///////////////////////////////////////////////////////////////////////////
    class TextureManager
    {
    public:
      VioletTextureHandle create(Name name);
      VioletTextureHandle create(Name name, Texture texture);
      VioletTextureHandle create(Name name, VioletTexture texture);
			VioletTextureHandle create(
				Name name,
				uint32_t width,
				uint32_t height,
				uint32_t layers = 1u,
				TextureFormat format = TextureFormat::kR8G8B8A8,
				uint32_t flags = 0u,
				const Vector<char>& data = Vector<char>()
			);
			VioletTextureHandle create(
				Name name,
				uint32_t width,
				uint32_t height,
				uint32_t layers,
				TextureFormat format,
				uint32_t flags,
				const Vector<unsigned char>& data
			);
      VioletTextureHandle get(Name name);
      VioletTextureHandle get(uint64_t hash);
			Vector<char> getData(VioletTextureHandle texture);
      void destroy(VioletTextureHandle texture);

    public:
      static TextureManager* getInstance();

    protected:
      VioletTextureManager& getManager();
      const VioletTextureManager& getManager() const;

    private:
      VioletTextureManager manager_;
    };
  }
}