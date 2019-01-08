#include <scripting/binding/script_binding.h>
#include <scripting/binding/assets/mesh.h>
#include <scripting/binding/assets/shader.h>
#include <scripting/binding/assets/texture.h>
#include <scripting/binding/assets/wave.h>
#include <scripting/binding/components/camera.h>
#include <scripting/binding/components/collider.h>
#include <scripting/binding/components/entity.h>
#include <scripting/binding/components/light.h>
#include <scripting/binding/components/lod.h>
#include <scripting/binding/components/mesh.h>
#include <scripting/binding/components/rigid_body.h>
#include <scripting/binding/components/transform.h>
#include <scripting/binding/components/wave_source.h>
#include <scripting/binding/graphics/globals.h>
#include <scripting/binding/graphics/renderer.h>
#include <scripting/binding/input/controller.h>
#include <scripting/binding/input/keyboard.h>
#include <scripting/binding/input/mouse.h>
#include <scripting/binding/math/quat.h>
#include <scripting/binding/math/vec2.h>
#include <scripting/binding/math/vec2.h>
#include <scripting/binding/math/vec4.h>
#include <scripting/binding/utilities/ini.h>
#include <scripting/binding/utilities/render_target.h>
#include <scripting/binding/utilities/shader_pass.h>
#include <scripting/binding/utilities/utilities.h>
#include <interfaces/iscript_context.h>
#include <interfaces/iworld.h>

namespace lambda
{
  namespace scripting
  {
    extern void ScriptBinding(world::IWorld* world)
    {
      Map<lambda::String, void*> binding;
      auto Bind = [&](const Map<lambda::String, void*>& insert) {
        binding.insert(insert.begin(), insert.end());
      };
      Bind(assets::mesh::Bind(world));
      Bind(assets::shader::Bind(world));
      Bind(assets::texture::Bind(world));
      Bind(assets::wave::Bind(world));
      Bind(components::camera::Bind(world));
      Bind(components::collider::Bind(world));
      Bind(components::entity::Bind(world));
      Bind(components::lights::Bind(world));
      Bind(components::lod::Bind(world));
      Bind(components::meshrender::Bind(world));
      Bind(components::rigidbody::Bind(world));
      Bind(components::transform::Bind(world));
      Bind(components::wavesource::Bind(world));
      Bind(graphics::globals::Bind(world));
      Bind(graphics::renderer::Bind(world));
      Bind(input::controller::Bind(world));
      Bind(input::keyboard::Bind(world));
      Bind(input::mouse::Bind(world));
      Bind(math::quat::Bind(world));
      Bind(math::vec2::Bind(world));
      Bind(math::vec2::Bind(world));
      Bind(math::vec4::Bind(world));
      Bind(utilities::ini::Bind(world));
      Bind(utilities::rendertarget::Bind(world));
      Bind(utilities::shaderpass::Bind(world));
      Bind(utilities::utilities::Bind(world));
      world->getScripting()->initialize(binding);
    }
    extern void ScriptRelease()
    {
      assets::mesh::Unbind();
      assets::shader::Unbind();
      assets::texture::Unbind();
      assets::wave::Unbind();
      components::camera::Unbind();
      components::collider::Unbind();
      components::entity::Unbind();
      components::lights::Unbind();
      components::lod::Unbind();
      components::meshrender::Unbind();
      components::rigidbody::Unbind();
      components::transform::Unbind();
      components::wavesource::Unbind();
      graphics::globals::Unbind();
      graphics::renderer::Unbind();
      input::controller::Unbind();
      input::keyboard::Unbind();
      input::mouse::Unbind();
      math::quat::Unbind();
      math::vec2::Unbind();
      math::vec2::Unbind();
      math::vec4::Unbind();
      utilities::ini::Unbind();
      utilities::rendertarget::Unbind();
      utilities::shaderpass::Unbind();
      utilities::utilities::Unbind();
    }
  }
}