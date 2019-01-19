#include <scripting/binding/components/wave_source.h>
#include <scripting/binding/assets/wave.h>
#include <systems/wave_source_system.h>
#include <systems/entity_system.h>
#include <interfaces/iworld.h>
#include <platform/scene.h>

namespace lambda
{
  namespace scripting
  {
    namespace components
    {
      namespace wavesource
      {
        lambda::components::WaveSourceSystem* g_wave_source_system;
        entity::EntitySystem* g_entity_system;

        void Create(const uint64_t& id)
        {
          g_wave_source_system->addComponent((entity::Entity)id);
        }
        void Destroy(const uint64_t& id)
        {
          g_wave_source_system->removeComponent((entity::Entity)id);
        }
        void SetBuffer(const uint64_t& id, const uint64_t& wave_id)
        {
          g_wave_source_system->setBuffer((entity::Entity)id, assets::wave::Get(wave_id));
        }
        uint64_t GetBuffer(const uint64_t& id)
        {
          auto buffer = g_wave_source_system->getBuffer((entity::Entity)id);

          for (uint64_t i = 0u; i < assets::wave::Count(); ++i)
            if (assets::wave::Get(i) == buffer)
              return i;

          return UINT64_MAX;
        }
        void Play(const uint64_t& id)
        {
          g_wave_source_system->play((entity::Entity)id);
        }
        void Pause(const uint64_t& id)
        {
          g_wave_source_system->pause((entity::Entity)id);
        }
        void Stop(const uint64_t& id)
        {
          g_wave_source_system->stop((entity::Entity)id);
        }
        uint8_t GetState(const uint64_t& id)
        {
          return (uint8_t)g_wave_source_system->getState((entity::Entity)id);
        }
        void SetRelativeToListener(const uint64_t& id, const bool& relative_to_listener)
        {
          g_wave_source_system->setRelativeToListener((entity::Entity)id, relative_to_listener);
        }
        bool GetRelativeToListener(const uint64_t& id)
        {
          return g_wave_source_system->getRelativeToListener((entity::Entity)id);
        }
        void SetLoop(const uint64_t& id, const bool& loop)
        {
          g_wave_source_system->setLoop((entity::Entity)id, loop);
        }
        bool GetLoop(const uint64_t& id)
        {
          return g_wave_source_system->getLoop((entity::Entity)id);
        }
        void SetOffset(const uint64_t& id, const float& offset)
        {
          g_wave_source_system->setOffset((entity::Entity)id, offset);
        }
        void SetVolume(const uint64_t& id, const float& volume)
        {
          g_wave_source_system->setVolume((entity::Entity)id, volume);
        }
        float GetVolume(const uint64_t& id)
        {
          return g_wave_source_system->getVolume((entity::Entity)id);
        }
        void SetGain(const uint64_t& id, const float& gain)
        {
          g_wave_source_system->setGain((entity::Entity)id, gain);
        }
        float GetGain(const uint64_t& id)
        {
          return g_wave_source_system->getGain((entity::Entity)id);
        }
        void SetPitch(const uint64_t& id, const float& pitch)
        {
          g_wave_source_system->setPitch((entity::Entity)id, pitch);
        }
        float GetPitch(const uint64_t& id)
        {
          return g_wave_source_system->getPitch((entity::Entity)id);
        }
        void SetRadius(const uint64_t& id, const float& radius)
        {
          g_wave_source_system->setRadius((entity::Entity)id, radius);
        }
        float GetRadius(const uint64_t& id)
        {
          return g_wave_source_system->getRadius((entity::Entity)id);
        }
        void SetListener(const uint64_t& id)
        {
          g_wave_source_system->setListener((entity::Entity)id);
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_wave_source_system = world->getScene().getSystem<lambda::components::WaveSourceSystem>().get();
          g_entity_system = world->getScene().getSystem<entity::EntitySystem>().get();

          return Map<lambda::String, void*>{
            { "void Violet_Components_WaveSource::Create(const uint64&in)", (void*)Create },
            { "void Violet_Components_WaveSource::Destroy(const uint64&in)", (void*)Destroy },
            { "void Violet_Components_WaveSource::SetBuffer(const uint64&in, const uint64&in)", (void*)SetBuffer },
            { "uint64 Violet_Components_WaveSource::GetBuffer(const uint64&in)", (void*)GetBuffer },
            { "void Violet_Components_WaveSource::Play(const uint64&in)", (void*)Play },
            { "void Violet_Components_WaveSource::Pause(const uint64&in)", (void*)Pause },
            { "void Violet_Components_WaveSource::Stop(const uint64&in)", (void*)Stop },
            { "uint8 Violet_Components_WaveSource::GetState(const uint64&in)", (void*)GetState },
            { "void Violet_Components_WaveSource::SetRelativeToListener(const uint64&in, const bool&in)", (void*)SetRelativeToListener },
            { "bool Violet_Components_WaveSource::GetRelativeToListener(const uint64&in)", (void*)GetRelativeToListener },
            { "void Violet_Components_WaveSource::SetLoop(const uint64&in, const bool&in)", (void*)SetLoop },
            { "bool Violet_Components_WaveSource::GetLoop(const uint64&in)", (void*)GetLoop },
            { "void Violet_Components_WaveSource::SetOffset(const uint64&in, const float&in)", (void*)SetOffset },
            { "void Violet_Components_WaveSource::SetVolume(const uint64&in, const float&in)", (void*)SetVolume },
            { "float Violet_Components_WaveSource::GetVolume(const uint64&in)", (void*)GetVolume },
            { "void Violet_Components_WaveSource::SetGain(const uint64&in, const float&in)", (void*)SetGain },
            { "float Violet_Components_WaveSource::GetGain(const uint64&in)", (void*)GetGain },
            { "void Violet_Components_WaveSource::SetPitch(const uint64&in, const float&in)", (void*)SetPitch },
            { "float Violet_Components_WaveSource::GetPitch(const uint64&in)", (void*)GetPitch },
            { "void Violet_Components_WaveSource::SetRadius(const uint64&in, const float&in)", (void*)SetRadius },
            { "float Violet_Components_WaveSource::GetRadius(const uint64&in)", (void*)GetRadius },
            { "void Violet_Components_WaveSource::SetListener(const uint64&in)", (void*)SetListener }
          };
        }

        void Unbind()
        {
          g_wave_source_system = nullptr;
          g_entity_system = nullptr;
        }
      }
    }
  }
}
