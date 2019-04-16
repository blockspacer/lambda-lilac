#pragma once
#include "interfaces/irenderer.h"
#include "windows/win32/win32_window.h"
#include <d3d11.h>
#include <d3d11_1.h>
#include <wrl/client.h>
#include "d3d11_state_manager.h"

struct ID3D11SamplerState;
struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11BlendState;
struct ID3DUserDefinedAnnotation;

#define GPU_MARKERS 1
#define GPU_TIMERS 0

#define MAX_TEXTURE_COUNT 16u
#define MAX_CONSTANT_BUFFER_COUNT 16u

namespace lambda
{
	namespace windows
	{
		class D3D11Mesh;
		class D3D11Shader;
		class D3D11Texture;

		///////////////////////////////////////////////////////////////////////////
		struct D3D11XContext
		{
			Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;
			Microsoft::WRL::ComPtr<ID3D11Device> device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backbuffer;
			bool vsync;
		};

		///////////////////////////////////////////////////////////////////////////
		struct D3D11Default
		{
			glm::vec4 viewport;
		};

		///////////////////////////////////////////////////////////////////////////
		struct D3D11FullScreenQuad
		{
			asset::VioletMeshHandle mesh;
			asset::VioletShaderHandle shader;
		};

#if FLUSH_METHOD
		struct IRenderAction;
#endif

		///////////////////////////////////////////////////////////////////////////
		class D3D11RenderBuffer : public platform::IRenderBuffer
		{
		public:
			D3D11RenderBuffer(
				uint32_t size,
				uint32_t flags,
				ID3D11Buffer* buffer,
				ID3D11DeviceContext* context
			);
			virtual void*    lock()   override;
			virtual void     unlock() override;
			virtual uint32_t getFlags()  const override;
			virtual uint32_t getSize()   const override;
			ID3D11Buffer*    getBuffer() const;

		private:
			ID3D11DeviceContext* context_;
			ID3D11Buffer* buffer_;
			uint32_t flags_;
			uint32_t size_;
		};

		///////////////////////////////////////////////////////////////////////////
		class D3D11RenderTexture : public platform::IRenderTexture
		{
		public:
			D3D11RenderTexture(
				uint32_t width,
				uint32_t height,
				uint32_t depth,
				uint32_t mip_count,
				TextureFormat format,
				uint32_t flags,
				D3D11Texture* texture,
				ID3D11DeviceContext* context
			);
			virtual void*    lock(uint32_t level)   override;
			virtual void     unlock(uint32_t level) override;
			virtual uint32_t getWidth()    const override;
			virtual uint32_t getHeight()   const override;
			virtual uint32_t getDepth()    const override;
			virtual uint32_t getMipCount() const override;
			virtual uint32_t getFlags()    const override;
			virtual TextureFormat getFormat() const override;
			D3D11Texture* getTexture() const;

		private:
			ID3D11DeviceContext* context_;
			D3D11Texture* texture_;
			uint32_t flags_;
			uint32_t width_;
			uint32_t height_;
			uint32_t depth_;
			uint32_t mip_count_;
			TextureFormat format_;
		};

		///////////////////////////////////////////////////////////////////////////
		struct D3D11TimestampQuery
		{
			Microsoft::WRL::ComPtr<ID3D11Query> push[2u];
			Microsoft::WRL::ComPtr<ID3D11Query> pop[2u];
			uint64_t micro_seconds;
		};

		///////////////////////////////////////////////////////////////////////////
		class D3D11Context : public platform::IRenderer
		{
		private:
			struct MemoryStats
			{
				uint32_t vertex;
				uint32_t index;
				uint32_t constant;
				uint32_t texture;
				uint32_t render_target;
			} memory_stats_;
#define TO_MB(bytes) (std::round(bytes / 10000) / 100)

		public:
			virtual platform::IRenderBuffer* allocRenderBuffer(
				uint32_t size,
				uint32_t flags,
				void* data = nullptr
			) override;
			virtual void freeRenderBuffer(platform::IRenderBuffer*& buffer) override;
			platform::IRenderTexture* allocRenderTexture(
				asset::VioletTextureHandle texture
			);
			void freeRenderTexture(platform::IRenderTexture*& texture);

			ID3D11DeviceContext* getD3D11Context() const;
			ID3D11Device* getD3D11Device() const;

