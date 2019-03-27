#pragma once
#include "assets/shader.h"
#include "vulkan_mesh.h"
#include "platform/shader_variable_manager.h"
#include "vulkan.h"
#include <set>

namespace lambda
{
  namespace linux
  {
    class VulkanRenderer;
    class VulkanRenderBuffer;

    ///////////////////////////////////////////////////////////////////////////
    struct VulkanBuffer
    {
      VulkanRenderBuffer* buffer;
      size_t slot = 0u;
      bool bound = false;
    };

    ///////////////////////////////////////////////////////////////////////////
    class VulkanShader : public asset::IGPUAsset
    {
    public:
      VulkanShader(asset::VioletShaderHandle shader, VulkanRenderer* renderer);
      virtual ~VulkanShader() override;
      void bind();
      void unbind();
      void bindBuffers();
	  Vector<uint32_t> getStages() const;

      void updateShaderVariable(const platform::ShaderVariable& variable);

      Vector<platform::ShaderBuffer>& getVsBuffers();
      Vector<platform::ShaderBuffer>& getPsBuffers();
      Vector<platform::ShaderBuffer>& getGsBuffers();

	  Vector<VulkanReflectionInfo> getTextures();
	  Vector<VulkanReflectionInfo> getSamplers();
	  uint32_t getNumRenderTargets();

	  void reflect();

    private:

    private:
	  Vector<uint32_t> stages_;

	  VkShaderModule vs_;
	  VkShaderModule ps_;
	  VkShaderModule gs_;

	  Vector<VulkanReflectionInfo> textures_;
	  Vector<VulkanReflectionInfo> samplers_;
	  uint32_t num_render_targets_;

	  VulkanRenderer* renderer_;
      Vector<platform::ShaderBuffer> vs_buffers_;
      Vector<platform::ShaderBuffer> ps_buffers_;
      Vector<platform::ShaderBuffer> gs_buffers_;
      Vector<VulkanBuffer> vs_Vulkan_buffers_;
      Vector<VulkanBuffer> ps_Vulkan_buffers_;
      Vector<VulkanBuffer> gs_Vulkan_buffers_;
	  VezPipeline pipeline_;
    };
  }
}