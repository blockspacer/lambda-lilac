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

namespace lambda
{
  namespace windows
  {
		class D3D11Context;

    ///////////////////////////////////////////////////////////////////////////
    class D3D11StateManager
    {
    public:
      void initialize(D3D11Context* context);
	  void deinitialize();
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
			void reset();

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

			D3D11Context* context_;
      Microsoft::WRL::ComPtr<ID3D11RasterizerState> bound_rasterizer_state_;
      Microsoft::WRL::ComPtr<ID3D11BlendState> bound_blend_state_;
      Microsoft::WRL::ComPtr<ID3D11SamplerState> bound_sampler_state_;
      Microsoft::WRL::ComPtr<ID3D11DepthStencilState>
        bound_depth_stencil_state_;
      
      asset::Topology bound_topology_;
    };
  }
}