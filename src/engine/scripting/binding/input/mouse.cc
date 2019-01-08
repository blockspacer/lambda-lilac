#include <scripting/binding/input/mouse.h>

namespace lambda
{
  namespace scripting
  {
    namespace input
    {
      namespace mouse
      {
        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          return Map<lambda::String, void*>();
        }

        void Unbind()
        {

        }
      }
    }
  }
}