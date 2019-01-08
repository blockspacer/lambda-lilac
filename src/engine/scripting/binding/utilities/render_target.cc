#include <scripting/binding/utilities/render_target.h>
#include <scripting/binding/assets/texture.h>
#include <platform/render_target.h>
#include <interfaces/iworld.h>
#include <interfaces/irenderer.h>
#include <platform/post_process_manager.h>
#include <assets/asset_manager.h>
#include <assets/shader_io.h>
#include <assets/texture.h>
#include <algorithm>

namespace lambda
{
  namespace scripting
  {
    namespace utilities
    {
      namespace rendertarget
      {
        world::IWorld* g_world;

        void Register(const String& name, const float& render_scale, const uint64_t& texture_id, const bool& from_texture)
        {
          asset::VioletTextureHandle texture = assets::texture::Get(texture_id);
          g_world->getPostProcessManager().addTarget(
            platform::RenderTarget(name, render_scale, texture, from_texture)
          );
        }
        void SetFinalRenderTarget(const String& name)
        {
          g_world->getPostProcessManager().setFinalTarget(name);
        }
        enum RenderTargetFlags
        {
          kClear = 0,
          kResize = 1
        };
        void SetFlag(const String& name, const uint8_t& flag, const bool& value)
        {
          platform::RenderTarget& render_target = g_world->getPostProcessManager().getTarget(name);

          switch (flag)
          {
          case RenderTargetFlags::kClear:
            render_target.getTexture()->getLayer(0u).setFlags(render_target.getTexture()->getLayer(0u).getFlags() | kTextureFlagClear);
            break;
          case RenderTargetFlags::kResize:
            render_target.getTexture()->getLayer(0u).setFlags(render_target.getTexture()->getLayer(0u).getFlags() | kTextureFlagResize);
            break;
          }
        }
        void Hammerhead(const String& input, const String& output)
        {
          platform::RenderTarget& rt_input = g_world->getPostProcessManager().getTarget(input);
          asset::ShaderHandle shader = asset::AssetManager::getInstance().createAsset(Name(output + "_Hammerhead"),
            foundation::Memory::constructShared<asset::Shader>(
              io::ShaderIO::asAsset(io::ShaderIO::load("resources/shaders/hammerhead.fx"))
              )
          );

          float as = (float)rt_input.getTexture()->getLayer(0u).getHeight() / (float)rt_input.getTexture()->getLayer(0u).getWidth();
          VioletTexture violet_texture;
          violet_texture.width     = 512u;
          violet_texture.height    = (uint32_t)(512.0f * as);
          violet_texture.mip_count = 1u + (uint16_t)floor(log10((float)std::max(violet_texture.width, violet_texture.height)) / log10(2.0f));
          violet_texture.file      = output;
          violet_texture.flags     = kTextureFlagIsRenderTarget;
          violet_texture.format    = TextureFormat::kR16G16B16A16;
          violet_texture.hash      = hash(output);
          asset::VioletTextureHandle texture = asset::TextureManager::getInstance()->create(output, violet_texture);

          auto mesh = asset::AssetManager::getInstance().createAsset<asset::Mesh>(Name("__hammerhead_mesh__"),
            foundation::Memory::constructShared<asset::Mesh>(asset::Mesh::createScreenQuad())
            );
          g_world->getRenderer()->setMesh(mesh);
          g_world->getRenderer()->setSubMesh(0u);

          platform::RenderTarget rt_in(Name("Hammerhead_In"), texture);
          for (uint32_t i = 0u; i < violet_texture.mip_count; ++i)
          {
            g_world->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("roughness"), ((float)i / (float)violet_texture.mip_count)));
            platform::RenderTarget rt_out(Name("Hammerhead_Out_" + toString(i)), texture);
            rt_out.setMipMap(i);

            platform::ShaderPass shader_pass(
              Name("Hammerhead_" + toString(i)),
              shader,
              { rt_input },
              //{ i == 0u ? rt_input : rt_in },
              { rt_out }
            );

            g_world->getRenderer()->bindShaderPass(shader_pass);
            g_world->getRenderer()->draw();
          }

          g_world->getPostProcessManager().addTarget(platform::RenderTarget(Name(output), texture));
        }
        void IrradianceConvolution(const String& input, const String& output)
        {
          platform::RenderTarget& rt_input = g_world->getPostProcessManager().getTarget(input);
          asset::ShaderHandle shader = asset::AssetManager::getInstance().createAsset(Name(output + "_IrradianceConvolution"),
            foundation::Memory::constructShared<asset::Shader>(
              io::ShaderIO::asAsset(io::ShaderIO::load("resources/shaders/irradiance_convolution.fx"))
              )
          );

          float as = (float)rt_input.getTexture()->getLayer(0u).getHeight() / (float)rt_input.getTexture()->getLayer(0u).getWidth();
          
          VioletTexture violet_texture;
          violet_texture.width     = 512u;
          violet_texture.height    = (uint32_t)(512.0f * as);
          violet_texture.mip_count = 1u;
          violet_texture.file      = output;
          violet_texture.flags     = kTextureFlagIsRenderTarget;
          violet_texture.format    = TextureFormat::kR16G16B16A16;
          violet_texture.hash      = hash(output);
          asset::VioletTextureHandle texture = asset::TextureManager::getInstance()->create(output, violet_texture);

          auto mesh = asset::AssetManager::getInstance().createAsset<asset::Mesh>(Name("__irradiance_convolution_mesh__"),
            foundation::Memory::constructShared<asset::Mesh>(asset::Mesh::createScreenQuad())
          );
          g_world->getRenderer()->setMesh(mesh);
          g_world->getRenderer()->setSubMesh(0u);

          platform::RenderTarget rt_out(Name("IrradianceConvolution_Out"), texture);

          platform::ShaderPass shader_pass(
            Name("IrradianceConvolution"),
            shader,
            { rt_input },
            { rt_out }
          );

          g_world->getRenderer()->bindShaderPass(shader_pass);
          g_world->getRenderer()->draw();

          g_world->getPostProcessManager().addTarget(platform::RenderTarget(Name(output), texture));
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_world = world;

          return Map<lambda::String, void*>{
            { "void Violet_Utilities_RenderTarget::Register(const String& in, const float& in, const uint64& in, const bool& in)", (void*)Register },
            { "void Violet_Utilities_RenderTarget::SetFinalRenderTarget(const String& in)",                                        (void*)SetFinalRenderTarget },
            { "void Violet_Utilities_RenderTarget::SetFlag(const String& in, const uint8& in, const bool& in)",                    (void*)SetFlag },
            { "void Violet_Utilities_RenderTarget::Hammerhead(const String& in, const String& in)",                                (void*)Hammerhead },
            { "void Violet_Utilities_RenderTarget::IrradianceConvolution(const String& in, const String& in)",                     (void*)IrradianceConvolution },
          };
        }

        void Unbind()
        {
          g_world = nullptr;
        }
      }
    }
  }
}