			virtual ~D3D11Context();
			virtual void setWindow(platform::IWindow* window) override;
			virtual void setOverrideScene(scene::Scene* scene) override;
			virtual void initialize(scene::Scene& scene) override;
			virtual void deinitialize() override;
			virtual void resize() override;
			virtual void update(const double& delta_time) override;
			virtual void startFrame() override;
			virtual void endFrame(bool display) override;

			/////////////////////////////////////////////////////////////////////////
			///// Deferred Calls ////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////
			virtual void draw() override;
			virtual void drawInstanced(const Vector<glm::mat4>& matrices) override;

			virtual void setRasterizerState(
				const platform::RasterizerState& rasterizer_state
			) override;
			virtual void setBlendState(
				const platform::BlendState& blend_state
			) override;
			virtual void setDepthStencilState(
				const platform::DepthStencilState& depth_stencil_state
			) override;
			virtual void setSamplerState(
				const platform::SamplerState& sampler_state,
				unsigned char slot
			) override;

			virtual void generateMipMaps(
				const asset::VioletTextureHandle& texture
			) override;
			virtual void copyToScreen(
				const asset::VioletTextureHandle& texture
			) override;
			virtual void copyToTexture(
				const asset::VioletTextureHandle& src,
				const asset::VioletTextureHandle& dst
			) override;
			virtual void bindShaderPass(
				const platform::ShaderPass& shader_pass
			) override;
			virtual void clearRenderTarget(
				asset::VioletTextureHandle texture,
				const glm::vec4& colour
			) override;

			virtual void setScissorRects(const Vector<glm::vec4>& rects) override;
			virtual void setViewports(const Vector<glm::vec4>& rects) override;

			virtual void setMesh(asset::VioletMeshHandle mesh) override;
			virtual void setSubMesh(const uint32_t& sub_mesh_idx) override;
			virtual void setShader(asset::VioletShaderHandle shader) override;
			virtual void setTexture(
				asset::VioletTextureHandle texture,
				uint8_t slot
			) override;
			virtual void setConstantBuffer(
				platform::IRenderBuffer* constant_buffer,
				uint8_t slot = 0
			) override;
			virtual void setRenderTargets(
				Vector<asset::VioletTextureHandle> render_targets,
				asset::VioletTextureHandle depth_buffer
			) override;
			void setRenderTargets(
				Vector<ID3D11RenderTargetView*> render_targets,
				ID3D11DepthStencilView* depth_buffer
			);

			virtual void pushMarker(const String& name) override;
			virtual void setMarker(const String& name) override;
			virtual void popMarker() override;

			virtual void  beginTimer(const String& name) override;
			virtual void  endTimer(const String& name) override;
			virtual uint64_t getTimerMicroSeconds(const String& name) override;
			/////////////////////////////////////////////////////////////////////////
			///// Deferred Calls End ////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////

			virtual void setRenderScale(const float& render_scale) override;
			virtual float getRenderScale() override;

			virtual void setVSync(bool vsync) override;
			virtual bool getVSync() const override;

			virtual void destroyTexture(const size_t& hash) override;
			virtual void destroyShader(const size_t& hash) override;
			virtual void destroyMesh(const size_t& hash) override;

#if FLUSH_METHOD
			void flush(Vector<IRenderAction*> actions);
#endif
			void present();

		protected:
			ID3D11RenderTargetView* getRTV(
				asset::VioletTextureHandle texture,
				int idx = -1,
				int layer = 0,
				int mip_map = 0
			);
			ID3D11DepthStencilView* getDSV(
				asset::VioletTextureHandle texture,
				int idx = -1,
				int layer = 0,
				int mip_map = 0
			);

		protected:
			void draw(ID3D11Buffer* buffer);
			scene::Scene* getScene() const;
			void resizeImpl();

		private:
			bool queued_update_ = false;
			float queued_delta_time_ = 0.0f;
			bool queued_resize_ = false;
			scene::Scene* scene_;
			scene::Scene* override_scene_;
			float render_scale_ = 1.0f;

			D3D11FullScreenQuad full_screen_quad_;
			D3D11StateManager state_manager_;

			D3D11XContext context_;
			D3D11Default default_;

			glm::vec2 screen_size_;
			float delta_time_ = 0.0f;
			float total_time_ = 0.0f;
			float dynamic_resolution_scale_ = 1.0f;
			Vector<D3D11RenderBuffer*> transient_render_buffers_;

