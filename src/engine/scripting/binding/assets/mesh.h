#include <containers/containers.h>
#include <assets/mesh.h>

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
      namespace mesh
      {
        extern asset::VioletMeshHandle Get(const uint64_t id);
        extern Map<lambda::String, void*> Bind(world::IWorld* world);
        void Unbind();
      }
    }
  }
}