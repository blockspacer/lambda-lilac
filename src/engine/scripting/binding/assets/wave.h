#include <containers/containers.h>
#include <assets/wave.h>

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
      namespace wave
      {
        extern asset::VioletWaveHandle Get(const uint64_t& id);
        extern uint64_t Count();
        extern Map<lambda::String, void*> Bind(world::IWorld* world);
        void Unbind();
      }
    }
  }
}
