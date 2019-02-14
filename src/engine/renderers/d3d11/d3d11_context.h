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

#define GPU_MARKERS 0
#define GPU_TIMERS 0

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
      asset::MeshHandle mesh;
      asset::VioletShaderHandle shader;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct D3D11Bound
    {
      ID3D11ShaderResourceView* textures[8u];
      D3D11Mesh* mesh;
      D3D11Shader* shader;
      uint32_t sub_mesh_idx;
    };

    struct IRenderAction;

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
      platform::IRenderBuffer* allocRenderBuffer(
        uint32_t size, 
        uint32_t flags, 
        void* data = nullptr
      );
      void freeRenderBuffer(platform::IRenderBuffer*& buffer);
			platform::IRenderTexture* allocRenderTexture(
				asset::VioletTextureHandle texture
			);
			void freeRenderTexture(platform::IRenderTexture*& texture);

      ID3D11DeviceContext* getD3D11Context() const;
      ID3D11Device* getD3D11Device() const;

      virtual ~D3D11Context();
      virtual void setWindow(
        foundation::SharedPointer<platform::IWindow> window
      ) override;
			virtual void initialize(world::IWorld* world) override;
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
      virtual void bindShaderPass(
        const platform::ShaderPass& shader_pass
      ) override;
      virtual void clearRenderTarget(
        asset::VioletTextureHandle texture, 
        const glm::vec4& colour
      ) override;

      virtual void setScissorRect(const glm::vec4& rect) override;
      virtual void setViewports(const Vector<glm::vec4>& rects);

      virtual void setMesh(asset::MeshHandle mesh) override;
      virtual void setSubMesh(const uint32_t& sub_mesh_idx) override;
      virtual void setShader(asset::VioletShaderHandle shader) override;
      virtual void setTexture(
        asset::VioletTextureHandle texture, 
        uint8_t slot
      ) override;

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

      void setShaderVariable(
        const platform::ShaderVariable& variable
      ) override;
      virtual void destroyAsset(
        foundation::SharedPointer<asset::IAsset> asset
      ) override;

      void flush(Vector<IRenderAction*> actions);
      void present();

    protected:
      ID3D11RenderTargetView* getRenderTargetView(
        asset::VioletTextureHandle texture, 
        int idx = -1, 
        int mip_map = 0
      );
      ID3D11DepthStencilView* getDepthStencilView(
        asset::VioletTextureHandle texture, 
        int idx = -1
      );

    protected:
      void draw(ID3D11Buffer* buffer);

    private:
      world::IWorld* world_;
      float render_scale_ = 1.0f;
      
      D3D11FullScreenQuad full_screen_quad_;
      D3D11StateManager state_manager_;
      
      D3D11XContext context_;
      D3D11Default default_;
      D3D11Bound   bound_;

      glm::mat4x4 model_;
      glm::vec2 screen_size_;
      glm::vec2 metallic_roughness_;
      double delta_time_ = 0.0f;
      double total_time_ = 0.0f;

      asset::MeshHandle mesh_;
      uint32_t sub_mesh_idx_;
      asset::VioletShaderHandle shader_;
      unsigned char highest_bound_texture_;
      static const unsigned char MAX_TEXTURE_COUNT = 16u;
      ID3D11ShaderResourceView* textures_[MAX_TEXTURE_COUNT];
      bool texture_contains_alpha_[MAX_TEXTURE_COUNT];

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
				D3D11Mesh* getMesh(asset::MeshHandle mesh);
				void removeMesh(asset::MeshHandle mesh);
				void removeMesh(size_t mesh);
				D3D11Shader* getShader(asset::VioletShaderHandle shader);
				void removeShader(asset::VioletShaderHandle shader);
				void removeShader(size_t shader);
        void setD3D11Context(D3D11Context* context);
        void setDevice(ID3D11Device* device);
        void setContext(ID3D11DeviceContext* context);
        void deleteNotReffedAssets(const float& dt);

      private:
        template<typename T>
        struct RefType
        {
          T* t;
          uint32_t ref;
          float time;
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