/** @file mesh.as */

/**
* @addtogroup Assets
* @{
**/

namespace Asset //! Namespace containing all asset related classes, enums and functions
{
  /**
  * @class Asset::Mesh
  * @brief A wrapper class for meshes. Allows you to interact with engine meshes.
  * @author Hilze Vonck
  **/
  class Mesh
  {
    /**
    * @brief Constructor which will initialize this mesh to invalid
    * @public
    **/
    Mesh()
    {
      this.id = ToUint64(-1);
    }
    /**
    * @brief Constructor which will initialize this mesh to a given id
    * @public
    **/
    Mesh(const uint64&in id)
    {
      this.id = id;
      if(id != ToUint64(-1))
      {
        Violet_Assets_Mesh::IncRef(id);
      }
    }
    /**
    * @brief Constructor which will initialize this mesh to the same id as the other mesh
    * @param other (Asset::Mesh) The mesh which needs to be copied
    * @public
    **/
    Mesh(Mesh other)
    {
      id = other.id;
      if(id != ToUint64(-1))
      {
        Violet_Assets_Mesh::IncRef(id);
      }
    }
    ~Mesh()
    {
      if(id != ToUint64(-1))
      {
        Violet_Assets_Mesh::DecRef(id);
      }
    }
    /**
    * @brief Returns the amount of sub meshes that this mesh has
    * @return (uint16) The amount of sub meshes that this mesh has
    * @public
    **/
    uint16 GetSubMeshCount() const
    {
      return Violet_Assets_Mesh::GetSubMeshCount(id);
    }
    /**
    * @brief Returns the engine id of this mesh
    * @return (uint16) The engine id of this mesh
    * @public
    **/
    uint64 GetId() const
    {
      return id;
    }

    Mesh Decimate(const float&in reduction = 0.5f, const float&in target_error = 1.0f) const
    {
      return Mesh(Violet_Assets_Mesh::Decimate(id, reduction, target_error));
    }

    void SetPositions(const Array<Vec3>&in pos)
    {
      Violet_Assets_Mesh::SetPositions(id, pos);
    }
    void SetNormals(const Array<Vec3>&in nor)
    {
      Violet_Assets_Mesh::SetNormals(id, nor);
    }
    void SetTexCoords(const Array<Vec2>&in tex)
    {
      Violet_Assets_Mesh::SetTexCoords(id, tex);
    }
    void SetColours(const Array<Vec4>&in col)
    {
      Violet_Assets_Mesh::SetColours(id, col);
    }
    void SetTangents(const Array<Vec3>&in tan)
    {
      Violet_Assets_Mesh::SetTangents(id, tan);
    }
    void RecalculateTangents()
    {
      Violet_Assets_Mesh::RecalculateTangents(id);
    }
    void SetJoints(const Array<Vec4>&in joi)
    {
      Violet_Assets_Mesh::SetJoints(id, joi);
    }
    void SetWeights(const Array<Vec4>&in wei)
    {
      Violet_Assets_Mesh::SetWeights(id, wei);
    }
    void SetIndices(const Array<uint32>&in idx)
    {
      Violet_Assets_Mesh::SetIndices(id, idx);
    }

    private uint64 id;
  }
}

/**
* @}
**/
