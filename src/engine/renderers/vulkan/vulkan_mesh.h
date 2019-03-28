#pragma once
#include "assets/mesh.h"
#include <memory/memory.h>
#include "vulkan.h"

namespace lambda
{
  namespace linux
  {
    class VulkanRenderer;
    class VulkanRenderBuffer;

    ///////////////////////////////////////////////////////////////////////////
    class VulkanMesh : public asset::IGPUAsset
    {
    public:
      VulkanMesh(asset::MeshHandle mesh, VulkanRenderer* renderer);
      virtual ~VulkanMesh() override;

      void bind(
        const Vector<uint32_t>& stages,
        const uint32_t& sub_mesh_idx
      );
      void draw(const uint32_t& sub_mesh_idx);

    private:
      void update(
        const asset::Mesh::Buffer& mesh_buffer, 
        VulkanRenderBuffer*& buffer, 
        bool is_vertex
      );
      void updateBuffer(
        const void* data, 
        uint32_t size, 
        VulkanRenderBuffer*& buffer, 
        bool is_vertex
      );
      void generateBuffer(
        const void* data, 
        uint32_t size, 
        VulkanRenderBuffer*& buffer, 
        bool is_vertex, 
        bool is_dynamic
      );

    private:
      VulkanRenderer* renderer_;
	  asset::MeshHandle mesh_;
	  UnorderedMap<uint32_t, VulkanRenderBuffer*> buffer_;
    };
  }
}