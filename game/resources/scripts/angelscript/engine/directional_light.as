/** @file directional_light.as */
#include "base_light.as"

/**
* @addtogroup Components
* @{
**/

/**
* @addtogroup Lights
* @{
**/

/**
* @class DirectionalLight
* @brief Directional light component. Can be attached to an entity
* @author Hilze Vonck
**/
class DirectionalLight : BaseLight
{
  void Initialize() final
  {
    Violet_Components_Light::CreateDirectional(GetId());
  }
  /**
  * @brief Shadow mapping only! Sets the size of the light. The size is a single float since it will represent both the width and the height of the frustum
  * @param size (const float) The new size for the frustum
  * @public
  **/
  void SetSize(const float&in size)
  {
    Violet_Components_Light::SetSize(GetId(), size);
  }
  /**
  * @brief Shadow mapping only! Gets the size of the lights frustum. The size is a single float since it will represent both the width and the height of the frustum
  * @return (float) The size of the frustum
  * @public
  **/
  float GetSize() const
  {
    return Violet_Components_Light::GetSize(GetId());
  }
}

/**
* @}
**/

/**
* @}
**/
