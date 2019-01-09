/** @file spot_light.as */
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
* @class SpotLight
* @brief Spot light component. Can be attached to an entity
* @author Hilze Vonck
**/
class SpotLight : BaseLight
{
  void Initialize() final
  {
    Violet_Components_Light::CreateSpot(GetId());
  }
  /**
  * @brief Sets the inner cut off of the light
  * @param angle (const Utility::Angle&) The lights new inner cut off
  * @public
  **/
  void SetInnerCutOff(const Utility::Angle&in angle)
  {
    Violet_Components_Light::SetInnerCutOff(GetId(), angle.AsRad());
  }
  /**
  * @brief Gets the inner cut off of the light
  * @return (Utility::Angle) The lights inner cut off
  * @public
  **/
  Utility::Angle GetInnerCutOff() const
  {
    return Utility::AngleFromRad(Violet_Components_Light::GetInnerCutOff(GetId()));
  }
  /**
  * @brief Sets the outer cut off of the light
  * @param angle (const Utility::Angle&) The lights new outer cut off
  * @public
  **/
  void SetOuterCutOff(const Utility::Angle&in angle)
  {
    Violet_Components_Light::SetOuterCutOff(GetId(), angle.AsRad());
  }
  /**
  * @brief Gets the outer cut off of the light
  * @return (Utility::Angle) The lights outer cut off
  * @public
  **/
  Utility::Angle GetOuterCutOff() const
  {
    return Utility::AngleFromRad(Violet_Components_Light::GetOuterCutOff(GetId()));
  }
}

/**
* @}
**/

/**
* @}
**/
