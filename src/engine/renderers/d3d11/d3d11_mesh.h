#pragma once
#include "assets/mesh.h"
#include <wrl/client.h>
#include <memory/memory.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

namespace lambda
{
  namespace windows
  {
    class D3D11Context;
    class D3D11RenderBuffer;

    ///////////////////////////////////////////////////////////////////////////
    enum class MeshStages : uint16_t
    {
      kTranslation = 1 << 0,
      kNormal      = 1 << 1,
      kTexCoord    = 1 << 2,
      kColour      = 1 << 3,
      kTangent     = 1 << 4,
      kJoint       = 1 << 5,
      kWeights     = 1 << 6,
      kCount       = 7u,
    };

    ///////////////////////////////////////////////////////////////////////////
    inline MeshStages operator|(MeshStages a, MeshStages b)
    {
      return static_cast<MeshStages>(static_cast<uint16_t>(a) | 
        static_cast<uint16_t>(b));
    }
    
    ///////////////////////////////////////////////////////////////////////////
    inline MeshStages operator|=(MeshStages a, MeshStages b)
    {
      return static_cast<MeshStages>(static_cast<uint16_t>(a) | 
        static_cast<uint16_t>(b));
    }

    ///////////////////////////////////////////////////////////////////////////
    class D3D11Mesh : public asset::IGPUAsset
    {
    public:
      D3D11Mesh(D3D11Context* context);
      virtual ~D3D11Mesh() override;

      void bind(
        const uint16_t& stages, 
        const asset::Mesh& mesh, 
        const uint32_t& sub_mesh_idx
      );
      void draw(const asset::Mesh& mesh, const uint32_t& sub_mesh_idx);

    private:
      void update(
        const asset::Mesh::Buffer& mesh_buffer, 
        D3D11RenderBuffer*& buffer, 
        bool is_vertex
      );
      void updateBuffer(
        const void* data, 
        UINT size, 
        D3D11RenderBuffer*& buffer, 
        bool is_vertex
      );
      void generateBuffer(
        const void* data, 
        UINT size, 
        D3D11RenderBuffer*& buffer, 
        bool is_vertex, 
        bool is_dynamic
      );

    private:
      D3D11Context* context_;
      Array<D3D11RenderBuffer*, asset::MeshElements::kCount> buffer_;
    };
  }
}