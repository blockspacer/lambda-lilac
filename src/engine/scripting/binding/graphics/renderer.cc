#include <scripting/binding/graphics/renderer.h>
#include <interfaces/iworld.h>
#include <interfaces/irenderer.h>

namespace lambda
{
  namespace scripting
  {
    namespace graphics
    {
      namespace renderer
      {
        world::IWorld* g_world;

        void SetVSync(const bool& vsync)
        {
          g_world->getRenderer()->setVSync(vsync);
        }
        bool GetVSync()
        {
          return g_world->getRenderer()->getVSync();
        }
        void SetRenderScale(const float& render_scale)
        {
          g_world->getRenderer()->setRenderScale(render_scale);
        }
        float GetRenderScale()
        {
          return g_world->getRenderer()->getRenderScale();
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_world = world;

          return Map<lambda::String, void*>{
            { "void Violet_Graphics_Renderer::SetVSync(const bool& in)",        (void*)SetVSync },
            { "bool Violet_Graphics_Renderer::SetVSync()",                      (void*)GetVSync },
            { "void Violet_Graphics_Renderer::SetRenderScale(const float& in)", (void*)SetRenderScale },
            { "float Violet_Graphics_Renderer::GetRenderScale()",               (void*)GetRenderScale }
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
