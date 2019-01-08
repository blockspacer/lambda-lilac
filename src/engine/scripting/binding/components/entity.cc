#include "entity.h"

namespace lambda
{
  namespace scripting
  {
    namespace components
    {
      namespace entity
      {
        lambda::Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          // Not implemented.
          // WHICH IS GOOD
          return lambda::Map<lambda::String, void*>();
        }

        void Unbind()
        {
          // Not implemented.
          // WHICH IS GOOD
        }
      }
    }
  }
}