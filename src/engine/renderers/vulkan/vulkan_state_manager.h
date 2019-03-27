#pragma once
#include "platform/rasterizer_state.h"
#include "platform/blend_state.h"
#include "platform/sampler_state.h"
#include "platform/depth_stencil_state.h"
#include <containers/containers.h>
#include "assets/mesh.h"
#include "vulkan.h"

namespace eastl {

  /////////////////////////////////////////////////////////////////////////////
  template <>
  struct hash<lambda::platform::BlendState>
  {
    std::size_t operator()(const lambda::platform::BlendState& k) const
    {
      unsigned char a = (unsigned char)k.getBlendEnable();
      unsigned char b = ((unsigned char)k.getBlendOp()) << 4u;
      unsigned char c = (unsigned char)k.getBlendOpAlpha();
      unsigned char d = ((unsigned char)k.getDestBlend()) << 4u;
      unsigned char e = (unsigned char)k.getDestBlendAlpha();
      unsigned char f = ((unsigned char)k.getSrcBlend()) << 4u;
      unsigned char g = (unsigned char)k.getSrcBlendAlpha();
      unsigned char h = ((unsigned char)k.getWriteMask()) << 4u;

      return (a | b) | ((c | d) << 8u) | ((e | f) << 16u) | ((g | h) << 24u);
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  template <>
  struct hash<lambda::platform::RasterizerState>
  {
    std::size_t operator()(const lambda::platform::RasterizerState& k) const
    {
      unsigned char a = (unsigned char)k.getCullMode();
      unsigned char b = (unsigned char)k.getFillMode();
      return a | (b << 8u);
    }
  };
  
  /////////////////////////////////////////////////////////////////////////////
  template <>
  struct hash<lambda::platform::DepthStencilState>
  {
    std::size_t operator()(const lambda::platform::DepthStencilState& k) const
    {
      unsigned char a = (unsigned char)k.getDepthCompareOp();
      return a;
    }
  };
  
  /////////////////////////////////////////////////////////////////////////////
  template <>
  struct hash<lambda::platform::SamplerState>
  {
    std::size_t operator()(const lambda::platform::SamplerState& k) const
    {
      unsigned char a = (unsigned char)k.getClampMode();
      unsigned char b = (unsigned char)k.getSampleMode();
      return a | (b << 8u);
    }
  };
}

namespace lambda
{
  namespace linux
  {
	  class VulkanRenderer;

    ///////////////////////////////////////////////////////////////////////////
    class VulkanStateManager
    {
    public:
      void initialize(VulkanRenderer* renderer);
	  void update(Vector<VulkanReflectionInfo> samplers);
	  void bindRasterizerState(
        const platform::RasterizerState& rasterizer_state
      );
      void bindBlendState(const platform::BlendState& blend_state);
      void bindDepthStencilState(
        const platform::DepthStencilState& depth_stencil_state
      );
      void bindSamplerState(
        const platform::SamplerState& sampler_state, 
        unsigned char slots
      );
      void bindTopology(const asset::Topology& topology);

    private:
      UnorderedMap<platform::RasterizerState, 
        VezRasterizationState> rasterizer_states_;
      UnorderedMap<platform::BlendState, 
        VezColorBlendState> blend_states_;
      UnorderedMap<platform::SamplerState, 
        VkSampler> sampler_states_;
	  UnorderedMap<platform::DepthStencilState,
		  VezPipelineDepthStencilState> depth_stencil_states_;
	  UnorderedMap<asset::Topology,
		  VezInputAssemblyState> input_assembly_states_;

	  VulkanRenderer* renderer_;
	  
	  bool     dirty_rasterizer_;
	  bool     dirty_blend_;
	  bool     dirty_depth_stencil_;
	  bool     dirty_input_assembly_;
	  uint16_t dirty_sampler_;
	  VezRasterizationState        rasterizer_;
	  VezColorBlendState           blend_;
	  VkSampler                    samplers_[16];
	  VezPipelineDepthStencilState depth_stencil_;
	  VezInputAssemblyState        input_assembly_;
    };
  }
}