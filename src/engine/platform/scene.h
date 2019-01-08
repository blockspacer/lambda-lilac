#pragma once
#include "interfaces/isystem.h"
#include <containers/containers.h>
#include <memory/memory.h>

namespace lambda
{
  namespace world
  {
    ///////////////////////////////////////////////////////////////////////////
    class Scene
    {
    public:
      Scene();
			~Scene();

			void initialize(IWorld* world);

      template<typename T>
      foundation::SharedPointer<T> getSystem()
      {
        return eastl::static_shared_pointer_cast<T>(
          systems_.at(T::systemId())
        );
      }
      
      const Vector<foundation::SharedPointer<components::ISystem>>& 
        getAllSystems() const;
      
      Vector<foundation::SharedPointer<components::ISystem>>& getAllSystems();

    private:
      Vector<foundation::SharedPointer<components::ISystem>> systems_;
    };
  }
}