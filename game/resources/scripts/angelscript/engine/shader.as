/** @file shader.as */

/**
* @addtogroup Assets
* @{
**/

namespace Asset //! Namespace containing all asset related classes, enums and functions
{
  /**
  * @class Asset::Shader
  * @brief A wrapper class for shaders. Allows you to interact with engine shaders.
  * @author Hilze Vonck
  **/
  class Shader
  {
    /**
    * @brief Constructor which will initialize this shader to invalid
    * @public
    **/
    Shader()
    {
      this.id = ToUint64(-1);
    }
    /**
    * @brief Constructor which will initialize this shader to a given id
    * @public
    **/
    Shader(const uint64&in id)
    {
      this.id = id;
    }
    /**
    * @brief Constructor which will initialize this shader to the same id as the other shader
    * @param other (Asset::Shader) The shader which needs to be copied
    * @public
    **/
    Shader(const Shader&in other)
    {
      id = other.id;
    }
    /**
    * @brief Set a float variable in the shader by name
    * @param name (const String) The name of the variable
    * @param value (float) The new value of the variable
    * @public
    **/
    void SetVariable(const String&in name, const float&in value)
    {
      Violet_Assets_Shader::SetVariableFloat1(id, name, value);
    }
    /**
    * @brief Set a Vec2 variable in the shader by name
    * @param name (const String) The name of the variable
    * @param value (Vec2) The new value of the variable
    * @public
    **/
    void SetVariable(const String&in name, const Vec2&in value)
    {
      Violet_Assets_Shader::SetVariableFloat2(id, name, value);
    }
    /**
    * @brief Set a Vec3 variable in the shader by name
    * @param name (const String) The name of the variable
    * @param value (Vec3) The new value of the variable
    * @public
    **/
    void SetVariable(const String&in name, const Vec3&in value)
    {
      Violet_Assets_Shader::SetVariableFloat3(id, name, value);
    }
    /**
    * @brief Returns the engine id of this shader
    * @return (uint64) The engine id of this shader
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
