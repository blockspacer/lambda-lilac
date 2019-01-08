/** @file mesh_render.as */
#include "assets.as"
#include "icomponent.as"

/**
* @addtogroup Components
* @{
**/

/**
* @class MeshRender
* @brief Mesh render component which can be attached to an entity
* @author Hilze Vonck
**/
class MeshRender : IComponent
{
    void Initialize() final
    {
        Violet_Components_MeshRender::Create(GetId());
    }
    void Destroy() final
    {
        Violet_Components_MeshRender::Destroy(GetId());
    }
    /**
    * @brief Attaches a mesh to this mesh render. Will create child entities for all sub meshes
    * @param mesh (const Asset::Mesh&) The mesh that needs to be attached
    * @public
    **/
    void Attach(const Asset::Mesh&in mesh)
    {
        Violet_Components_MeshRender::Attach(GetId(), mesh.GetId());
    }
    /**
    * @brief Sets the mesh that will be used
    * @param mesh (const Asset::Mesh&) The mesh that will be used
    * @public
    **/
    void SetMesh(const Asset::Mesh&in mesh)
    {
        Violet_Components_MeshRender::SetMesh(GetId(), mesh.GetId());
    }
    /**
    * @brief Sets the sub mesh that will be used
    * @param sub_mesh (const uint16) The sub mesh that will be used
    * @public
    **/
    void SetSubMesh(const uint16&in sub_mesh)
    {
        Violet_Components_MeshRender::SetSubMesh(GetId(), sub_mesh);
    }

    void SetAlbedoTexture(const Asset::Texture&in texture)
    {
      Violet_Components_MeshRender::SetAlbedoTexture(GetId(), texture.GetId());
    }
    void SetNormalTexture(const Asset::Texture&in texture)
    {
      Violet_Components_MeshRender::SetNormalTexture(GetId(), texture.GetId());
    }
    void SetMetallicRoughnessTexture(const Asset::Texture&in texture)
    {
      Violet_Components_MeshRender::SetMetallicRoughnessTexture(GetId(), texture.GetId());
    }
    void MakeStatic()
    {
      Violet_Components_MeshRender::MakeStatic(GetId());
    }
    void MakeStaticRecursive()
    {
      Violet_Components_MeshRender::MakeStaticRecursive(GetId());
    }
    void MakeDynamic()
    {
      Violet_Components_MeshRender::MakeDynamic(GetId());
    }
    void MakeDynamicRecursive()
    {
      Violet_Components_MeshRender::MakeDynamicRecursive(GetId());
    }
}

/**
* @}
**/
