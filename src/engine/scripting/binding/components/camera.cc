#include <scripting/binding/components/camera.h>
#include <scripting/binding/assets/shader.h>
#include <systems/camera_system.h>
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
      namespace camera
      {
        lambda::components::CameraSystem* g_camera_system;
        entity::EntitySystem* g_entity_system;
        scripting::IScriptContext* g_script_context;
        world::IWorld* g_world;

        void Create(const uint64_t& id)
        {
          g_camera_system->addComponent(entity::Entity(id, g_entity_system));
        }
        void Destroy(const uint64_t& id)
        {
          g_camera_system->removeComponent(entity::Entity(id, g_entity_system));
        }
        void SetNear(const uint64_t& id, const float& near)
        {
          g_camera_system->setNearPlane(entity::Entity(id, g_entity_system), utilities::Distance(near));
        }
        float GetNear(const uint64_t& id)
        {
          return g_camera_system->getNearPlane(entity::Entity(id, g_entity_system)).asMeter();
        }
        void SetFar(const uint64_t& id, const float& far)
        {
          g_camera_system->setFarPlane(entity::Entity(id, g_entity_system), utilities::Distance(far));
        }
        float GetFar(const uint64_t& id)
        {
          return g_camera_system->getFarPlane(entity::Entity(id, g_entity_system)).asMeter();
        }
        void AddShaderPass(const uint64_t& id, const String& name, const uint64_t& shader_id, const void* input_array, const void* output_array)
        {
          scripting::ScriptArray input_values = g_script_context->scriptArray(input_array);
          scripting::ScriptArray output_values = g_script_context->scriptArray(output_array);
          Vector<platform::RenderTarget> input(input_values.vec_string.size());
          Vector<platform::RenderTarget> output(output_values.vec_string.size());
          asset::ShaderHandle shader = assets::shader::Get(shader_id);

          for (uint32_t i = 0; i < input_values.vec_string.size(); ++i)
            input.at(i) = g_world->getPostProcessManager().getTarget(input_values.vec_string.at(i));
          for (uint32_t i = 0; i < output_values.vec_string.size(); ++i)
            output.at(i) = g_world->getPostProcessManager().getTarget(output_values.vec_string.at(i));

          g_camera_system->getComponent(
            entity::Entity(id, g_entity_system)
          ).addShaderPass(platform::ShaderPass(name, shader, input, output));
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_camera_system = world->getScene().getSystem<lambda::components::CameraSystem>().get();
          g_entity_system = world->getScene().getSystem<entity::EntitySystem>().get();
          g_script_context = world->getScripting().get();
          g_world = world;

          return Map<lambda::String, void*>{
            { "void Violet_Components_Camera::Create(const uint64& in)",                   (void*)Create },
            { "void Violet_Components_Camera::Destroy(const uint64& in)",                  (void*)Destroy },
            { "float Violet_Components_Camera::GetNear(const uint64& in)",                 (void*)GetNear },
            { "void Violet_Components_Camera::SetNear(const uint64& in, const float& in)", (void*)SetNear },
            { "float Violet_Components_Camera::GetFar(const uint64& in)",                  (void*)GetFar },
            { "void Violet_Components_Camera::SetFar(const uint64& in, const float& in)",  (void*)SetFar },
            { "void Violet_Components_Camera::AddShaderPass(const uint64& in, const String& in, const uint64& in, const Array<String>& in, const Array<String>& in)", (void*)AddShaderPass }
          };
        }

        void Unbind()
        {
          g_camera_system = nullptr;
          g_entity_system = nullptr;
          g_script_context = nullptr;
          g_world = nullptr;
        }
      }
    }
  }
}
