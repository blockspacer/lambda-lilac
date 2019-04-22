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
    class D3D11Mesh
    {
    public:
      D3D11Mesh(D3D11Context* context);
      ~D3D11Mesh();

      void bind(
        const Vector<uint32_t>& stages,
				asset::VioletMeshHandle mesh,
        const uint32_t& sub_mesh_idx
      );
      void draw(asset::VioletMeshHandle mesh, const uint32_t& sub_mesh_idx, const uint32_t& instance_count);

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
			UnorderedMap<uint32_t, D3D11RenderBuffer*> buffer_;
    };
  }
}