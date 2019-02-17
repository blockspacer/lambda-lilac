#include <gui/gpu_driver.h>
#include <utils/utilities.h>
#include <assets/asset_manager.h>
#include <interfaces/irenderer.h>
#include <interfaces/iworld.h>

#include <Ultralight/platform/Platform.h>
#include <Ultralight/platform/Config.h>
#include <Ultralight/Renderer.h>

namespace lambda
{
	namespace gui
	{
		///////////////////////////////////////////////////////////////////////////
		MyGPUDriver::MyGPUDriver()
		{
		}
		
		///////////////////////////////////////////////////////////////////////////
		MyGPUDriver::~MyGPUDriver()
		{
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::BeginSynchronize()
		{
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::EndSynchronize()
		{
		}
		
		///////////////////////////////////////////////////////////////////////////
		uint32_t MyGPUDriver::NextTextureId()
		{
			return next_texture_id_++;
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::CreateTexture(
			uint32_t texture_id, 
			ultralight::Ref<ultralight::Bitmap> bitmap)
		{
			Name name("GUI_TEX_" + toString(texture_id));
			uint32_t      width  = bitmap->width();
			uint32_t      height = bitmap->height();
			uint32_t      flags  = kTextureFlagDynamicData;
			TextureFormat format = ultralight::kBitmapFormat_RGBA8 ? 
				TextureFormat::kR8G8B8A8 : TextureFormat::kA8;

			Vector<uint8_t> data;
			
			if (bitmap->IsEmpty())
			{
				flags |= kTextureFlagIsRenderTarget;
			}
			else
			{
				data.resize(bitmap->size());
				memcpy(data.data(), bitmap->LockPixels(), bitmap->size());
				bitmap->UnlockPixels();
			}
			
			asset::VioletTextureHandle texture =
				asset::TextureManager::getInstance()->create(
				name,
				width,
				height,
				1, 
				format,
				flags,
				data
			);

			textures_.insert(eastl::make_pair(texture_id, texture));
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::UpdateTexture(
			uint32_t texture_id, 
			ultralight::Ref<ultralight::Bitmap> bitmap)
		{
			asset::VioletTextureHandle texture = textures_[texture_id];

			if (texture->getLayer(0).getWidth() == bitmap->width() &&
				(texture->getLayer(0).getFormat() == TextureFormat::kR8G8B8A8) == 
				(bitmap->format() == ultralight::kBitmapFormat_RGBA8))
			{

				uint32_t s = (uint32_t)((bitmap->size() + 
					(sizeof(uint32_t) - 1u)) / sizeof(uint32_t));
				Vector<uint32_t> data(s);
				memcpy(data.data(), bitmap->LockPixels(), bitmap->size());
				texture->getLayer(0).setData(data);
				bitmap->UnlockPixels();
			}
			else 
			{
				uint32_t bpp, bpr, bpl;
				calculateImageMemory(
					texture->getLayer(0).getFormat(),
					texture->getLayer(0).getWidth(),
					texture->getLayer(0).getHeight(),
					bpp, 
					bpr, 
					bpl
				);

				Vector<uint32_t> data = texture->getLayer(0).getData();

				ultralight::Ref<ultralight::Bitmap> mapped_bitmap = 
					ultralight::Bitmap::Create(
					bitmap->width(),
					bitmap->height(),
					ultralight::kBitmapFormat_RGBA8,
					bpr,
					data.data(),
					bpl,
					false
				);

				texture->getLayer(0).setData(data);

				ultralight::IntRect dest_rect = { 
					0, 
					0, 
					(int)bitmap->width(), 
					(int)bitmap->height() 
				};

				mapped_bitmap->DrawBitmap(dest_rect, dest_rect, bitmap, false);
			}
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::BindTexture(
			uint8_t texture_unit, 
			uint32_t texture_id)
		{
			uint8_t slot = texture_unit;
			asset::VioletTextureHandle texture = textures_[texture_id];
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::DestroyTexture(uint32_t texture_id)
		{
			asset::TextureManager::getInstance()->destroy(textures_[texture_id]);
			textures_.erase(texture_id);
		}
		
		///////////////////////////////////////////////////////////////////////////
		uint32_t MyGPUDriver::NextRenderBufferId()
		{
			return next_render_buffer_id_++;
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::CreateRenderBuffer(
			uint32_t render_buffer_id, 
			const ultralight::RenderBuffer& buffer)
		{
			if (render_buffer_id == 0)
				return;

			if (render_targets_.find(render_buffer_id) != render_targets_.end())
				return;

			auto tex_entry = textures_.find(buffer.texture_id);
			if (tex_entry == textures_.end()) 
				return;

			render_targets_.insert(
				eastl::make_pair(render_buffer_id, tex_entry->second)
			);
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::BindRenderBuffer(uint32_t render_buffer_id)
		{
			//world::IWorld* world_;
			//world_->getRenderer()->setRenderTarget();
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::ClearRenderBuffer(uint32_t render_buffer_id)
		{
			//world::IWorld* world_;
			//world_->getRenderer()->clearRenderTarget(render_targets_[render_buffer_id], glm::vec4(0.0f));
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::DestroyRenderBuffer(uint32_t render_buffer_id)
		{
			render_targets_.erase(render_buffer_id);
		}
		
		///////////////////////////////////////////////////////////////////////////
		uint32_t MyGPUDriver::NextGeometryId()
		{
			return next_geometry_id_++;
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::CreateGeometry(
			uint32_t geometry_id, 
			const ultralight::VertexBuffer& vertices, 
			const ultralight::IndexBuffer& indices)
		{
			asset::MeshHandle mesh = asset::AssetManager::getInstance().createAsset(
				Name("GUI_GEO_" + toString(geometry_id)), 
				foundation::Memory::constructShared<asset::Mesh>()
			);
			geometry_[geometry_id] = mesh;

			Vector<uint8_t> vertex_data(vertices.size);
			memcpy(vertex_data.data(), vertices.data, vertex_data.size());
			mesh->set(
				vertices.format == ultralight::kVertexBufferFormat_2f_4ub_2f ? 
				constexprHash("Type_01") : constexprHash("Type_02"),
				vertex_data
			);

			Vector<uint8_t> index_data(indices.size);
			memcpy(index_data.data(), indices.data, index_data.size());
			mesh->set(
				asset::MeshElements::kIndices,
				index_data
			);
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::UpdateGeometry(
			uint32_t geometry_id, 
			const ultralight::VertexBuffer& vertices, 
			const ultralight::IndexBuffer& indices)
		{
			asset::MeshHandle mesh = geometry_[geometry_id];

			Vector<uint8_t> vertex_data(vertices.size);
			memcpy(vertex_data.data(), vertices.data, vertex_data.size());
			mesh->set(
				vertices.format == ultralight::kVertexBufferFormat_2f_4ub_2f ?
				constexprHash("Type_01") : constexprHash("Type_02"),
				vertex_data
			);

			Vector<uint8_t> index_data(indices.size);
			memcpy(index_data.data(), indices.data, index_data.size());
			mesh->set(
				asset::MeshElements::kIndices,
				index_data
			);
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::DrawGeometry(
			uint32_t geometry_id, 
			uint32_t indices_count, 
			uint32_t indices_offset, 
			const ultralight::GPUState& state)
		{
			//world::IWorld* world_;
			//platform::IRenderer* renderer = world_->getRenderer();

			// BindRenderBuffer();

			// SetViewport();

			// if (1) BindTexture();
			// if (2) BindTexture();

			// UpdateConstantBuffer();

			// SetSamplers();

			// BindShader();

			// Draw();
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::DestroyGeometry(uint32_t geometry_id)
		{
			geometry_.erase(geometry_id);
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::UpdateCommandList(const ultralight::CommandList& list)
		{
			if (list.size) 
			{
				command_list_.resize(list.size);
				memcpy(
					&command_list_[0], 
					list.commands, 
					sizeof(ultralight::Command) * list.size
				);
			}
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyGPUDriver::HasCommandsPending()
		{
			return command_list_.empty();
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::DrawCommandList()
		{
			if (command_list_.empty())
				return;

			batch_count_ = 0;

			for (auto& cmd : command_list_) 
			{
				if (cmd.command_type == ultralight::kCommandType_DrawGeometry)
					DrawGeometry(
						cmd.geometry_id, 
						cmd.indices_count, 
						cmd.indices_offset, 
						cmd.gpu_state
					);
				else if 
					(cmd.command_type == ultralight::kCommandType_ClearRenderBuffer)
					ClearRenderBuffer(cmd.gpu_state.render_buffer_id);
			}

			command_list_.clear();
		}
	}
}