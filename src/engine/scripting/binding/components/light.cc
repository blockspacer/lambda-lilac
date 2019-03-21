#include <scripting/binding/components/light.h>
#include <scripting/binding/assets/texture.h>
#include <systems/light_system.h>
#include <systems/entity_system.h>
#include <interfaces/iworld.h>
#include <interfaces/irenderer.h>
#include <interfaces/iscript_context.h>
#include <platform/post_process_manager.h>

namespace lambda
{
  namespace scripting
  {
    namespace components
    {
      namespace lights
      {
        world::IWorld* g_world;
        entity::EntitySystem* g_entity_system;
        lambda::components::LightSystem* g_light_system;

        void CreateSpot(const uint64_t& id)
        {
          g_light_system->addSpotLight((entity::Entity)id);
        }
        void CreatePoint(const uint64_t& id)
        {
          g_light_system->addPointLight((entity::Entity)id);
        }
        void CreateDirectional(const uint64_t& id)
        {
          g_light_system->addDirectionalLight((entity::Entity)id);
        }
        void CreateCascade(const uint64_t& id)
        {
          g_light_system->addCascadedLight((entity::Entity)id);
        }
        void Destroy(const uint64_t& id)
        {
          g_light_system->removeComponent((entity::Entity)id);
        }
        void SetLightColour(const uint64_t& id, const ScriptVec3& colour)
        {
          g_light_system->setColour((entity::Entity)id, colour);
        }
        ScriptVec3 GetLightColour(const uint64_t& id)
        {
          return g_light_system->getColour((entity::Entity)id);
        }
        void SetLightIntensity(const uint64_t& id, const float& intensity)
        {
          g_light_system->setIntensity((entity::Entity)id, intensity);
        }
        float GetLightIntensity(const uint64_t& id)
        {
          return g_light_system->getIntensity((entity::Entity)id);
        }
        void SetAmbientColour(const uint64_t& id, const ScriptVec3& colour)
        {
          g_light_system->setAmbient((entity::Entity)id, colour);
        }
        ScriptVec3 GetAmbientColour(const uint64_t& id)
        {
          return g_light_system->getDirectionalLight((entity::Entity)id).getAmbient();
        }
        void SetShadowType(const uint64_t& id, const uint8_t& shadow_type)
        {
          g_light_system->setShadowType((entity::Entity)id, (lambda::components::ShadowType)shadow_type);
        }
        uint8_t GetShadowType(const uint64_t& id)
        {
          return (uint8_t)g_light_system->getShadowType((entity::Entity)id);
        }
        void SetDepth(const uint64_t& id, const float& depth)
        {
          g_light_system->setDepth((entity::Entity)id, depth);
        }
        float GetDepth(const uint64_t& id)
        {
          return g_light_system->getDepth((entity::Entity)id);
        }
        void SetRenderTargets(const uint64_t& id, const void* shadow_maps)
        {
          scripting::ScriptArray input_values = g_world->getScripting()->scriptArray(shadow_maps);
          Vector<platform::RenderTarget> maps(input_values.vec_string.size());
          for (uint32_t i = 0u; i < maps.size(); ++i)
            maps.at(i) = g_world->getPostProcessManager().getTarget(input_values.vec_string.at(i));

		  LMB_ASSERT(false, "Light::SetRenderTargets is not implemented!");
          //g_light_system->setRenderTarget((entity::Entity)id, maps);
        }
        void SetTexture(const uint64_t& id, const uint64_t& texture_id)
        {
          g_light_system->setTexture((entity::Entity)id, assets::texture::Get(texture_id));
        }
        void SetEnabled(const uint64_t& id, const bool& enabled)
        {
          g_light_system->setEnabled((entity::Entity)id, enabled);
        }
        bool GetEnabled(const uint64_t& id)
        {
          return g_light_system->getEnabled((entity::Entity)id);
        }
        void SetDynamicFrequency(const uint64_t& id, const uint8_t& frequency)
        {
          g_light_system->setDynamicFrequency((entity::Entity)id, frequency);
        }
        uint8_t GetDynamicFrequency(const uint64_t& id)
        {
          return g_light_system->getDynamicFrequency((entity::Entity)id);
        }
        void MakeRSM(const uint64_t& id)
        {
          g_light_system->setRSM((entity::Entity)id, true);
        }
        void SetInnerCutOff(const uint64_t& id, const float& inner_cut_off)
        {
          g_light_system->setCutOff((entity::Entity)id, utilities::Angle::fromRad(inner_cut_off));
        }
        float GetInnerCutOff(const uint64_t& id)
        {
          return g_light_system->getCutOff((entity::Entity)id).asRad();
        }
        void SetOuterCutOff(const uint64_t& id, const float& outer_cut_off)
        {
          g_light_system->setOuterCutOff((entity::Entity)id, utilities::Angle::fromRad(outer_cut_off));
        }
        float GetOuterCutOff(const uint64_t& id)
        {
          return g_light_system->getOuterCutOff((entity::Entity)id).asRad();
        }
        void SetSize(const uint64_t& id, const float& size)
        {
          g_light_system->setSize((entity::Entity)id, size);
        }
        float GetSize(const uint64_t& id)
        {
          return g_light_system->getSize((entity::Entity)id);
        }
     
        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_world = world;
          g_light_system = g_world->getScene().getSystem<lambda::components::LightSystem>().get();
          g_entity_system = g_world->getScene().getSystem<entity::EntitySystem>().get();

