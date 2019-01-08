#include <containers/containers.h>
#include <assets/shader.h>

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
      namespace shader
      {
        extern asset::ShaderHandle Get(const uint64_t& id);
        extern Map<lambda::String, void*> Bind(world::IWorld* world);
        void Unbind();
      }
    }
  }
}