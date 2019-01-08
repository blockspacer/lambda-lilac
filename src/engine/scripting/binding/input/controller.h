#include <containers/containers.h>

namespace lambda
{
  namespace world
  {
    class IWorld;
  }
  namespace scripting
  {
    namespace input
    {
      namespace controller
      {
        extern Map<lambda::String, void*> Bind(world::IWorld* world);
        void Unbind();
      }
    }
  }
}