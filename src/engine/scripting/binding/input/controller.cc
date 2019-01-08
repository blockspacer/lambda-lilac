#include <scripting/binding/input/controller.h>

namespace lambda
{
  namespace scripting
  {
    namespace input
    {
      namespace controller
      {
        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          return Map<lambda::String, void*>{};
        }
        void Unbind()
        {

        }
      }
    }
  }
}