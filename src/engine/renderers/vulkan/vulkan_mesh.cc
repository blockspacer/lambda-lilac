#include "vulkan_mesh.h"
#include "vulkan_renderer.h"

namespace lambda
{
	namespace linux
	{
		///////////////////////////////////////////////////////////////////////////
		VulkanMesh::VulkanMesh(VulkanRenderer* renderer)
			: renderer_(renderer)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		VulkanMesh::~VulkanMesh()
		{
			for (auto& it : buffer_)
				if (it.second != nullptr)
					renderer_->freeRenderBuffer((platform::IRenderBuffer*&)it.second);
		}

		///////////////////////////////////////////////////////////////////////////
		void VulkanMesh::bind(
			const Vector<uint32_t>& stages,
			asset::MeshHandle mesh,
			const uint32_t& sub_mesh_idx)
		{
			Vector<VkBuffer> buffers;
			Vector<VkDeviceSize> offsets;

			asset::SubMesh& sub_mesh = mesh->getSubMeshes().at(sub_mesh_idx);

			auto size = mesh->get(asset::MeshElements::kPositions).count;

			for (const auto& stage : stages)
			{
				VulkanRenderBuffer*& Vulkan_buffer = buffer_[stage];
				const asset::Mesh::Buffer& mesh_buffer = mesh->get(stage);
				if (Vulkan_buffer == nullptr || mesh->changed(stage))
					update(mesh_buffer, Vulkan_buffer, true);
				buffers.push_back(
					Vulkan_buffer ? Vulkan_buffer->getBuffer() : nullptr
				);
				offsets.push_back(sub_mesh.offsets[stage].offset);
			}

			vezCmdBindVertexBuffers(
				0, 
				(uint32_t)buffers.size(),
				buffers.data(), 
				offsets.data()
			);

			const asset::Mesh::Buffer& indices =
				mesh->get(asset::MeshElements::kIndices);
			if (indices.count > 0u)
			{
				if (buffer_[asset::MeshElements::kIndices] == nullptr ||
					mesh->changed(asset::MeshElements::kIndices))
					update(indices, buffer_.at(asset::MeshElements::kIndices), false);

				VkIndexType index_type = VK_INDEX_TYPE_UINT32;
				switch (indices.size)
				{
				case sizeof(uint16_t) :
					index_type = VK_INDEX_TYPE_UINT16;
					break;
				case sizeof(uint32_t) :
					index_type = VK_INDEX_TYPE_UINT32;
					break;
				default:
					foundation::Error("Mesh: Unknown index size: " +
						toString(indices.size) + "\n");
					break;
				}
				vezCmdBindIndexBuffer(
					buffer_.at(asset::MeshElements::kIndices)->getBuffer(), 
					sub_mesh.offsets[asset::MeshElements::kIndices].offset, 
					index_type
				);
			}
		}

		///////////////////////////////////////////////////////////////////////////
		void VulkanMesh::draw(asset::MeshHandle mesh, const uint32_t& sub_mesh_idx)
		{
			asset::SubMesh& sub_mesh = mesh->getSubMeshes().at(sub_mesh_idx);

			if (buffer_.find(asset::MeshElements::kIndices) != buffer_.end() &&
				buffer_.at(asset::MeshElements::kIndices)->getSize() > 0)
			{
				const asset::SubMesh::Offset& idx =
					sub_mesh.offsets[asset::MeshElements::kIndices];
				vezCmdDrawIndexed(
					(uint32_t)idx.count, 
					1, 
					(uint32_t)sub_mesh.index_offset, 
					(uint32_t)sub_mesh.vertex_offset, 
					0
				);
			}
			else
				vezCmdDraw(
					(uint32_t)sub_mesh.offsets[asset::MeshElements::kPositions].count,
					1,
					0,
					0
				);
		}

		///////////////////////////////////////////////////////////////////////////
		void VulkanMesh::update(
			const asset::Mesh::Buffer& mesh_buffer,
			VulkanRenderBuffer*& buffer,
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
		void VulkanMesh::updateBuffer(
			const void* data,
			uint32_t size,
			VulkanRenderBuffer*& buffer,
			bool is_vertex)
		{
			if (data == nullptr || size == 0u)
			{
				if (buffer != nullptr)
					renderer_->freeRenderBuffer((platform::IRenderBuffer*&)buffer);
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
		void VulkanMesh::generateBuffer(
			const void* data,
			uint32_t size,
			VulkanRenderBuffer*& buffer,
			bool is_vertex,
			bool is_dynamic)
		{
			if (buffer != nullptr)
				renderer_->freeRenderBuffer((platform::IRenderBuffer*&)buffer);
			if (data == nullptr || size == 0u)
				return;

			uint32_t flags = 0u;

			flags |= is_dynamic ?
				VulkanRenderBuffer::kFlagDynamic : VulkanRenderBuffer::kFlagImmutable;

			flags |= is_vertex ?
				VulkanRenderBuffer::kFlagVertex : VulkanRenderBuffer::kFlagIndex;

			buffer = (VulkanRenderBuffer*)renderer_->allocRenderBuffer(
				size,
				flags,
				(void*)data
			);
		};
	}
}