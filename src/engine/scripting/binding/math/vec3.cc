#include <scripting/binding/math/vec3.h>

namespace lambda
{
  namespace scripting
  {
    namespace math
    {
      namespace vec3
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