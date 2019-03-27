#include <gui/gpu_driver.h>
#include <utils/utilities.h>
#include <assets/asset_manager.h>
#include <interfaces/irenderer.h>
#include <interfaces/iworld.h>
#include <platform/shader_variable.h>
#include <platform/blend_state.h>
#include <platform/sampler_state.h>
#include <platform/render_target.h>

#include <Ultralight/platform/Platform.h>
#include <Ultralight/platform/Config.h>
#include <Ultralight/Renderer.h>

namespace lambda
{
	namespace gui
	{
		///////////////////////////////////////////////////////////////////////////
		MyGPUDriver::MyGPUDriver(world::IWorld* world)
			: world_(world)
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
			uint32_t      width = bitmap->width();
			uint32_t      height = bitmap->height();
			uint32_t      flags = kTextureFlagDynamicData;
			TextureFormat format = ultralight::kBitmapFormat_RGBA8 ?
				TextureFormat::kB8G8R8A8 : TextureFormat::kA8;

			Vector<char> data;

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
				(texture->getLayer(0).getFormat() == TextureFormat::kB8G8R8A8) ==
				(bitmap->format() == ultralight::kBitmapFormat_RGBA8))
			{

				Vector<char> data(bitmap->size());
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

				Vector<char> data = texture->getLayer(0).getData();

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
			world_->getRenderer()->setTexture(texture, slot);
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
			world_->getRenderer()->setRenderTargets(
			{ render_targets_[render_buffer_id] },
				asset::VioletTextureHandle()
			);
		}

		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::ClearRenderBuffer(uint32_t render_buffer_id)
		{
			world_->getRenderer()->clearRenderTarget(
				render_targets_[render_buffer_id],
				glm::vec4(0.0f)
			);
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

		struct VertexType01
		{
			float d[8];
		};
		struct VertexType02
		{
			float d[38];
		};

		void rebalance01(const ultralight::VertexBuffer& vertices, Vector<VertexType01>& output)
		{
			uint32_t size = vertices.size / sizeof(ultralight::Vertex_2f_4ub_2f);
			output.resize(size);
			for (uint32_t i = 0; i < size; ++i)
			{
				const ultralight::Vertex_2f_4ub_2f& vertex = ((ultralight::Vertex_2f_4ub_2f*)vertices.data)[i];
				output[i].d[0] = vertex.pos[0];
				output[i].d[1] = vertex.pos[1];
				output[i].d[2] = (float)vertex.color[0] / 255.0f;
				output[i].d[3] = (float)vertex.color[1] / 255.0f;
				output[i].d[4] = (float)vertex.color[2] / 255.0f;
				output[i].d[5] = (float)vertex.color[3] / 255.0f;
				output[i].d[6] = vertex.obj[0];
				output[i].d[7] = vertex.obj[1];
			}
		}

		void rebalance02(const ultralight::VertexBuffer& vertices, Vector<VertexType02>& output)
		{
			uint32_t size = vertices.size / sizeof(ultralight::Vertex_2f_4ub_2f_2f_28f);
			output.resize(size);
			for (uint32_t i = 0; i < size; ++i)
			{
				const ultralight::Vertex_2f_4ub_2f_2f_28f& vertex = ((ultralight::Vertex_2f_4ub_2f_2f_28f*)vertices.data)[i];
				output[i].d[0] = vertex.pos[0];
				output[i].d[1] = vertex.pos[1];
				output[i].d[2] = (float)vertex.color[0] / 255.0f;
				output[i].d[3] = (float)vertex.color[1] / 255.0f;
				output[i].d[4] = (float)vertex.color[2] / 255.0f;
				output[i].d[5] = (float)vertex.color[3] / 255.0f;
				output[i].d[6] = vertex.tex[0];
				output[i].d[7] = vertex.tex[1];
				output[i].d[8] = vertex.obj[0];
				output[i].d[9] = vertex.obj[1];
				output[i].d[10] = vertex.data0[0];
				output[i].d[11] = vertex.data0[1]; 
				output[i].d[12] = vertex.data0[2]; 
				output[i].d[13] = vertex.data0[3];
				output[i].d[14] = vertex.data1[0];
				output[i].d[15] = vertex.data1[1];
				output[i].d[16] = vertex.data1[2]; 
				output[i].d[17] = vertex.data1[3];
				output[i].d[18] = vertex.data2[0];
				output[i].d[19] = vertex.data2[1];
				output[i].d[20] = vertex.data2[2];
				output[i].d[21] = vertex.data2[3];
				output[i].d[22] = vertex.data3[0]; 
				output[i].d[23] = vertex.data3[1];
				output[i].d[24] = vertex.data3[2]; 
				output[i].d[25] = vertex.data3[3];
				output[i].d[26] = vertex.data4[0];
				output[i].d[27] = vertex.data4[1];
				output[i].d[28] = vertex.data4[2]; 
				output[i].d[29] = vertex.data4[3];
				output[i].d[30] = vertex.data5[0];
				output[i].d[31] = vertex.data5[1]; 
				output[i].d[32] = vertex.data5[2]; 
				output[i].d[33] = vertex.data5[3];
				output[i].d[34] = vertex.data6[0];
				output[i].d[35] = vertex.data6[1]; 
				output[i].d[36] = vertex.data6[2]; 
				output[i].d[37] = vertex.data6[3];
			}
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

			if (vertices.format == ultralight::kVertexBufferFormat_2f_4ub_2f)
			{
				Vector<VertexType01> vertex_data;
				rebalance01(vertices, vertex_data);
				mesh->set(asset::MeshElements::kPositions, vertex_data);
			}
			else
			{
				Vector<VertexType02> vertex_data;
				rebalance02(vertices, vertex_data);
				mesh->set(asset::MeshElements::kPositions, vertex_data);
			}

			Vector<uint32_t> index_data(indices.size / sizeof(uint32_t));
			memcpy(index_data.data(), indices.data, indices.size);
			mesh->set(
				asset::MeshElements::kIndices,
				index_data
			);
			asset::SubMesh sub_mesh;
			sub_mesh.offset(asset::MeshElements::kPositions).stride = (vertices.format == ultralight::kVertexBufferFormat_2f_4ub_2f) ? sizeof(VertexType01) : sizeof(VertexType02);
			sub_mesh.offset(asset::MeshElements::kPositions).count = vertices.size / sub_mesh.offset(asset::MeshElements::kPositions).stride;
			sub_mesh.offset(asset::MeshElements::kIndices).stride = sizeof(uint32_t);
			sub_mesh.offset(asset::MeshElements::kIndices).count = indices.size / sub_mesh.offset(asset::MeshElements::kIndices).stride;
			mesh->setSubMeshes({ sub_mesh });
		}

		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::UpdateGeometry(
			uint32_t geometry_id,
			const ultralight::VertexBuffer& vertices,
			const ultralight::IndexBuffer& indices)
		{
			asset::MeshHandle mesh = geometry_[geometry_id];

			if (vertices.format == ultralight::kVertexBufferFormat_2f_4ub_2f)
			{
				Vector<VertexType01> vertex_data;
				rebalance01(vertices, vertex_data);
				mesh->set(asset::MeshElements::kPositions, vertex_data);
			}
			else
			{
				Vector<VertexType02> vertex_data;
				rebalance02(vertices, vertex_data);
				mesh->set(asset::MeshElements::kPositions, vertex_data);
			}

			Vector<uint32_t> index_data(indices.size / sizeof(uint32_t));
			memcpy(index_data.data(), indices.data, indices.size);
			mesh->set(asset::MeshElements::kIndices, index_data);

			asset::SubMesh sub_mesh;
			sub_mesh.offset(asset::MeshElements::kPositions).stride = (vertices.format == ultralight::kVertexBufferFormat_2f_4ub_2f) ? sizeof(VertexType01) : sizeof(VertexType02);
			sub_mesh.offset(asset::MeshElements::kPositions).count = vertices.size / sub_mesh.offset(asset::MeshElements::kPositions).stride;
			sub_mesh.offset(asset::MeshElements::kIndices).stride = sizeof(uint32_t);
			sub_mesh.offset(asset::MeshElements::kIndices).count = indices.size / sub_mesh.offset(asset::MeshElements::kIndices).stride;
			mesh->setSubMeshes({ sub_mesh });
		}

		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::DrawGeometry(
			uint32_t geometry_id,
			uint32_t indices_count,
			uint32_t indices_offset,
			const ultralight::GPUState& state)
		{
			foundation::SharedPtr<platform::IRenderer> renderer = world_->getRenderer();

			renderer->setRenderTargets(
			{ render_targets_[state.render_buffer_id] },
				asset::VioletTextureHandle()
			);

			glm::vec4 viewport(
				0,
				0,
				state.viewport_width,
				state.viewport_height
			);
			renderer->setViewports({ viewport });
			renderer->setScissorRects({ viewport });

			for (int i = 0; i < 8; ++i)
				renderer->setTexture(asset::VioletTextureHandle(), i);

			if (state.texture_1_id)
				BindTexture(0, state.texture_1_id);
			if (state.texture_2_id)
				BindTexture(1, state.texture_2_id);
			if (state.texture_3_id)
				BindTexture(2, state.texture_3_id);
				
			BindShader(state);

			asset::SubMesh sub_mesh = geometry_[geometry_id]->getSubMeshes()[0];
			sub_mesh.offsets[asset::MeshElements::kIndices].count = indices_count;
			sub_mesh.index_offset = indices_offset;
			geometry_[geometry_id]->setSubMeshes({ sub_mesh });

			renderer->setSubMesh(0);
			renderer->setMesh(geometry_[geometry_id]);

			renderer->setBlendState(state.enable_blend ? platform::BlendState::Alpha() : platform::BlendState::Default());
			renderer->setSamplerState(platform::SamplerState::LinearClamp(), 0);

			renderer->draw();
			batch_count_++;
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
			return !command_list_.empty();
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
		
		glm::vec4 toVec4(const ultralight::vec4 v)
		{
			return glm::vec4(v.value[0], v.value[1], v.value[2], v.value[3]);
		}
		
		glm::mat4x4 toMat4(const ultralight::Matrix4x4 v)
		{
			return glm::mat4x4(
				v.data[0],  v.data[1],  v.data[2],  v.data[3],
				v.data[4],  v.data[5],  v.data[6],  v.data[7],
				v.data[8],  v.data[9],  v.data[10], v.data[11],
				v.data[12], v.data[13], v.data[14], v.data[15]
			);
		}

		///////////////////////////////////////////////////////////////////////////
		void MyGPUDriver::BindShader(const ultralight::GPUState& state)
		{
			if (!shader_fill_)
			{
				shader_fill_ = asset::ShaderManager::getInstance()->get(Name("resources/shaders/gui_fill.fx"));
				shader_fill_->setKeepInMemory(true);
			}
			if (!shader_fill_path_)
			{
				shader_fill_path_ = asset::ShaderManager::getInstance()->get(Name("resources/shaders/gui_fill_path.fx"));
				shader_fill_path_->setKeepInMemory(true);
			}

			asset::VioletShaderHandle shader = (state.shader_type == ultralight::kShaderType_Fill) ? shader_fill_ : shader_fill_path_;

			world_->getRenderer()->setShader(shader);

			float scale = 1.0f; // TODO (Hilze): Re-evaluate.
			world_->getShaderVariableManager().setVariable(platform::ShaderVariable(Name("State"),     glm::vec4(0.0, state.viewport_width, state.viewport_height, scale)));
			world_->getShaderVariableManager().setVariable(platform::ShaderVariable(Name("Transform"), toMat4(state.transform)));
			world_->getShaderVariableManager().setVariable(platform::ShaderVariable(Name("Scalar4_0"), glm::vec4(state.uniform_scalar[0], state.uniform_scalar[1], state.uniform_scalar[2], state.uniform_scalar[3])));
			world_->getShaderVariableManager().setVariable(platform::ShaderVariable(Name("Scalar4_1"), glm::vec4(state.uniform_scalar[4], state.uniform_scalar[5], state.uniform_scalar[6], state.uniform_scalar[7])));
			for (size_t i = 0; i < 8; ++i)
				world_->getShaderVariableManager().setVariable(platform::ShaderVariable(Name("Vector_" + toString(i)), toVec4(state.uniform_vector[i])));
			world_->getShaderVariableManager().setVariable(platform::ShaderVariable(Name("ClipSize"),  (float)state.clip_size));
			for (size_t i = 0; i < state.clip_size; ++i)
				world_->getShaderVariableManager().setVariable(platform::ShaderVariable(Name("Clip_" + toString(i)), toMat4(state.clip[i])));
		}

		///////////////////////////////////////////////////////////////////////////
		asset::VioletTextureHandle MyGPUDriver::GetRenderBuffer(
			uint32_t render_target_id)
		{
			return render_targets_[render_target_id];
		}
	}
}