#include "d3d11_mesh.h"
#include <d3d11.h>
#include <utils/console.h>
#include <renderers/d3d11/d3d11_context.h>

namespace lambda
{
  namespace windows
  {
    ///////////////////////////////////////////////////////////////////////////
    D3D11Mesh::D3D11Mesh(D3D11Context* context) 
      : context_(context)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    D3D11Mesh::~D3D11Mesh()
    {
      for (auto& it : buffer_)
        if (it.second != nullptr)
          context_->freeRenderBuffer((platform::IRenderBuffer*&)it.second);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Mesh::bind(
      const Vector<uint32_t>& stages,
      asset::MeshHandle mesh,
      const uint32_t& sub_mesh_idx)
    {
      Vector<UINT> strides;
      Vector<ID3D11Buffer*> buffers;
      Vector<UINT> offsets;

      asset::SubMesh& sub_mesh = mesh->getSubMeshes().at(sub_mesh_idx);

      auto size = mesh->get(asset::MeshElements::kPositions).count;

			for(const auto& stage : stages)
      {
        D3D11RenderBuffer*& d3d11_buffer = buffer_[stage];
        const asset::Mesh::Buffer& mesh_buffer = mesh->get(stage);
        if (d3d11_buffer == nullptr || mesh->changed(stage))
          update(mesh_buffer, d3d11_buffer, true);
        buffers.push_back(
          d3d11_buffer ? d3d11_buffer->getBuffer() : nullptr
        );
        strides.push_back((UINT)sub_mesh.offsets[stage].stride);
        offsets.push_back((UINT)sub_mesh.offsets[stage].offset);
      }

      context_->getD3D11Context()->IASetVertexBuffers(
        0,
        (UINT)buffers.size(), 
        buffers.data(), 
        strides.data(), 
        offsets.data()
      );

      const asset::Mesh::Buffer& indices = 
        mesh->get(asset::MeshElements::kIndices);
      if (indices.count > 0u)
      {
        if (buffer_[asset::MeshElements::kIndices] == nullptr || 
          mesh->changed(asset::MeshElements::kIndices))
          update(indices, buffer_.at(asset::MeshElements::kIndices), false);
        
        DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
        switch (indices.size)
        {
        case sizeof(uint16_t) :
          format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
          break;
        case sizeof(uint32_t) :
          format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
          break;
        default:
          foundation::Error("Mesh: Unknown index size: " + 
            toString(indices.size) + "\n");
          break;
        }
        context_->getD3D11Context()->IASetIndexBuffer(
          buffer_.at(asset::MeshElements::kIndices)->getBuffer(),
          format,
          (UINT)sub_mesh.offsets[asset::MeshElements::kIndices].offset
        );
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Mesh::draw(asset::MeshHandle mesh, const uint32_t& sub_mesh_idx)
    {
      asset::SubMesh& sub_mesh = mesh->getSubMeshes().at(sub_mesh_idx);
     
      if (buffer_.at(asset::MeshElements::kIndices) && 
        buffer_.at(asset::MeshElements::kIndices)->getSize() > 0)
      {
        const asset::SubMesh::Offset& idx = 
          sub_mesh.offsets[asset::MeshElements::kIndices];
        context_->getD3D11Context()->DrawIndexed(
          (UINT)idx.count, 
          (UINT)sub_mesh.index_offset, 
          (INT)sub_mesh.vertex_offset
        );
      }
      else
        context_->getD3D11Context()->Draw(
          (UINT)sub_mesh.offsets[asset::MeshElements::kPositions].count,
          0
        );
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Mesh::update(
      const asset::Mesh::Buffer& mesh_buffer, 
      D3D11RenderBuffer*& buffer, 
      bool is_vertex)
    {
      if (buffer != nullptr && 
        mesh_buffer.size * mesh_buffer.count == buffer->getSize())
        updateBuffer(
          mesh_buffer.data, 
          mesh_buffer.size * mesh_buffer.count, 
          buffer, 
          is_vertex
        );
      else
        generateBuffer(
          mesh_buffer.data, 
          mesh_buffer.size * mesh_buffer.count, 
          buffer, 
          is_vertex, 
          false
        );
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Mesh::updateBuffer(
      const void* data, 
      UINT size, 
      D3D11RenderBuffer*& buffer, 
      bool is_vertex)
    {
      if (data == nullptr || size == 0u)
      {
        if (buffer != nullptr)
          context_->freeRenderBuffer((platform::IRenderBuffer*&)buffer);
        return;
      }
      
      if ((buffer->getFlags() & platform::IRenderBuffer::kFlagDynamic) == 0u)
        generateBuffer(data, size, buffer, is_vertex, true);
      else
      {
        void* buffer_data = buffer->lock();
        memcpy(buffer_data, data, size);
        buffer->unlock();
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Mesh::generateBuffer(
      const void* data, 
      UINT size,
      D3D11RenderBuffer*& buffer, 
      bool is_vertex, 
      bool is_dynamic)
    {
      if (buffer != nullptr)
        context_->freeRenderBuffer((platform::IRenderBuffer*&)buffer);
      if (data == nullptr || size == 0u)
        return;

      uint32_t flags = 0u;
      
      flags |= is_dynamic ? 
        D3D11RenderBuffer::kFlagDynamic  : D3D11RenderBuffer::kFlagImmutable;
      
      flags |= is_vertex  ? 
        D3D11RenderBuffer::kFlagVertex   : D3D11RenderBuffer::kFlagIndex;
      
      buffer = (D3D11RenderBuffer*)context_->allocRenderBuffer(
        size, 
        flags, 
        (void*)data
      );
    };
  }
}
