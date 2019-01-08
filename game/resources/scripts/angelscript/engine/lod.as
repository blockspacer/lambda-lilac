/** @file lod.as */
#include "icomponent.as"
#include "mesh.as"

/**
* @addtogroup Components
* @{
**/

/**
* @class LOD
* @brief A lod component which you can attach to an entity
* @author Hilze Vonck
**/
class LOD : IComponent
{
  void Initialize() final
  {
    Violet_Components_LOD::Create(GetId());
  }
  void Destroy() final
  {
    Violet_Components_LOD::Destroy(GetId());
  }
  void AddLOD(const Asset::Mesh&in mesh, const float&in distance)
  {
    Violet_Components_LOD::AddLOD(GetId(), mesh.GetId(), distance);
  }
}

void AddLODRecursive(const uint64&in id, const Asset::Mesh&in mesh, const float&in distance)
{
  Violet_Components_LOD::AddLODRecursive(id, mesh.GetId(), distance);
}

/**
* @}
**/
