#include <scripting/binding/graphics/globals.h>
#include <interfaces/iworld.h>
#include <interfaces/iscript_context.h>
#include <scripting/binding/assets/shader.h>
#include <systems/light_system.h>

namespace lambda
{
  namespace scripting
  {
    namespace graphics
    {
      namespace globals
      {
        scripting::IScriptContext* g_script_context;
        components::LightSystem* g_light_system;

        void SetShadersDirectional(const uint64_t& generate, const void* input_array, const uint64_t& publish)
        {
          scripting::ScriptArray input_values = g_script_context->scriptArray(input_array);
          Vector<asset::ShaderHandle> modify(input_values.vec_uint64.size());

          for (uint32_t i = 0; i < input_values.vec_uint64.size(); ++i)
          {
            modify.at(i) = assets::shader::Get(input_values.vec_uint64.at(i));
          }

          asset::ShaderHandle generate_shader = assets::shader::Get(generate);
          asset::ShaderHandle publish_shader  = assets::shader::Get(publish);

          g_light_system->setShadersDirectional(generate_shader, modify, publish_shader);
        }
        void SetShadersPoint(const uint64_t& generate, const void* input_array, const uint64_t& publish)
        {
          scripting::ScriptArray input_values = g_script_context->scriptArray(input_array);
          Vector<asset::ShaderHandle> modify(input_values.vec_uint64.size());

          for (uint32_t i = 0; i < input_values.vec_uint64.size(); ++i)
          {
            modify.at(i) = assets::shader::Get(input_values.vec_uint64.at(i));
          }

          asset::ShaderHandle generate_shader = assets::shader::Get(generate);
          asset::ShaderHandle publish_shader  = assets::shader::Get(publish);

          g_light_system->setShadersPoint(generate_shader, modify, publish_shader);
        }
        void SetShadersSpot(const uint64_t& generate, const void* input_array, const uint64_t& publish)
        {
          scripting::ScriptArray input_values = g_script_context->scriptArray(input_array);
          Vector<asset::ShaderHandle> modify(input_values.vec_uint64.size());

          for (uint32_t i = 0; i < input_values.vec_uint64.size(); ++i)
          {
            modify.at(i) = assets::shader::Get(input_values.vec_uint64.at(i));
          }

          asset::ShaderHandle generate_shader = assets::shader::Get(generate);
          asset::ShaderHandle publish_shader  = assets::shader::Get(publish);

          g_light_system->setShadersSpot(generate_shader, modify, publish_shader);
        }
        void SetShadersCascade(const uint64_t& generate, const void* input_array, const uint64_t& publish)
        {
          scripting::ScriptArray input_values = g_script_context->scriptArray(input_array);
          Vector<asset::ShaderHandle> modify(input_values.vec_uint64.size());

          for (uint32_t i = 0; i < input_values.vec_uint64.size(); ++i)
          {
            modify.at(i) = assets::shader::Get(input_values.vec_uint64.at(i));
          }

          asset::ShaderHandle generate_shader = assets::shader::Get(generate);
          asset::ShaderHandle publish_shader  = assets::shader::Get(publish);

          g_light_system->setShadersCascade(generate_shader, modify, publish_shader);
        }
        void SetShadersDirectionalRSM(const uint64_t& generate, const void* input_array, const uint64_t& publish)
        {
          scripting::ScriptArray input_values = g_script_context->scriptArray(input_array);
          Vector<asset::ShaderHandle> modify(input_values.vec_uint64.size());

          for (uint32_t i = 0; i < input_values.vec_uint64.size(); ++i)
          {
            modify.at(i) = assets::shader::Get(input_values.vec_uint64.at(i));
          }

          asset::ShaderHandle generate_shader = assets::shader::Get(generate);
          asset::ShaderHandle publish_shader  = assets::shader::Get(publish);

          g_light_system->setShadersDirectionalRSM(generate_shader, modify, publish_shader);
        }
        void SetShadersSpotRSM(const uint64_t& generate, const void* input_array, const uint64_t& publish)
        {
          scripting::ScriptArray input_values = g_script_context->scriptArray(input_array);
          Vector<asset::ShaderHandle> modify(input_values.vec_uint64.size());

          for (uint32_t i = 0; i < input_values.vec_uint64.size(); ++i)
          {
            modify.at(i) = assets::shader::Get(input_values.vec_uint64.at(i));
          }

          asset::ShaderHandle generate_shader = assets::shader::Get(generate);
          asset::ShaderHandle publish_shader  = assets::shader::Get(publish);

          g_light_system->setShadersSpotRSM(generate_shader, modify, publish_shader);
        }

        Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_script_context = world->getScripting().get();
          g_light_system = world->getScene().getSystem<components::LightSystem>().get();

          return Map<lambda::String, void*>{
            { "void Violet_Graphics_Globals::SetShadersDirectional(const uint64& in, const Array<uint64>& in, const uint64& in)",    (void*)SetShadersDirectional },
            { "void Violet_Graphics_Globals::SetShadersPoint(const uint64& in, const Array<uint64>& in, const uint64& in)",          (void*)SetShadersPoint },
            { "void Violet_Graphics_Globals::SetShadersSpot(const uint64& in, const Array<uint64>& in, const uint64& in)",           (void*)SetShadersSpot },
            { "void Violet_Graphics_Globals::SetShadersCascade(const uint64& in, const Array<uint64>& in, const uint64& in)",        (void*)SetShadersCascade },
            { "void Violet_Graphics_Globals::SetShadersDirectionalRSM(const uint64& in, const Array<uint64>& in, const uint64& in)", (void*)SetShadersDirectionalRSM },
            { "void Violet_Graphics_Globals::SetShadersSpotRSM(const uint64& in, const Array<uint64>& in, const uint64& in)",        (void*)SetShadersSpotRSM }
          };
        }

        void Unbind()
        {
          g_light_system = nullptr;
          g_script_context = nullptr;
        }
      }
    }
  }
}
