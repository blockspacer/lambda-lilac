#include <scripting/binding/utilities/shader_pass.h>
#include <scripting/binding/assets/shader.h>
#include <platform/shader_pass.h>
#include <platform/post_process_manager.h>
#include <interfaces/iworld.h>
#include <interfaces/iscript_context.h>
#include <interfaces/irenderer.h>

namespace lambda
{
  namespace scripting
  {
    namespace utilities
    {
      namespace shaderpass
      {
        world::IWorld* g_world;
        scripting::IScriptContext* g_script_context;

        void Register(const String& name, const uint64_t& shader_id, const void* input_array, const void* output_array)
        {
          scripting::ScriptArray input_values = g_script_context->scriptArray(input_array);
          scripting::ScriptArray output_values = g_script_context->scriptArray(output_array);
          Vector<platform::RenderTarget> input(input_values.vec_string.size());
          Vector<platform::RenderTarget> output(output_values.vec_string.size());
          asset::ShaderHandle shader = assets::shader::Get(shader_id);

          for (uint32_t i = 0; i < input_values.vec_string.size(); ++i)
          {
            input.at(i) = g_world->getPostProcessManager().getTarget(input_values.vec_string.at(i));
          }
          for (uint32_t i = 0; i < output_values.vec_string.size(); ++i)
          {
            output.at(i) = g_world->getPostProcessManager().getTarget(output_values.vec_string.at(i));
          }

          g_world->getPostProcessManager().addPass(
            platform::ShaderPass(name, shader, input, output)
          );
        }
        void SetEnabled(const String& name, const bool& enabled)
        {
          Name _name(name);
          Vector<platform::ShaderPass>& passes = g_world->getPostProcessManager().getPasses();
          for (auto& pass : passes)
          {
            if (pass.getName() == _name)
            {
              pass.setEnabled(enabled);
            }
          }
        }
        void SetVariableFloat1(const String& name, const float& v1)
        {
          g_world->getRenderer()->setShaderVariable(platform::ShaderVariable(Name(name), v1));
        }
        void SetVariableFloat2(const String& name, const scripting::ScriptVec2& v1)
        {
          g_world->getRenderer()->setShaderVariable(platform::ShaderVariable(Name(name), glm::vec2(v1.x, v1.y)));
        }
        void SetVariableFloat3(const String& name, const scripting::ScriptVec3& v1)
        {
          g_world->getRenderer()->setShaderVariable(platform::ShaderVariable(Name(name), glm::vec3(v1.x, v1.y, v1.z)));
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_world = world;
          g_script_context = world->getScripting().get();

          return Map<lambda::String, void*>{
            { "void Violet_Utilities_ShaderPass::Register(const String& in, const uint64& in, const Array<String>& in, const Array<String>& in)", (void*)Register },
            { "void Violet_Utilities_ShaderPass::SetEnabled(const String& in, const bool& in)",                                                   (void*)SetEnabled },
            { "void Violet_Utilities_ShaderPass::SetShaderVariable(const String& in, const float& in)",                                           (void*)SetVariableFloat1 },
            { "void Violet_Utilities_ShaderPass::SetShaderVariable(const String& in, const Vec2& in)",                                            (void*)SetVariableFloat2 },
            { "void Violet_Utilities_ShaderPass::SetShaderVariable(const String& in, const Vec3& in)",                                            (void*)SetVariableFloat3 }
          };
        }

        void Unbind()
        {
          g_world = nullptr;
          g_script_context = nullptr;
        }
      }
    }
  }
}
