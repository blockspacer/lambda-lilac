/** @file point_light.as */
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
* @class PointLight
* @brief point light component. Can be attached to an entity
* @author Hilze Vonck
**/
class PointLight : BaseLight
{
  void Create() final
  {
    Violet_Components_Light::CreatePoint(GetId());
  }
}

/**
* @}
**/

/**
* @}
**/
