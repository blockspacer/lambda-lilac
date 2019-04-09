#pragma once
#include <assets/texture.h>
#include <assets/shader.h>
#include <assets/mesh.h>

#include <Ultralight/platform/GPUDriver.h>

namespace lambda
{
	namespace scene
	{
		struct Scene;
	}

  namespace gui
  {
		///////////////////////////////////////////////////////////////////////////
		///// MY GPU DRIVER ///////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////
		class MyGPUDriver
			: public ultralight::GPUDriver
		{
		public:
			MyGPUDriver(scene::Scene& scene);
			virtual ~MyGPUDriver();

			void setScene(scene::Scene& scene);

			virtual void BeginSynchronize() override;

			virtual void EndSynchronize() override;

			virtual uint32_t NextTextureId() override;

			virtual void CreateTexture(uint32_t texture_id,
				ultralight::Ref<ultralight::Bitmap> bitmap) override;

			virtual void UpdateTexture(uint32_t texture_id,
				ultralight::Ref<ultralight::Bitmap> bitmap) override;

			virtual void BindTexture(uint8_t texture_unit,
				uint32_t texture_id) override;

			virtual void DestroyTexture(uint32_t texture_id) override;

			virtual uint32_t NextRenderBufferId() override;

			virtual void CreateRenderBuffer(uint32_t render_buffer_id,
				const ultralight::RenderBuffer& buffer) override;

			virtual void BindRenderBuffer(uint32_t render_buffer_id) override;

			virtual void ClearRenderBuffer(uint32_t render_buffer_id) override;

			virtual void DestroyRenderBuffer(uint32_t render_buffer_id) override;

			virtual uint32_t NextGeometryId() override;

			virtual void CreateGeometry(uint32_t geometry_id,
				const ultralight::VertexBuffer& vertices,
				const ultralight::IndexBuffer& indices) override;

			virtual void UpdateGeometry(uint32_t geometry_id,
				const ultralight::VertexBuffer& vertices,
				const ultralight::IndexBuffer& indices) override;

			virtual void DrawGeometry(uint32_t geometry_id,
				uint32_t indices_count,
				uint32_t indices_offset,
				const ultralight::GPUState& state) override;

			virtual void DestroyGeometry(uint32_t geometry_id) override;
			virtual void UpdateCommandList(const ultralight::CommandList& list) override;
			virtual bool HasCommandsPending() override;
			virtual void DrawCommandList() override;

			void BindShader(const ultralight::GPUState& state);

			asset::VioletTextureHandle GetRenderBuffer(uint32_t render_target_id);

		private:
			scene::Scene* scene_;

			UnorderedMap<uint32_t, asset::VioletTextureHandle> textures_;
			UnorderedMap<uint32_t, asset::VioletTextureHandle> render_targets_;
			UnorderedMap<uint32_t, asset::VioletMeshHandle>          geometry_;
			asset::VioletShaderHandle                          shader_;

			uint32_t next_texture_id_       = 1;
			uint32_t next_render_buffer_id_ = 1;
			uint32_t next_geometry_id_      = 1;

			Vector<ultralight::Command> command_list_;
			int batch_count_;

			asset::VioletShaderHandle shader_fill_;
			asset::VioletShaderHandle shader_fill_path_;
		};
  }
}
