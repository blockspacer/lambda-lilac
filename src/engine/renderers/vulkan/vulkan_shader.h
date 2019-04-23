#pragma once
#include "assets/shader.h"
#include "vulkan_mesh.h"
#include "vulkan.h"
#include <set>

namespace lambda
{
  namespace linux
  {
    class VulkanRenderer;
    class VulkanRenderBuffer;

    ///////////////////////////////////////////////////////////////////////////
    class VulkanShader
    {
    public:
      VulkanShader(asset::VioletShaderHandle shader, VulkanRenderer* renderer);
      ~VulkanShader();
      void bind();
	  Vector<uint32_t> getStages() const;

	  Vector<VulkanReflectionInfo> getTextures();
	  Vector<VulkanReflectionInfo> getSamplers();
    Vector<VulkanReflectionInfo> getBuffers();
	  uint32_t getNumRenderTargets();
		const Vector<VkPipelineShaderStageCreateInfo>& getShaderStages();

	  void reflect();

    private:

    private:
	  Vector<uint32_t> stages_;

	  VkShaderModule vs_;
	  VkShaderModule ps_;
	  VkShaderModule gs_;

	  Vector<VulkanReflectionInfo> textures_;
	  Vector<VulkanReflectionInfo> samplers_;
      Vector<VulkanReflectionInfo> buffers_;
	  uint32_t num_render_targets_;
		Vector<VkPipelineShaderStageCreateInfo> shader_stages_;

	  VulkanRenderer* renderer_;
	  //VezVertexInputFormat input_format_;
	};
  }
}