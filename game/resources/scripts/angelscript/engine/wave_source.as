/** @file wave_source.as */
#include "wave.as"
#include "enums.as"
#include "icomponent.as"

/**
* @addtogroup Components
* @{
**/

/**
* @class WaveSource
* @brief A wave source component which you can attach to an entity
* @author Hilze Vonck
**/
class WaveSource : IComponent
{
    void Initialize() final
    {
        Violet_Components_WaveSource::Create(GetId());
    }
    void Destroy() final
    {
        Violet_Components_WaveSource::Destroy(GetId());
    }

    void SetBuffer(const Asset::Wave&in buffer)
    {
        Violet_Components_WaveSource::SetBuffer(GetId(), buffer.GetId());
    }
    Asset::Wave GetBuffer() const
    {
        return Asset::Wave(Violet_Components_WaveSource::GetBuffer(GetId()));
    }
    void Play()
    {
        Violet_Components_WaveSource::Play(GetId());
    }
    void Pause()
    {
        Violet_Components_WaveSource::Pause(GetId());
    }
    void Stop()
    {
        Violet_Components_WaveSource::Play(GetId());
    }
    WaveSourceState GetState() const
    {
        return WaveSourceState(Violet_Components_WaveSource::GetState(GetId()));
    }
    void SetRelativeToListener(const bool&in relative)
    {
        Violet_Components_WaveSource::SetRelativeToListener(GetId(), relative);
    }
    bool GetRelativeToListener() const
    {
        return Violet_Components_WaveSource::GetRelativeToListener(GetId());
    }
    void SetLoop(const bool&in loop)
    {
        Violet_Components_WaveSource::SetLoop(GetId(), loop);
    }
    bool GetLoop() const
    {
        return Violet_Components_WaveSource::GetLoop(GetId());
    }
    void SetOffset(const float&in seconds)
    {
        Violet_Components_WaveSource::SetOffset(GetId(), seconds);
    }
    void SetVolume(const float&in volume)
    {
        Violet_Components_WaveSource::SetVolume(GetId(), volume);
    }
    float GetVolume() const
    {
        return Violet_Components_WaveSource::GetVolume(GetId());
    }
    void SetGain(const float&in gain)
    {
        Violet_Components_WaveSource::SetGain(GetId(), gain);
    }
    float GetGain() const
    {
      return Violet_Components_WaveSource::GetGain(GetId());
    }
    void SetPitch(const float&in pitch)
    {
        Violet_Components_WaveSource::SetPitch(GetId(), pitch);
    }
    float GetPitch() const
    {
      return Violet_Components_WaveSource::GetPitch(GetId());
    }
    void SetRadius(const float&in radius)
    {
        Violet_Components_WaveSource::SetRadius(GetId(), radius);
    }
    float GetRadius() const
    {
      return Violet_Components_WaveSource::GetPitch(GetId());
    }
}

/**
* @}
**/
