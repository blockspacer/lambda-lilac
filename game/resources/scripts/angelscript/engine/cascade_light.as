/** @file cascade_light.as */
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
* @class CascadeLight
* @brief Cascade light component. Can be attached to an entity
* @author Hilze Vonck
**/
class CascadeLight : BaseLight
{
  void Initialize() final
  {
    Violet_Components_Light::CreateCascade(GetId());
  }
}

/**
* @}
**/

/**
* @}
**/
