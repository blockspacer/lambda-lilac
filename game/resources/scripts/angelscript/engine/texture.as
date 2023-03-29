/** @file texture.as */
#include "enums.as"

/**
* @addtogroup Assets
* @{
**/

namespace Asset //! Namespace containing all asset related classes, enums and functions
{
  /**
  * @class Asset::Texture
  * @brief A wrapper class for textures. Allows you to interact with engine textures.
  * @author Hilze Vonck
  **/
  class Texture
  {
    /**
    * @brief Constructor which will initialize this texture to invalid
    * @public
    **/
    Texture()
    {
      this.id = ToUint64(-1);
    }
    /**
    * @brief Constructor which will initialize this texture to a given id
    * @public
    **/
    Texture(const uint64&in id)
    {
      this.id = id;
    }
    /**
    * @brief Constructor which will initialize this texture to the same id as the other texture
    * @param other (Asset::Texture) The texture which needs to be copied
    * @public
    **/
    Texture(const Texture&in other)
    {
      id = other.id;
    }
    /**
    * @brief Set whether or not mip maps need to be generated for this texture. This needs to be called right after the texture is created in order for this to work.
    * @param generate_mip_maps (const bool) Wether or not mip maps should be generated
    * @public
    **/
    void SetGenerateMipMaps(const bool&in generate_mip_maps) const
    {
      Violet_Assets_Texture::SetGenerateMipMaps(id, generate_mip_maps);
    }
    /**
    * @brief Get whether or not mip maps need to be generated for this texture
    * @return (bool) Wether or not mip maps should be generated
    * @public
    **/
    bool GetGenerateMipMaps() const
    {
      return Violet_Assets_Texture::GetGenerateMipMaps(id);
    }
    /**
    * @brief Get the size of the texture
    * @return (Vec2) The size of the texture
    * @public
    **/
    Vec2 GetSize() const
    {
      return Violet_Assets_Texture::GetSize(id);
    }
    /**
    * @brief Get the engine id of the texture
    * @return (uint64) The engine id of the texture
    * @public
    **/
    uint64 GetId() const
    {
      return id;
    }

    private uint64 id;
  }
}

/**
* @}
**/
