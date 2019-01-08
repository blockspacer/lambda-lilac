/** @file collider.as */
#include "icomponent.as"
#include "mesh.as"

/**
* @addtogroup Components
* @{
**/

/**
* @class Collider
* @brief A collider component which you can attach to an entity
* @author Hilze Vonck
**/
class Collider : IComponent
{
  void Initialize() final
  {
    Violet_Components_Collider::Create(GetId());
  }
  void Destroy() final
  {
    Violet_Components_Collider::Destroy(GetId());
  }
  void MakeBoxCollider()
  {
    Violet_Components_Collider::MakeBoxCollider(GetId());
  }
  void MakeSphereCollider()
  {
    Violet_Components_Collider::MakeSphereCollider(GetId());
  }
  void MakeCapsuleCollider()
  {
    Violet_Components_Collider::MakeCapsuleCollider(GetId());
  }
  void MakeMeshCollider(const Asset::Mesh&in mesh)
  {
    Violet_Components_Collider::MakeMeshCollider(GetId(), mesh.GetId());
  }
}
void MakeMeshCollider(const uint64&in id, const Asset::Mesh&in mesh)
{
  Violet_Components_Collider::MakeMeshCollider(id, mesh.GetId());
}

/**
* @}
**/
