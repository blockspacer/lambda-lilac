#pragma once
#include "platform/rasterizer_state.h"
#include "platform/blend_state.h"
#include "platform/sampler_state.h"
#include "platform/depth_stencil_state.h"
#include <containers/containers.h>
#include "assets/mesh.h"
#include <wrl/client.h>
#include <d3d11.h> // TODO (Hilze): Remove this!

//struct ID3D11RasterizerState;
//struct ID3D11BlendState;
//struct ID3D11SamplerState;
//struct ID3D11Device;
//struct ID3D11DeviceContext;
//enum D3D11_BLEND;
//enum D3D11_BLEND_OP;

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
  namespace windows
  {
    ///////////////////////////////////////////////////////////////////////////
    class D3D11StateManager
    {
    public:
      void initialize(
        Microsoft::WRL::ComPtr<ID3D11Device> device, 
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
      );
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
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>> rasterizer_states_;
      UnorderedMap<platform::BlendState, 
        Microsoft::WRL::ComPtr<ID3D11BlendState>> blend_states_;
      UnorderedMap<platform::SamplerState, 
        Microsoft::WRL::ComPtr<ID3D11SamplerState>> sampler_states_;
      UnorderedMap<platform::DepthStencilState, 
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState>> depth_stencil_states_;
      Map<platform::BlendState::BlendMode, D3D11_BLEND> blend_mode_to_d3d11_;
      Map<platform::BlendState::BlendOp, D3D11_BLEND_OP> blend_op_to_d3d11_;
      Map<platform::DepthStencilState::DepthCompareOp, 
        D3D11_COMPARISON_FUNC> depth_compare_op_to_d3d11_;

      Microsoft::WRL::ComPtr<ID3D11Device> device_;
      Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
      Microsoft::WRL::ComPtr<ID3D11RasterizerState> bound_rasterizer_state_;
      Microsoft::WRL::ComPtr<ID3D11BlendState> bound_blend_state_;
      Microsoft::WRL::ComPtr<ID3D11SamplerState> bound_sampler_state_;
      Microsoft::WRL::ComPtr<ID3D11DepthStencilState>
        bound_depth_stencil_state_;
      
      asset::Topology bound_topology_;
    };
  }
}