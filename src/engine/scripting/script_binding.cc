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

namespace lambda
{
  namespace scripting
  {
    void ScriptBinding(world::IWorld* world)
    {
      assets::mesh::Bind(world);
      assets::shader::Bind(world);
      assets::texture::Bind(world);
      assets::wave::Bind(world);
      components::camera::Bind(world);
      components::collider::Bind(world);
      components::entity::Bind(world);
      components::lights::Bind(world);
      components::lod::Bind(world);
      components::meshrender::Bind(world);
      components::rigidbody::Bind(world);
      components::transform::Bind(world);
      components::wavesource::Bind(world);
      graphics::globals::Bind(world);
      graphics::renderer::Bind(world);
      input::controller::Bind(world);
      input::keyboard::Bind(world);
      input::mouse::Bind(world);
      math::quat::Bind(world);
      math::vec2::Bind(world);
      math::vec2::Bind(world);
      math::vec4::Bind(world);
      utilities::ini::Bind(world);
      utilities::rendertarget::Bind(world);
      utilities::shaderpass::Bind(world);
      utilities::utilities::Bind(world);
    }
    void ScriptRelease()
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