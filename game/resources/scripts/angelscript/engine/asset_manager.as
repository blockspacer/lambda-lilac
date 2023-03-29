/** @file asset_manager.as */
#include "assets.as"

namespace Asset //! Namespace containing all asset related classes, enums and functions
{
    /**
    * @class Asset::AssetManager
    * @brief A manager of assets that allows you to load, create and unload assets.
    * @brief A static instance always exist: asset_manager.
    * @todo Add the possibility for the user to create meshes and shaders from code.
    * @author Hilze Vonck
    **/
  class AssetManager
  {
    /**
    * @brief Creates an empty texture based upon the size and the format provided. Useful for a staticly sized render target
    * @param size (const Vec2&) The size of the desired texture
    * @param format (const Asset::TextureFormat) The format of the desired texture
    * @return (Asset::Texture) The created texture
    * @public
    **/
    Texture CreateTexture(const Vec2&in size, const TextureFormat&in format)
    {
      return Texture(Violet_Assets_Texture::Create(size.x, size.y, format));
    }
    /**
    * @brief Creates a not empty texture based upon the size, bytes and the format provided. Useful for creating a texture from code
    * @param size (const Vec2&) The size of the desired texture
    * @param bytes (const Array<uint8>&) The raw texture
    * @param format (const Asset::TextureFormat) The format of the desired texture
    * @return (Asset::Texture) The created texture
    * @public
    **/
    Texture CreateTexture(const Vec2&in size, const Array<uint8>&in bytes, const TextureFormat&in format)
    {
      return Texture(Violet_Assets_Texture::Create(size.x, size.y, bytes, format));
    }
    /**
    * @brief Loads a texture from disk
    * @param file_path (const String) The file path of the desired texture
    * @return (Asset::Texture) The loaded texture
    * @public
    **/
    Texture LoadTexture(const String&in file_path)
    {
      return Texture(Violet_Assets_Texture::Load(file_path));
    }
    Texture LoadCubeMap(const String&in front, const String&in back, const String&in top, const String&in bottom, const String&in left, const String&in right)
    {
      return Texture(Violet_Assets_Texture::LoadCubeMap(front, back, top, bottom, left, right));
    }
    Mesh CreateMesh()
    {
      return Mesh(Violet_Assets_Mesh::Create());
    }
    Mesh GenerateMesh(const String&in type)
    {
      return Mesh(Violet_Assets_Mesh::CreateDefault(type));
    }
    /**
    * @brief Loads a mesh from disk
    * @param file_path (const String) The file path of the desired mesh
    * @return (Asset::Mesh) The loaded mesh
    * @public
    **/
    Mesh LoadMesh(const String&in file_path)
    {
      return Mesh(Violet_Assets_Mesh::Load(file_path));
    }
    /**
    * @brief Loads a shader from disk
    * @param file_path (const String) The file path of the desired shader
    * @return (Asset::Shader) The loaded shader
    * @public
    **/
    Shader LoadShader(const String&in file_path)
    {
      return Shader(Violet_Assets_Shader::Load(file_path));
    }
    /**
    * @brief Loads a wave from disk
    * @param file_path (const String) The file path of the desired wave
    * @return (Asset::Wave) The loaded wave
    * @public
    **/
    Wave LoadWave(const String&in file_path)
    {
      return Wave(Violet_Assets_Wave::Load(file_path));
    }
  }
}
Asset::AssetManager asset_manager; //!< The static instance of the asset manager