          return Map<lambda::String, void*>{
            { "void Violet_Components_Light::CreatePoint(const uint64& in)", (void*)CreatePoint },
            { "void Violet_Components_Light::CreateSpot(const uint64& in)", (void*)CreateSpot },
            { "void Violet_Components_Light::CreateDirectional(const uint64& in)", (void*)CreateDirectional },
            { "void Violet_Components_Light::CreateCascade(const uint64& in)", (void*)CreateCascade },
            { "void Violet_Components_Light::Destroy(const uint64& in)", (void*)Destroy },
            { "void Violet_Components_Light::SetLightColour(const uint64& in, const Vec3& in)", (void*)SetLightColour },
            { "Vec3 Violet_Components_Light::GetLightColour(const uint64& in)", (void*)GetLightColour },
            { "void Violet_Components_Light::SetLightIntensity(const uint64& in, const float& in)", (void*)SetLightIntensity },
            { "float Violet_Components_Light::GetLightIntensity(const uint64& in)", (void*)GetLightIntensity },
            { "void Violet_Components_Light::SetAmbientColour(const uint64& in, const Vec3& in)", (void*)SetAmbientColour },
            { "Vec3 Violet_Components_Light::GetAmbientColour(const uint64& in)", (void*)GetAmbientColour },
            { "void Violet_Components_Light::SetDepth(const uint64& in, const float& in)", (void*)SetDepth },
            { "float Violet_Components_Light::GetDepth(const uint64& in)", (void*)GetDepth },
            { "void Violet_Components_Light::SetShadowType(const uint64& in, const uint8& in)", (void*)SetShadowType },
            { "uint8 Violet_Components_Light::GetShadowType(const uint64& in)", (void*)GetShadowType },
            { "void Violet_Components_Light::SetRenderTargets(const uint64& in, const Array<String>& in)", (void*)SetRenderTargets },
            { "void Violet_Components_Light::SetTexture(const uint64& in, const uint64& in)", (void*)SetTexture },
            { "void Violet_Components_Light::SetEnabled(const uint64& in, const bool& in)", (void*)SetEnabled },
            { "bool Violet_Components_Light::GetEnabled(const uint64& in)", (void*)GetEnabled },
            { "void Violet_Components_Light::SetDynamicFrequency(const uint64& in, const uint8& in)", (void*)SetDynamicFrequency },
            { "uint8 Violet_Components_Light::GetDynamicFrequency(const uint64& in)", (void*)GetDynamicFrequency },
            { "void Violet_Components_Light::MakeRSM(const uint64& in)", (void*)MakeRSM },
            { "void Violet_Components_Light::SetSize(const uint64& in, const float& in)", (void*)SetSize },
            { "float Violet_Components_Light::GetSize(const uint64& in)", (void*)GetSize },
            { "void Violet_Components_Light::SetInnerCutOff(const uint64& in, const float& in)", (void*)SetInnerCutOff },
            { "float Violet_Components_Light::GetInnerCutOff(const uint64& in)", (void*)GetInnerCutOff },
            { "void Violet_Components_Light::SetOuterCutOff(const uint64& in, const float& in)", (void*)SetOuterCutOff },
            { "float Violet_Components_Light::GetOuterCutOff(const uint64& in)", (void*)GetOuterCutOff },
          };
        }

        void Unbind()
        {
          g_world = nullptr;
          g_light_system = nullptr;
          g_entity_system = nullptr;
        }
      }
    }
  }
}
