/** @file listener.as */
#include "icomponent.as"

/**
* @addtogroup Components
* @{
**/

/**
* @class Listener
* @brief A listener component which you can attach to an entity
* @author Hilze Vonck
**/
class Listener : IComponent
{
    void Initialize() final
    {
        Violet_Components_WaveSource::Create(GetId());
    }
    void Destroy() final
    {
        Violet_Components_WaveSource::Destroy(GetId());
    }

    void MakeMainListener() const
    {
        Violet_Components_WaveSource::SetListener(GetId());
    }
}

/**
* @}
**/
