#pragma once
#include "interfaces/irenderer.h"
#include "vulkan.h"
#include "vulkan_state_manager.h"

namespace lambda
{
	namespace linux
	{
		///////////////////////////////////////////////////////////////////////////
		struct Framebuffer
		{
			uint32_t rt_width;
			uint32_t rt_height;
			uint32_t num_framebuffers;
			VkImageView render_targets[9];

			bool operator==(const Framebuffer& other) const
			{
				if (num_framebuffers != other.num_framebuffers || rt_width != other.rt_width || rt_height != other.rt_height)
					return false;

				for (uint32_t i = 0; i < num_framebuffers; ++i)
					if (render_targets[i] != other.render_targets[i])
						return false;

				return true;
			}
		};
	}
}

#include <containers/containers.h>

namespace eastl
{
	/////////////////////////////////////////////////////////////////////////////
	template <>
	struct hash<lambda::linux::Framebuffer>
	{
		std::size_t operator()(const lambda::linux::Framebuffer& k) const
		{
			size_t hash = 0ull;
			lambda::hashCombine(hash, (size_t)k.rt_width);
			lambda::hashCombine(hash, (size_t)k.rt_height);
			for (uint32_t i = 0; i < k.num_framebuffers; ++i)
				lambda::hashCombine(hash, (size_t)k.render_targets[i]);
			return hash;
		}
	};
}

#define VIOLET_USE_GPU_MARKERS 0
#define MAX_CONSTANT_BUFFER_COUNT 16u

namespace lambda
{
  namespace linux
  {
	  class VulkanTexture;
	  class VulkanShader;
	  class VulkanRenderer;
	  class VulkanMesh;

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
		  virtual uint32_t getFlags()   const override;
		  virtual uint32_t getSize()    const override;
		  size_t           getGPUSize() const;
		  VkBuffer         getBuffer()  const;

	  private:
		  void* data_;
		  VulkanRenderer* renderer_;
		  VkBuffer buffer_;
		  uint32_t flags_;
		  uint32_t size_;
		  size_t   gpu_size_;
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

	  struct VulkanCommandBuffer
	  {
	  public:
		  VulkanCommandBuffer();
		  void create(VkDevice device, VkQueue graphics_queue);
		  void destroy(VkDevice device);
		  void begin();
		  void end();
		  bool tryBegin();
		  bool tryEnd();
		  bool isRecording() const;
		  VkCommandBuffer getActiveCommandBuffer() const;
		  Vector<VkCommandBuffer> getCommandBuffers() const;
		  void reset();

	  private:
		  VkDevice device;
		  VkQueue graphics_queue;
		  Vector<VkCommandBuffer> command_buffers;
		  int32_t current_command_buffer;
		  bool recording;
	  };

	  struct VulkanFramebuffer
	  {
		  Framebuffer    alloc;
		  VezFramebuffer framebuffer;
		  Vector<VezAttachmentReference> attachment_references;
	  };

    ///////////////////////////////////////////////////////////////////////////
    class VulkanRenderer : public platform::IRenderer
    {
      struct MemoryStats
      {
        size_t vertex;
        size_t index;
        size_t constant;
        size_t texture;
        size_t render_target;
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

      VulkanRenderer();
      virtual ~VulkanRenderer();
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
        uint8_t slot = 0
      ) override;
	  virtual void setConstantBuffer(
		  platform::IRenderBuffer* constant_buffer,
		  uint8_t slot = 0
	  ) override;
	  virtual void setUserData(glm::vec4 user_data, uint8_t slot = 0) override;
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

	  VkDevice getDevice() const;
	  VkCommandBuffer getCommandBuffer() const;

    protected:
	  virtual void destroyTexture(const size_t& hash) override;
	  virtual void destroyShader(const size_t& hash) override;
	  virtual void destroyMesh(const size_t& hash) override;

	private:
		void createCommandBuffer();

		void copy(
			VkImage src,
			glm::uvec2 src_size,
			VkImage dst,
			glm::uvec2 dst_size
		);

		scene::Scene* getScene() const;

	private:
	  bool vsync_;
	  scene::Scene* scene_;
	  scene::Scene* override_scene_;

	  VkInstance instance_;
	  VkPhysicalDevice physical_device_;
	  VkSurfaceKHR surface_;
	  VkDevice device_;
	  VezSwapchain swapchain_;

	  VkQueue graphics_queue_;
	  VulkanCommandBuffer command_buffer_;

	  enum class DirtyState : uint32_t
	  {
		  kTextures        = 1ul << 1ul,
		  kShader          = 1ul << 2ul,
		  kMesh            = 1ul << 3ul,
		  kRenderTargets   = 1ul << 4ul,
		  kScissorRects    = 1ul << 5ul,
		  kViewports       = 1ul << 6ul,
		  kConstantBuffers = 1ul << 7ul,
	  };

