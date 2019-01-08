#include <containers/containers.h>

namespace lambda
{
  namespace world
  {
    class IWorld;
  }
  namespace scripting
  {
    namespace graphics
    {
      namespace renderer
      {
        extern Map<lambda::String, void*> Bind(world::IWorld* world);
        void Unbind();
      }
    }
  }
}
