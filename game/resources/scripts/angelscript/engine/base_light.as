/** @file base_light.as */
#include "colour.as"
#include "angle.as"
#include "enums.as"
#include "icomponent.as"

/**
* @addtogroup Components
* @{
**/

/**
* @addtogroup Lights
* @{
**/

/**
* @class BaseLight
* @brief base light component. Used by the other light types
* @author Hilze Vonck
**/

class BaseLight : IComponent
{
  void Initialize()
  {
    Error("Base light initialize needs to be overwritten!");
  }
  void Destroy() final
  {
    Violet_Components_Light::Destroy(GetId());
  }
  /**
  * @brief Sets the colour of the light
  * @param colour (const Colour&) The new light colour
  * @public
  **/
  void SetLightColour(const Colour&in colour)
  {
    Violet_Components_Light::SetLightColour(GetId(), colour.AsRGB().AsVec3());
  }
  /**
  * @brief Gets the colour of the light
  * @return (Colour) The lights colour
  * @public
  **/
  Colour GetLightColour() const
  {
    return Colour(RGB(Violet_Components_Light::GetLightColour(GetId())));
  }
  /**
  * @brief Sets the intensity of the light
  * @param intensity (const float) The new light intensity
  * @public
  **/
  void SetLightIntensity(const float&in intensity)
  {
    Violet_Components_Light::SetLightIntensity(GetId(), intensity);
  }
  /**
  * @brief Gets the intensity of the light
  * @return (float) The lights intensity
  * @public
  **/
  float GetLightIntensity() const
  {
    return Violet_Components_Light::GetLightIntensity(GetId());
  }
  /**
  * @brief Sets the ambient colour of the light
  * @param colour (const Colour&) The lights new ambient colour
  * @public
  **/
  void SetAmbientColour(const Colour&in colour)
  {
    Violet_Components_Light::SetAmbientColour(GetId(), colour.AsRGB().AsVec3());
  }
  /**
  * @brief Gets the ambient colour of the light
  * @return (Colour) The lights ambient colour
  * @public
  **/
  Colour GetAmbientColour() const
  {
    return Colour(RGB(Violet_Components_Light::GetAmbientColour(GetId())));
  }
  /**
  * @brief Sets the depth of the light
  * @param depth (const float) The lights new depth
  * @public
  **/
  void SetDepth(const float&in depth)
  {
    Violet_Components_Light::SetDepth(GetId(), depth);
  }
  /**
  * @brief Gets the depth of the light
  * @return (float) The lights depth
  * @public
  **/
  float GetDepth() const
  {
    return Violet_Components_Light::GetDepth(GetId());
  }
  /**
  * @brief Shadow mapping only! Sets the shadow type of the light. See ShadowType for more info
  * @param shadow_type (const ShadowType) The new shadow type
  * @public
  **/
  void SetShadowType(const ShadowType&in shadow_type)
  {
    Violet_Components_Light::SetShadowType(GetId(), int8(shadow_type));
  }
  /**
  * @brief Shadow mapping only! Gets the shadow type of the light. See ShadowType for more info
  * @return (ShadowType) The shadow type
  * @public
  **/
  ShadowType GetShadowType() const
  {
    return ShadowType(Violet_Components_Light::GetShadowType(GetId()));
  }
  /**
  * @brief Shadow mapping only! Sets the shadow mapping render target of the light. The String provided needs to be a registered render target!
  * @param render_target (const String) The new shadow mapping render target
  * @public
  **/
  void SetRenderTarget(const String&in render_target)
  {
    Array<String> render_targets;
    render_targets.PushBack(render_target);
    SetRenderTargets(render_targets);
  }
  /**
  * @brief Shadow mapping only! Sets the shadow mapping render target of the light. The String provided needs to be a registered render target!
  * @param render_target (const String) The new shadow mapping render target
  * @public
  **/
  void SetRenderTargets(const Array<String>&in render_targets)
  {
    Violet_Components_Light::SetRenderTargets(GetId(), render_targets);
  }
  /**
  * @brief Adds a texture to the light. Is useful for things like flash lights or when something needs to be projected onto a surface.
  * @param texture (const Asset::Texture&in) The texture that this light should use.
  * @public
  **/
  void SetTexture(const Asset::Texture&in texture)
  {
    Violet_Components_Light::SetTexture(GetId(), texture.GetId());
  }
  /**
  * @brief Sets whether or not this light is enabled.
  * @param enabled (const bool&in) Is this light enabled?
  * @public
  **/
  void SetEnabled(const bool&in enabled)
  {
    Violet_Components_Light::SetEnabled(GetId(), enabled);
  }
  /**
  * @brief Returns whether or not this light is enabled.
  * @return (bool) Is this light enabled?
  * @public
  **/
  bool GetEnabled()
  {
    return Violet_Components_Light::GetEnabled(GetId());
  }
  /**
  * @brief Set how many frames it should take for a dynamic light to be updated.
  * @param frequency (const uint8&in) The frequency of the dynamic updates.
  * @public
  **/
  void SetDynamicFrequency(const uint8&in frequency)
  {
    Violet_Components_Light::SetDynamicFrequency(GetId(), frequency);
  }
  /**
  * @brief Returns how many frames it should take for a dynamic light to be updated.
  * @return (uint8) The frequency of the dynamic updates.
  * @public
  **/
  uint8 GetDynamicFrequency()
  {
    return Violet_Components_Light::GetDynamicFrequency(GetId());
  }

  void MakeRSM()
  {
    Violet_Components_Light::MakeRSM(GetId());
  }
}