	  uint32_t dirty_state_;
	  bool isDirty(DirtyState dirty_state) const { return (dirty_state_ & ((uint32_t)dirty_state)) != 0; }
	  void makeDirty(DirtyState dirty_state) { dirty_state_ |= (uint32_t)dirty_state; }
	  void cleanDirty(DirtyState dirty_state) { dirty_state_ &= ~((uint32_t)dirty_state); }
	  void invalidateAll() { dirty_state_ = ~0ul; }

	  struct State
	  {
		  asset::VioletTextureHandle textures[16];
		  asset::VioletShaderHandle shader;
		  asset::VioletMeshHandle mesh;
		  uint32_t sub_mesh;

		  asset::VioletTextureHandle render_targets[8];
		  asset::VioletTextureHandle depth_target;
		  uint32_t rt_width;
		  uint32_t rt_height;

		  glm::vec4 scissor_rects[8];
		  glm::vec4 viewports[8];
		  uint32_t num_scissor_rects;
		  uint32_t num_viewports;

		  platform::IRenderBuffer* constant_buffers[MAX_CONSTANT_BUFFER_COUNT];
		  uint16_t                 dirty_constant_buffers[(int)ShaderStages::kCount];
	  } state_;

	  struct VkState
	  {
		  VkImageView textures[16];
		  VulkanShader* shader;
		  VulkanMesh* mesh;
		  VkImageView render_targets[8];
		  VkImageView depth_target;

		  VulkanFramebuffer* framebuffer;

		  VkRect2D scissor_rects[8];
		  VkViewport viewports[8];

		  VkBuffer constant_buffers[MAX_CONSTANT_BUFFER_COUNT];
	  } vk_state_;

	  struct ConstantBuffers
	  {
		  VulkanRenderBuffer* per_frame    = nullptr;
		  VulkanRenderBuffer* per_texture  = nullptr;
		  VulkanRenderBuffer* drs          = nullptr;
		  VulkanRenderBuffer* cb_user_data = nullptr;
		  glm::vec4 user_data[16];
	  } cbs_;

	  float delta_time_;
	  float total_time_;
	  glm::vec3 screen_size_;
	  float  render_scale_;
	  float  dynamic_resolution_scale_ = 1.0f;

	  VkImage backbuffer_;
	  uint32_t backbuffer_width_;
	  uint32_t backbuffer_height_;
	  VkImageView backbuffer_view_;

	  asset::VioletTextureHandle default_texture_;

	  struct Memory
	  {
		  template<typename T>
		  struct Entry
		  {
			  T ptr = nullptr;
			  uint32_t hit_count = 0u;
			  bool keep_in_memory = false;
		  };

		  VulkanShader* getShader(asset::VioletShaderHandle handle);
		  VulkanRenderTexture* getTexture(asset::VioletTextureHandle handle);
		  VulkanMesh* getMesh(asset::VioletMeshHandle handle);
		  VulkanFramebuffer* getFramebuffer(const Framebuffer& framebuffer);
		  void removeShader(asset::VioletShaderHandle handle);
		  void removeTexture(asset::VioletTextureHandle handle);
		  void removeMesh(asset::VioletMeshHandle handle);
		  void removeFramebuffer(const Framebuffer& framebuffer);
		  void removeShader(size_t handle);
		  void removeTexture(size_t handle);
		  void removeMesh(size_t handle);

		  void removeUnusedEntries();
		  void removeAllEntries();
		  UnorderedMap<size_t, Entry<VulkanShader*>> shaders;
		  UnorderedMap<size_t, Entry<VulkanRenderTexture*>> textures;
		  UnorderedMap<size_t, Entry<VulkanMesh*>> meshes;
		  UnorderedMap<Framebuffer, Entry<VulkanFramebuffer*>> framebuffers;
		  VulkanRenderer* renderer;
	  } memory_;

	  VulkanStateManager state_manager_;
	  struct VulkanFullScreenQuad
	  {
		  asset::VioletMeshHandle mesh;
		  asset::VioletShaderHandle shader;
	  } full_screen_quad_;

#if VIOLET_DEBUG
	  VkDebugUtilsMessengerEXT debug_messenger_;
#endif
#if VIOLET_USE_GPU_MARKERS
	  PFN_vkCmdDebugMarkerBeginEXT  debug_marker_begin_;
	  PFN_vkCmdDebugMarkerEndEXT    debug_marker_end_;
	  PFN_vkCmdDebugMarkerInsertEXT debug_marker_insert_;
	  bool has_debug_markers_;
	  uint32_t marker_depth_;
#endif
	};
  }
}