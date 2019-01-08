#include <containers/containers.h>
#include <assets/texture.h>

namespace lambda
{
  namespace world
  {
    class IWorld;
  }
  namespace scripting
  {
    namespace assets
    {
      namespace texture
      {
        extern asset::VioletTextureHandle Get(const uint64_t& id);
        extern Map<lambda::String, void*> Bind(world::IWorld* world);
        void Unbind();
      }
    }
  }
}
