#pragma once
#include "interfaces/irenderer.h"

#if VIOLET_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.hpp>
#include <VEZ.h>

namespace lambda
{
  namespace linux
  {
	  class VulkanTexture;
	  class VulkanRenderer;

	  extern const char* vkErrorCode(const VkResult& result);

	  ///////////////////////////////////////////////////////////////////////////
	  class VulkanRenderBuffer : public platform::IRenderBuffer
	  {
	  public:
		  VulkanRenderBuffer(
			  uint32_t size,
			  uint32_t flags,
			  VkBuffer buffer,
			  VulkanRenderer* renderer
		  );
		  virtual void*    lock()   override;
		  virtual void     unlock() override;
		  virtual uint32_t getFlags()  const override;
		  virtual uint32_t getSize()   const override;
		  VkBuffer         getBuffer() const;

	  private:
		  void* data_;
		  VulkanRenderer* renderer_;
		  VkBuffer buffer_;
		  uint32_t flags_;
		  uint32_t size_;
	  };

	  ///////////////////////////////////////////////////////////////////////////
	  class VulkanRenderTexture : public platform::IRenderTexture
	  {
	  public:
		  VulkanRenderTexture(
			  uint32_t width,
			  uint32_t height,
			  uint32_t depth,
			  uint32_t mip_count,
			  TextureFormat format,
			  uint32_t flags,
			  VulkanTexture* texture,
			  VulkanRenderer* renderer
		  );
		  virtual void*    lock(uint32_t level)   override;
		  virtual void     unlock(uint32_t level) override;
		  virtual uint32_t getWidth()    const override;
		  virtual uint32_t getHeight()   const override;
		  virtual uint32_t getDepth()    const override;
		  virtual uint32_t getMipCount() const override;
		  virtual uint32_t getFlags()    const override;
		  virtual TextureFormat getFormat() const override;
		  VulkanTexture* getTexture() const;

	  private:
		  void* data_;
		  VulkanRenderer* renderer_;
		  VulkanTexture* texture_;
		  uint32_t flags_;
		  uint32_t width_;
		  uint32_t height_;
		  uint32_t depth_;
		  uint32_t mip_count_;
		  TextureFormat format_;
	  };

    ///////////////////////////////////////////////////////////////////////////
    class VulkanRenderer : public platform::IRenderer
    {
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

      VulkanRenderer();
      virtual ~VulkanRenderer();
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

      virtual void setScissorRect(const glm::vec4& rect) override;
      virtual void setViewports(const Vector<glm::vec4>& rects);

      virtual void setMesh(asset::MeshHandle mesh) override;
      virtual void setSubMesh(const uint32_t& sub_mesh_idx) override;
      virtual void setShader(asset::VioletShaderHandle shader) override;
      virtual void setTexture(
        asset::VioletTextureHandle texture, 
        uint8_t slot = 0
      ) override;
	  virtual void setRenderTargets(
        Vector<asset::VioletTextureHandle> render_targets,
        asset::VioletTextureHandle depth_buffer
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

	  VkDevice getDevice() const;
	  VkCommandBuffer getCommandBuffer() const;

    protected:
      virtual void destroyAsset(
        foundation::SharedPointer<asset::IAsset> asset
      ) override;

	private:
		void createFrameBuffer(platform::IWindow* window);
		void createCommandBuffer();

	private:
	  bool vsync_;
	  world::IWorld* world_;

	  VkInstance instance_;
	  VkPhysicalDevice physical_device_;
	  VkSurfaceKHR surface_;
	  VkDevice device_;
	  VezSwapchain swapchain_;

	  VkQueue graphics_queue_;
	  VkCommandBuffer command_buffer_;

	  struct FrameBuffer
	  {
		  VkImage        color_image              = VK_NULL_HANDLE;
		  VkImageView    color_image_view         = VK_NULL_HANDLE;
		  VkImage        depth_stencil_image      = VK_NULL_HANDLE;
		  VkImageView    depth_stencil_image_view = VK_NULL_HANDLE;
		  VezFramebuffer handle                   = VK_NULL_HANDLE;
	  } frame_buffer_;

	  struct Test
	  {
		  struct Pipeline
		  {
			  VezPipeline pipeline = VK_NULL_HANDLE;
			  std::vector<VkShaderModule> shader_modules;
		  } pipeline;

		  VkBuffer vertex_buffer = VK_NULL_HANDLE;
		  VkBuffer index_buffer = VK_NULL_HANDLE;
		  VkImage image = VK_NULL_HANDLE;
		  VkImageView image_view = VK_NULL_HANDLE;
		  VkSampler sampler = VK_NULL_HANDLE;
		  VkBuffer uniform_buffer = VK_NULL_HANDLE;
	  } test_;
    };
  }
}