/** @file camera.as */
#include "icomponent.as"

/**
* @addtogroup Components
* @{
**/

/**
* @class Camera
* @brief A camera component which you can attach to an entity
* @author Hilze Vonck
**/
class Camera : IComponent
{
    void Initialize() final
    {
        Violet_Components_Camera::Create(GetId());
    }
    void Destroy() final
    {
        Violet_Components_Camera::Destroy(GetId());
    }
    /**
    * @brief Adds a shader pass to this camera
    * @param name (const String) The name of this shader pass
    * @param shader (const Asset::Shader&) The shader which needs to be used
    * @param input (const Array<String>&) The input textures / render targets that the shader requires
    * @param output (const Array<String>&) The render targets that this shader will render to
    * @public
    **/
    void AddShaderPass(const String&in name, const Asset::Shader&inout shader, const Array<String>&in input, const Array<String>&in output)
    {
      Violet_Components_Camera::AddShaderPass(GetId(), name, shader.GetId(), input, output);
    }


    void SetNear(const float&in near)
    {
      Violet_Components_Camera::SetNear(GetId(), near);
    }
    float GetNear()
    {
      return Violet_Components_Camera::GetNear(GetId());
    }
    void SetFar(const float&in far)
    {
      Violet_Components_Camera::SetFar(GetId(), far);
    }
    float GetFar()
    {
      return Violet_Components_Camera::GetFar(GetId());
    }
}

/**
* @}
**/
