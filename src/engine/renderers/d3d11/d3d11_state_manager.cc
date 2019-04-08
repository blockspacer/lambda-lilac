#include "d3d11_state_manager.h"
#include <d3d11.h>
#include "assets/mesh.h"
#include <utils/console.h>

namespace lambda
{
  namespace windows
  {
    ///////////////////////////////////////////////////////////////////////////
    void D3D11StateManager::initialize(
      Microsoft::WRL::ComPtr<ID3D11Device> device, 
      Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
    {
      device_  = device;
      context_ = context;

      blend_states_.clear();
      bound_blend_state_.Reset();
      bound_rasterizer_state_.Reset();
      bound_sampler_state_.Reset();
      bound_depth_stencil_state_.Reset();
      depth_stencil_states_.clear();
      rasterizer_states_.clear();
      sampler_states_.clear();

      blend_mode_to_d3d11_ = {
        { platform::BlendState::BlendMode::kZero, 
        D3D11_BLEND::D3D11_BLEND_ZERO },
        { platform::BlendState::BlendMode::kOne,
        D3D11_BLEND::D3D11_BLEND_ONE },
        { platform::BlendState::BlendMode::kSrcColour,
        D3D11_BLEND::D3D11_BLEND_SRC_COLOR },
        { platform::BlendState::BlendMode::kInvSrcColour,
        D3D11_BLEND::D3D11_BLEND_INV_SRC_COLOR },
        { platform::BlendState::BlendMode::kSrcAlpha,
        D3D11_BLEND::D3D11_BLEND_SRC_ALPHA },
        { platform::BlendState::BlendMode::kInvSrcAlpha,
        D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA },
        { platform::BlendState::BlendMode::kDestAlpha,
        D3D11_BLEND::D3D11_BLEND_DEST_ALPHA },
        { platform::BlendState::BlendMode::kInvDestAlpha,
        D3D11_BLEND::D3D11_BLEND_INV_DEST_ALPHA },
        { platform::BlendState::BlendMode::kDestColour,
        D3D11_BLEND::D3D11_BLEND_DEST_COLOR },
        { platform::BlendState::BlendMode::kInvDestColour,
        D3D11_BLEND::D3D11_BLEND_INV_DEST_COLOR },
        { platform::BlendState::BlendMode::kSrcAlphaSat,
        D3D11_BLEND::D3D11_BLEND_SRC_ALPHA_SAT },
        { platform::BlendState::BlendMode::kBlendFactor,
        D3D11_BLEND::D3D11_BLEND_BLEND_FACTOR },
        { platform::BlendState::BlendMode::kInvBlendFactor,
        D3D11_BLEND::D3D11_BLEND_INV_BLEND_FACTOR },
        { platform::BlendState::BlendMode::kSrc1Colour,
        D3D11_BLEND::D3D11_BLEND_SRC1_COLOR },
        { platform::BlendState::BlendMode::kInvSrc1Colour,
        D3D11_BLEND::D3D11_BLEND_INV_SRC1_COLOR },
        { platform::BlendState::BlendMode::kSrc1Alpha,
        D3D11_BLEND::D3D11_BLEND_SRC1_ALPHA },
        { platform::BlendState::BlendMode::kInvSrc1Alpha,
        D3D11_BLEND::D3D11_BLEND_INV_SRC1_ALPHA }
      };

      blend_op_to_d3d11_ = {
        { platform::BlendState::BlendOp::kAdd, 
        D3D11_BLEND_OP::D3D11_BLEND_OP_ADD },
        { platform::BlendState::BlendOp::kSubtract, 
        D3D11_BLEND_OP::D3D11_BLEND_OP_SUBTRACT },
        { platform::BlendState::BlendOp::kRevSubtract, 
        D3D11_BLEND_OP::D3D11_BLEND_OP_REV_SUBTRACT },
        { platform::BlendState::BlendOp::kMin, 
        D3D11_BLEND_OP::D3D11_BLEND_OP_MIN },
        { platform::BlendState::BlendOp::kMax, 
        D3D11_BLEND_OP::D3D11_BLEND_OP_MAX }
      };

      depth_compare_op_to_d3d11_ = {
        { platform::DepthStencilState::DepthCompareOp::kNever,
        D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER },
        { platform::DepthStencilState::DepthCompareOp::kAlways,
        D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS },
        { platform::DepthStencilState::DepthCompareOp::kEqual,
        D3D11_COMPARISON_FUNC::D3D11_COMPARISON_EQUAL },
        { platform::DepthStencilState::DepthCompareOp::kNotEqual,
        D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NOT_EQUAL },
        { platform::DepthStencilState::DepthCompareOp::kLess,
        D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS },
        { platform::DepthStencilState::DepthCompareOp::kLessEqual,
        D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL },
        { platform::DepthStencilState::DepthCompareOp::kGreater,
        D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER },
        { platform::DepthStencilState::DepthCompareOp::kGreaterEqual,
        D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER_EQUAL },
      };
    }

	///////////////////////////////////////////////////////////////////////////
	void D3D11StateManager::deinitialize()
	{
		rasterizer_states_.clear();
		blend_states_.clear();
		sampler_states_.clear();
		depth_stencil_states_.clear();
		blend_mode_to_d3d11_.clear();
		blend_op_to_d3d11_.clear();
		depth_compare_op_to_d3d11_.clear();

		device_                    = nullptr;
		context_                   = nullptr;
		bound_rasterizer_state_    = nullptr;
		bound_blend_state_         = nullptr;
		bound_sampler_state_       = nullptr;
		bound_depth_stencil_state_ = nullptr;
	}
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11StateManager::bindRasterizerState(
      const platform::RasterizerState& rasterizer_state)
    {
      LMB_ASSERT(device_ != nullptr, "TODO (Hilze): Fill in");
      LMB_ASSERT(context_ != nullptr, "TODO (Hilze): Fill in");

      // Get the state.
      Microsoft::WRL::ComPtr<ID3D11RasterizerState> new_rasterizer_state;
      const auto& it = rasterizer_states_.find(rasterizer_state);
      if (it == rasterizer_states_.end())
      {
        D3D11_RASTERIZER_DESC desc{};
        desc.DepthClipEnable = true;
        desc.ScissorEnable   = true;
        switch (rasterizer_state.getCullMode())
        {
        case platform::RasterizerState::CullMode::kBack: 
          desc.CullMode = D3D11_CULL_BACK;  break;
        case platform::RasterizerState::CullMode::kFront: 
          desc.CullMode = D3D11_CULL_FRONT; break;
        case platform::RasterizerState::CullMode::kNone: 
          desc.CullMode = D3D11_CULL_NONE;  break;
        }
        switch (rasterizer_state.getFillMode())
        {
        case platform::RasterizerState::FillMode::kSolid:     
          desc.FillMode = D3D11_FILL_SOLID;     break;
        case platform::RasterizerState::FillMode::kWireframe: 
          desc.FillMode = D3D11_FILL_WIREFRAME; break;
        }
        device_->CreateRasterizerState(
          &desc, 
          new_rasterizer_state.ReleaseAndGetAddressOf()
        );
        rasterizer_states_.insert(
          eastl::make_pair(rasterizer_state, new_rasterizer_state)
        );
      }
      else
      {
        new_rasterizer_state = it->second;
      }

      // Check the state.
      if (new_rasterizer_state == bound_rasterizer_state_)
        return;

      bound_rasterizer_state_ = new_rasterizer_state;

      // Set the state.
      context_->RSSetState(bound_rasterizer_state_.Get());
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11StateManager::bindBlendState(
      const platform::BlendState& blend_state)
    {
      LMB_ASSERT(device_ != nullptr, "TODO (Hilze): Fill in");
      LMB_ASSERT(context_ != nullptr, "TODO (Hilze): Fill in");

      // Get the state.
      Microsoft::WRL::ComPtr<ID3D11BlendState> new_blend_state;
      const auto& it = blend_states_.find(blend_state);
      if (it == blend_states_.end())
      {
        D3D11_BLEND_DESC desc{};
        desc.IndependentBlendEnable = false;
        desc.AlphaToCoverageEnable  = blend_state.getAlphaToCoverage();
        desc.RenderTarget[0].BlendEnable = blend_state.getBlendEnable();
        desc.RenderTarget[0].BlendOp =
          blend_op_to_d3d11_.at(blend_state.getBlendOp());
        desc.RenderTarget[0].BlendOpAlpha = 
          blend_op_to_d3d11_.at(blend_state.getBlendOpAlpha());
        desc.RenderTarget[0].DestBlend =
          blend_mode_to_d3d11_.at(blend_state.getDestBlend());
        desc.RenderTarget[0].DestBlendAlpha = 
          blend_mode_to_d3d11_.at(blend_state.getDestBlendAlpha());
        desc.RenderTarget[0].SrcBlend = 
          blend_mode_to_d3d11_.at(blend_state.getSrcBlend());
        desc.RenderTarget[0].SrcBlendAlpha = 
          blend_mode_to_d3d11_.at(blend_state.getSrcBlendAlpha());
        desc.RenderTarget[0].RenderTargetWriteMask = 
          blend_state.getWriteMask();

        device_->CreateBlendState(
          &desc, 
          new_blend_state.ReleaseAndGetAddressOf()
        );
        
        blend_states_.insert(eastl::make_pair(blend_state, new_blend_state));
      }
      else
      {
        new_blend_state = it->second;
      }

      // Check the state.
      if (new_blend_state == bound_blend_state_)
      {
        return;
      }
      bound_blend_state_ = new_blend_state;

      // Set the state.
      static const float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
      context_->OMSetBlendState(
        bound_blend_state_.Get(),
        blend_factor, 
        0xffffffff
      );
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11StateManager::bindDepthStencilState(
      const platform::DepthStencilState & depth_stencil_state)
    {
      LMB_ASSERT(device_ != nullptr, "TODO (Hilze): Fill in");
      LMB_ASSERT(context_ != nullptr, "TODO (Hilze): Fill in");

      // Get the state.
      Microsoft::WRL::ComPtr<ID3D11DepthStencilState> new_depth_stencil_state;
      const auto& it = depth_stencil_states_.find(depth_stencil_state);
      if (it == depth_stencil_states_.end())
      {
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
        desc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
        desc.DepthEnable    = TRUE;
        desc.DepthWriteMask = (depth_stencil_state.getDepthWritemode() == 0u) 
          ? D3D11_DEPTH_WRITE_MASK_ZERO : D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc      = depth_compare_op_to_d3d11_.at(
          depth_stencil_state.getDepthCompareOp()
        );
        desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
        desc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
        desc.StencilEnable    = FALSE;
        desc.StencilReadMask  = D3D11_DEFAULT_STENCIL_READ_MASK;
        desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        

        device_->CreateDepthStencilState(
          &desc, 
          new_depth_stencil_state.ReleaseAndGetAddressOf()
        );
        depth_stencil_states_.insert(
          eastl::make_pair(depth_stencil_state, new_depth_stencil_state)
        );
      }
      else
        new_depth_stencil_state = it->second;

      if (new_depth_stencil_state == bound_depth_stencil_state_)
        return;

      bound_depth_stencil_state_ = new_depth_stencil_state;

      context_->OMSetDepthStencilState(new_depth_stencil_state.Get(), 0u);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11StateManager::bindSamplerState(
      const platform::SamplerState& sampler_state, 
      unsigned char slot)
    {
      LMB_ASSERT(device_ != nullptr, "TODO (Hilze): Fill in");
      LMB_ASSERT(context_ != nullptr, "TODO (Hilze): Fill in");

      // Get the state.
      Microsoft::WRL::ComPtr<ID3D11SamplerState> new_sampler_state;
      const auto& it = sampler_states_.find(sampler_state);
      if (it == sampler_states_.end())
      {
        D3D11_SAMPLER_DESC desc{};
        desc.MaxLOD         = D3D11_FLOAT32_MAX;
        desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        desc.BorderColor[0] = desc.BorderColor[1] = 
          desc.BorderColor[2] = desc.BorderColor[3] = 0.0f;

        switch (sampler_state.getClampMode())
        {
        case platform::SamplerState::ClampMode::kBorder: 
          desc.AddressU = desc.AddressV = 
            desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER; break;
        case platform::SamplerState::ClampMode::kClamp:  
          desc.AddressU = desc.AddressV =
            desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; break;
        case platform::SamplerState::ClampMode::kWrap:   
          desc.AddressU = desc.AddressV = 
            desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; break;
        }
        switch (sampler_state.getSampleMode())
        {
        case platform::SamplerState::SampleMode::kAnisotropic:
          desc.Filter = D3D11_FILTER_ANISOTROPIC; break;
        case platform::SamplerState::SampleMode::kLinear:     
          desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
        case platform::SamplerState::SampleMode::kPoint:     
          desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
        }

        device_->CreateSamplerState(
          &desc, 
          new_sampler_state.ReleaseAndGetAddressOf()
        );
        sampler_states_.insert(
          eastl::make_pair(sampler_state, new_sampler_state)
        );
      }
      else
        new_sampler_state = it->second;

      // Check the state.
      if (new_sampler_state == bound_sampler_state_)
        return;

      bound_sampler_state_ = new_sampler_state;

      // Set the state.
      context_->PSSetSamplers(slot, 1u, bound_sampler_state_.GetAddressOf());
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11StateManager::bindTopology(const asset::Topology& topology)
    {
      if (topology == bound_topology_)
      {
        return;
      }
      bound_topology_ = topology;

      D3D11_PRIMITIVE_TOPOLOGY new_topology = 
        D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
      switch (topology)
      {
      case asset::Topology::kLines:
        new_topology = 
          D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        break;
      case asset::Topology::kTriangles:
        new_topology = 
          D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;
      }

      context_->IASetPrimitiveTopology(new_topology);
    }
  }
}