			struct State
			{
				uint8_t                    num_scissor_rects;
				glm::vec4                  scissor_rects[8u];
				uint8_t                    num_viewports;
				glm::vec4                  viewports[8u];
				uint8_t                    num_render_targets;
				asset::VioletTextureHandle render_targets[8u];
				asset::VioletTextureHandle depth_target;
				asset::VioletTextureHandle textures[MAX_TEXTURE_COUNT];
				uint16_t                   dirty_textures;
				asset::VioletMeshHandle    mesh;
				uint32_t                   sub_mesh;
				asset::VioletShaderHandle  shader;
				platform::IRenderBuffer*   constant_buffers[MAX_CONSTANT_BUFFER_COUNT];
				uint16_t                   dirty_constant_buffers;
			} state_;

			struct DXState
			{
				D3D11_RECT                scissor_rects[8u];
				D3D11_VIEWPORT            viewports[8u];
				ID3D11RenderTargetView*   render_targets[8u];
				ID3D11DepthStencilView*   depth_target;
				ID3D11ShaderResourceView* textures[MAX_TEXTURE_COUNT];
				D3D11Shader*              shader;
				D3D11Mesh*                mesh;
				ID3D11Buffer*             constant_buffers[MAX_CONSTANT_BUFFER_COUNT];
			} dx_state_;

			enum class DirtyStates : uint32_t
			{
				kViewports = 1ull << 1ull,
				kScissorRects = 1ull << 2ull,
				kRenderTargets = 1ull << 3ull,
				kTextures = 1ull << 4ull,
				kMesh = 1ull << 5ull,
				kShader = 1ull << 6ull,
				kConstantBuffers = 1ull << 7ull,
			};

			uint32_t dirty_state_;
			bool isDirty(DirtyStates state) { return (dirty_state_ & ((uint32_t)state)) != 0u; }
			void makeDirty(DirtyStates state) { dirty_state_ |= ((uint32_t)state); }
			void cleanDirty(DirtyStates state) { dirty_state_ &= ~((uint32_t)state); }
			void invalidateAll() { dirty_state_ = ~0ul; }
			void cleanAll() { dirty_state_ = 0ul; }

			struct ConstantBuffers
			{
				D3D11RenderBuffer* per_frame   = nullptr;
				D3D11RenderBuffer* per_texture = nullptr;
				D3D11RenderBuffer* drs         = nullptr;
			} cbs_;

#if GPU_MARKERS
			Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation>
				user_defined_annotation_;
#endif

#if GPU_TIMERS
			UnorderedMap<String, D3D11TimestampQuery> gpu_timers_;
			Microsoft::WRL::ComPtr<ID3D11Query> timer_disjoint_;
			uint8_t gpu_timer_idx;
#endif

			// TODO (Hilze): Do something with this.
			class D3D11AssetManager
			{
			public:
				~D3D11AssetManager();
				D3D11RenderTexture* getTexture(asset::VioletTextureHandle texture);
				void removeTexture(asset::VioletTextureHandle texture);
				void removeTexture(size_t texture);
				D3D11Mesh* getMesh(asset::VioletMeshHandle mesh);
				void removeMesh(asset::VioletMeshHandle mesh);
				void removeMesh(size_t mesh);
				D3D11Shader* getShader(asset::VioletShaderHandle shader);
				void removeShader(asset::VioletShaderHandle shader);
				void removeShader(size_t shader);
				void setD3D11Context(D3D11Context* context);
				void setDevice(ID3D11Device* device);
				void setContext(ID3D11DeviceContext* context);
				void deleteNotReffedAssets(const float& dt);
				void deleteAllAssets();

			private:
				template<typename T>
				struct RefType
				{
					T* t;
					uint32_t ref;
					float time;
					bool keep_in_memory;
				};
				static constexpr float kMaxTime = 2.0f;
				UnorderedMap<uint64_t, RefType<D3D11RenderTexture>> textures_;
				UnorderedMap<uint64_t, RefType<D3D11Mesh>> meshes_;
				UnorderedMap<uint64_t, RefType<D3D11Shader>> shaders_;
				D3D11Context* d3d11_context_;
				ID3D11Device* device_;
				ID3D11DeviceContext* context_;
			} asset_manager_;
		};
	}
}