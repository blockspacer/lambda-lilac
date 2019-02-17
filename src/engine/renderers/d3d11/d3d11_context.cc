#include "d3d11_context.h"
#include <assert.h>
#include <d3d11.h>
#include "assets/shader.h"
#include "d3d11_shader.h"
#include "d3d11_mesh.h"
#include "assets/asset_manager.h"
#include "d3d11_texture.h"
#include "utils/decompose_matrix.h"
#include "platform/shader_variable.h"
#include <utils/console.h>
#include "interfaces/iimgui.h"
#include <utils/utilities.h>
#include "d3d11_renderer.h"
#include <memory/frame_heap.h>
#include "interfaces/iworld.h"

namespace lambda
{
  namespace windows
  {
    ///////////////////////////////////////////////////////////////////////////
    D3D11RenderBuffer::D3D11RenderBuffer(
      uint32_t size, 
      uint32_t flags, 
      ID3D11Buffer* buffer, 
      ID3D11DeviceContext* context) 
      : size_(size)
      , flags_(flags)
      , buffer_(buffer)
      , context_(context)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void* D3D11RenderBuffer::lock()
    {
      LMB_ASSERT(flags_ & kFlagDynamic, "TODO (Hilze): Fill in");

      D3D11_MAPPED_SUBRESOURCE resource;
      context_->Map(buffer_, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &resource);
      return resource.pData;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11RenderBuffer::unlock()
    {
      LMB_ASSERT(flags_ & kFlagDynamic, "TODO (Hilze): Fill in");
      
      context_->Unmap(buffer_, 0u);
    }

    ///////////////////////////////////////////////////////////////////////////
    uint32_t D3D11RenderBuffer::getFlags() const
    {
      return flags_;
    }

    ///////////////////////////////////////////////////////////////////////////
    uint32_t D3D11RenderBuffer::getSize() const
    {
      return size_;
    }

    ///////////////////////////////////////////////////////////////////////////
    ID3D11Buffer* D3D11RenderBuffer::getBuffer() const
    {
      return buffer_;
    }

    ///////////////////////////////////////////////////////////////////////////
    D3D11RenderTexture::D3D11RenderTexture(
      uint32_t width, 
      uint32_t height, 
      uint32_t depth,
      uint32_t mip_count, 
      TextureFormat format, 
      uint32_t flags, 
      D3D11Texture* texture, 
      ID3D11DeviceContext* context)
      : flags_(flags)
      , width_(width)
      , height_(height)
      , depth_(depth)
      , mip_count_(mip_count)
      , format_(format)
      , texture_(texture)
      , context_(context)
    {
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void* D3D11RenderTexture::lock(uint32_t level)
    {
      //LMB_ASSERT(flags_ & kFlagDynamic, "TODO (Hilze): Fill in");

      D3D11_MAPPED_SUBRESOURCE resource;
      context_->Map(
        texture_->getTexture(0u), 
        level, 
        D3D11_MAP_WRITE_DISCARD, 
        0u, 
        &resource
      );
      
      return resource.pData;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11RenderTexture::unlock(uint32_t level)
    {
      //LMB_ASSERT(flags_ & kFlagDynamic, "TODO (Hilze): Fill in");

      context_->Unmap(texture_->getTexture(0u), level);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    uint32_t D3D11RenderTexture::getWidth() const
    {
      return width_;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    uint32_t D3D11RenderTexture::getHeight() const
    {
      return height_;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    uint32_t D3D11RenderTexture::getDepth() const
    {
      return depth_;
    }

    ///////////////////////////////////////////////////////////////////////////
    uint32_t D3D11RenderTexture::getMipCount() const
    {
      return mip_count_;
    }

    ///////////////////////////////////////////////////////////////////////////
    uint32_t D3D11RenderTexture::getFlags() const
    {
      return flags_;
    }

    ///////////////////////////////////////////////////////////////////////////
    TextureFormat D3D11RenderTexture::getFormat() const
    {
      return format_;
    }

    ///////////////////////////////////////////////////////////////////////////
    D3D11Texture* D3D11RenderTexture::getTexture() const
    {
      return texture_;
    }

    ///////////////////////////////////////////////////////////////////////////
    platform::IRenderBuffer* D3D11Context::allocRenderBuffer(
      uint32_t size, 
      uint32_t flags, 
      void* data)
    {
      const bool is_dynamic   = 
        (flags & platform::IRenderBuffer::kFlagDynamic)   ? true : false;
      const bool is_staging   = 
        (flags & platform::IRenderBuffer::kFlagStaging)   ? true : false;
      const bool is_immutable = 
        (flags & platform::IRenderBuffer::kFlagImmutable) ? true : false;
      const bool is_vertex    = 
        (flags & platform::IRenderBuffer::kFlagVertex)    ? true : false;
      const bool is_index     = 
        (flags & platform::IRenderBuffer::kFlagIndex)     ? true : false;
      const bool is_constant  = 
        (flags & platform::IRenderBuffer::kFlagConstant)  ? true : false;

      D3D11_BUFFER_DESC buffer_desc{};
      buffer_desc.ByteWidth           = size;
      buffer_desc.Usage               = is_dynamic ? D3D11_USAGE_DYNAMIC      : 
        (is_staging ? D3D11_USAGE_STAGING     : 
        (is_immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT));
      buffer_desc.BindFlags           = is_vertex  ? D3D11_BIND_VERTEX_BUFFER : 
        (is_index   ? D3D11_BIND_INDEX_BUFFER : 
        (is_constant  ? D3D11_BIND_CONSTANT_BUFFER : 0u));
      buffer_desc.CPUAccessFlags      = is_dynamic ? D3D11_CPU_ACCESS_WRITE   : 
        (is_staging ? D3D11_CPU_ACCESS_WRITE  : 0u);
      buffer_desc.MiscFlags           = 0u;
      buffer_desc.StructureByteStride = 0u;

      D3D11_SUBRESOURCE_DATA subresource{};
      subresource.pSysMem = data;

      ID3D11Buffer* buffer;
      context_.device->CreateBuffer(
        &buffer_desc, 
        data ? &subresource : nullptr, 
        &buffer
      );

      if (is_vertex)
        memory_stats_.vertex   += size;
      if (is_index)
        memory_stats_.index    += size;
      if (is_constant)
        memory_stats_.constant += size;

      return foundation::Memory::construct<D3D11RenderBuffer>(
        size, 
        flags, 
        buffer, 
        context_.context.Get()
        );
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::freeRenderBuffer(platform::IRenderBuffer*& buffer)
    {
      const bool is_vertex   = 
        (buffer->getFlags() & platform::IRenderBuffer::kFlagVertex)   
        ? true : false;
      const bool is_index    = 
        (buffer->getFlags() & platform::IRenderBuffer::kFlagIndex)   
        ? true : false;
      const bool is_constant = 
        (buffer->getFlags() & platform::IRenderBuffer::kFlagConstant)  
        ? true : false;

      if (is_vertex)
        memory_stats_.vertex   -= buffer->getSize();
      if (is_index)
        memory_stats_.index    -= buffer->getSize();
      if (is_constant)
        memory_stats_.constant -= buffer->getSize();

      static_cast<D3D11RenderBuffer*>(buffer)->getBuffer()->Release();
      foundation::Memory::destruct(buffer);
      buffer = nullptr;
    }
  
    ///////////////////////////////////////////////////////////////////////////
    platform::IRenderTexture* D3D11Context::allocRenderTexture(
      asset::VioletTextureHandle texture)
    {
      //const bool generate_mips = 
      //  (texture->getLayer(0u).getFlags() & kTextureFlagMipMaps) 
      //    ? true : false;
      //const uint32_t mip_count = 
      //  generate_mips ? 
      //    ((uint32_t)floorf(std::log2f(std::fminf(
      //      (float)texture->getLayer(0u).getWidth(), 
      //      (float)texture->getLayer(0u).getHeight())) + 1.0f)) : 1u;

      D3D11RenderTexture* d3d11_texture = 
        foundation::Memory::construct<D3D11RenderTexture>(
        texture->getLayer(0u).getWidth(),
        texture->getLayer(0u).getHeight(),
        texture->getLayerCount(),
        texture->getLayer(0u).getMipCount(),
        texture->getLayer(0u).getFormat(),
        texture->getLayer(0u).getFlags(),
        foundation::Memory::construct<D3D11Texture>(
          texture, 
          context_.device.Get(), 
          context_.context.Get()
        ),
        context_.context.Get()
      );

      uint32_t size = 0u;
      for (uint32_t i = 0u; i < texture->getLayerCount(); ++i)
      {
        uint32_t w = texture->getLayer(0u).getWidth();
        uint32_t h = texture->getLayer(0u).getHeight();
        uint32_t bpp, bpr, bpl;
        for (uint32_t j = 0u; j < texture->getLayer(0u).getMipCount(); ++j)
        {
          calculateImageMemory(
            texture->getLayer(0u).getFormat(),
            w, 
            h, 
            bpp, 
            bpr, 
            bpl
          );
          size += bpl;
          w /= 2u;
          h /= 2u;
        }
      }
      if ((texture->getLayer(0u).getFlags() & kTextureFlagIsRenderTarget) 
        != 0u)
        memory_stats_.render_target += size;
      else
        memory_stats_.texture += size;

      return d3d11_texture;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::freeRenderTexture(platform::IRenderTexture*& texture)
    {
      uint32_t size = 0u;
      for (uint32_t i = 0u; i < texture->getDepth(); ++i)
      {
        uint32_t w = texture->getWidth();
        uint32_t h = texture->getHeight();
        uint32_t bpp, bpr, bpl;
        for (uint32_t j = 0u; j < texture->getMipCount(); ++j)
        {
          calculateImageMemory(texture->getFormat(), w, h, bpp, bpr, bpl);
          size += bpl;
          w /= 2u;
          h /= 2u;
        }
      }
      if ((texture->getFlags() & kTextureFlagIsRenderTarget) != 0u)
        memory_stats_.render_target -= size;
      else
        memory_stats_.texture -= size;

      foundation::Memory::destruct(
        ((D3D11RenderTexture*)texture)->getTexture()
      );
      foundation::Memory::destruct(texture);
    }
   
    ///////////////////////////////////////////////////////////////////////////
    ID3D11DeviceContext* D3D11Context::getD3D11Context() const
    {
      return context_.context.Get();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    ID3D11Device* D3D11Context::getD3D11Device() const
    {
      return context_.device.Get();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    D3D11Context::~D3D11Context()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setWindow(
      foundation::SharedPointer<platform::IWindow> window)
    {
      bound_.mesh   = nullptr;
      bound_.shader = nullptr;
      for (uint8_t i = 0u; i < 8u; ++i)
      {
        bound_.textures[i] = nullptr;
        textures_[i] = nullptr;
      }
      mesh_ = asset::MeshHandle();
      shader_ = asset::VioletShaderHandle();
      
      asset::AssetManager::getInstance().destroyAllGPUAssets();
      asset_manager_ = D3D11AssetManager();

      // Create device and context.
      {
        HRESULT result;
        DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
        IDXGIAdapter* adapter = nullptr;

        swap_chain_desc.BufferDesc.Width  = window->getSize().x;
        swap_chain_desc.BufferDesc.Height = window->getSize().y;
        swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
        swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
        swap_chain_desc.BufferDesc.ScanlineOrdering =
          DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        swap_chain_desc.BufferCount        = 1;
        swap_chain_desc.OutputWindow       = (HWND)window->getWindow();
        swap_chain_desc.SampleDesc.Count   = 1;
        swap_chain_desc.SampleDesc.Quality = 0;
        swap_chain_desc.Windowed           = true;
        swap_chain_desc.BufferUsage        = 
          DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        swap_chain_desc.SwapEffect         =
          DXGI_SWAP_EFFECT_DISCARD;// DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swap_chain_desc.Flags              =
          DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

#ifdef ENUM_ADAPTERS
        IDXGIFactory* factory = nullptr;
        CreateDXGIFactory(IID_PPV_ARGS(&factory));

        int idx = -1;
        IDXGIAdapter* selected_adapter = 0;
        size_t dedicated_memory = 0;
        while (SUCCEEDED(factory->EnumAdapters(++idx,&adapter)))
        {
          DXGI_ADAPTER_DESC desc{};
          adapter->GetDesc(&desc);
          if (desc.DedicatedVideoMemory > dedicated_memory)
          {
            if (selected_adapter != nullptr)
            {
              selected_adapter->Release();
            }
            dedicated_memory = desc.DedicatedVideoMemory;
            selected_adapter = adapter;
          }
          else
          {
            adapter->Release();
          }
        }
        adapter = selected_adapter;
#endif
        D3D_FEATURE_LEVEL feature_level[] = {
          D3D_FEATURE_LEVEL_11_1,
          D3D_FEATURE_LEVEL_11_0,
          D3D_FEATURE_LEVEL_10_1,
          D3D_FEATURE_LEVEL_10_0,
          D3D_FEATURE_LEVEL_9_3,
          D3D_FEATURE_LEVEL_9_2,
          D3D_FEATURE_LEVEL_9_1
        };
        D3D_FEATURE_LEVEL supported_level;

        UINT create_device_flags = 0;
#ifdef _DEBUG
        create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        D3D_DRIVER_TYPE driver_types[] = {
          D3D_DRIVER_TYPE_HARDWARE,
          D3D_DRIVER_TYPE_REFERENCE,
          D3D_DRIVER_TYPE_SOFTWARE,
          D3D_DRIVER_TYPE_WARP,
          D3D_DRIVER_TYPE_NULL,
          D3D_DRIVER_TYPE_UNKNOWN,
        };

        for (int i = 0; i < _countof(driver_types); ++i)
        {
          result = D3D11CreateDeviceAndSwapChain(
            adapter,
            driver_types[i],
            NULL,
            create_device_flags,
            feature_level,
            sizeof(feature_level) / sizeof(feature_level[0]),
            D3D11_SDK_VERSION,
            &swap_chain_desc,
            context_.swap_chain.ReleaseAndGetAddressOf(),
            context_.device.ReleaseAndGetAddressOf(),
            &supported_level,
            context_.context.ReleaseAndGetAddressOf()
          );
          if (result == E_INVALIDARG)
          {
            D3D11CreateDeviceAndSwapChain(
              adapter,
              driver_types[i],
              NULL,
              create_device_flags,
              &feature_level[1],
              sizeof(feature_level) / sizeof(feature_level[0]) - 1,
              D3D11_SDK_VERSION,
              &swap_chain_desc,
              context_.swap_chain.ReleaseAndGetAddressOf(),
              context_.device.ReleaseAndGetAddressOf(),
              &supported_level,
              context_.context.ReleaseAndGetAddressOf()
            );
          }

          if (SUCCEEDED(result))
          {
            foundation::Info("Renderer: Using feature level " + 
              toString(supported_level) + "\n");
            foundation::Info("Renderer: Using driver type " + 
              toString(i) + "\n");
            break;
          }
        }

        if (FAILED(result))
        {
          MessageBoxA((HWND)window->getWindow(), 
            "CreateDeviceAndSwapChain failed!", "Graphics Error", 0);
          return;
        }
      }

#if GPU_MARKERS
      HRESULT result = context_.context->QueryInterface(
        IID_PPV_ARGS(&user_defined_annotation_)
      );

      if (FAILED(result))
      {
        MessageBoxA((HWND)window->getWindow(), 
          "QueryInterface failed!", "Graphics Error", 0);
        return;
      }
#endif

#if GPU_TIMERS
      D3D11_QUERY_DESC query_desc{};
      query_desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
      context_.device->CreateQuery(
        &query_desc, 
        timer_disjoint_.ReleaseAndGetAddressOf()
      );
#endif

      state_manager_.initialize(context_.device, context_.context);
      asset_manager_.setD3D11Context(this);
      asset_manager_.setDevice(context_.device.Get());
      asset_manager_.setContext(context_.context.Get());

      resize();
      setVSync(context_.vsync);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::initialize(world::IWorld* world)
    {
			world_ = world;

      full_screen_quad_.mesh = 
        asset::AssetManager::getInstance().createAsset<asset::Mesh>(
          Name("__full_screen_quad_mesh__"),
          foundation::Memory::constructShared<asset::Mesh>(
            asset::Mesh::createScreenQuad()
          )
      );

			full_screen_quad_.shader = asset::ShaderManager::getInstance()->get(Name("resources/shaders/full_screen_quad.fx"));
    }
    void D3D11Context::deinitialize()
    {
      world_->getWindow().reset();
    }
    void D3D11Context::resize()
    {
      unsigned int width  = 0;
      unsigned int height = 0;
      
      context_.context->OMSetRenderTargets(0, nullptr, nullptr);
      context_.backbuffer.Reset();

      glm::uvec2 render_size = 
        (glm::uvec2)((glm::vec2)world_->getWindow()->getSize() *
          world_->getWindow()->getDPIMultiplier());
      context_.swap_chain->ResizeBuffers(
        1, 
        render_size.x,
        render_size.y, 
        DXGI_FORMAT_R8G8B8A8_UNORM, 
        0
      );

      ID3D11Texture2D* back_buffer;
      HRESULT result = context_.swap_chain->GetBuffer(
        0, 
        __uuidof(ID3D11Texture2D), 
        (void**)&back_buffer
      );
      
      D3D11_TEXTURE2D_DESC tex_desc{};
      back_buffer->GetDesc(&tex_desc);
      width  = tex_desc.Width;
      height = tex_desc.Height;
      screen_size_.x = (float)width;
      screen_size_.y = (float)height;
      
      if (FAILED(result))
      {
        MessageBoxA((HWND)world_->getWindow()->getWindow(), 
          "GetBackBuffer failed!", "Graphics Error", 0);
        return;
      }

      result = context_.device->CreateRenderTargetView(
        back_buffer, 
        NULL,
        context_.backbuffer.ReleaseAndGetAddressOf()
      );
      if (FAILED(result))
      {
        MessageBoxA((HWND)world_->getWindow()->getWindow(),
          "CreateRenderTargetView failed!", "Graphics Error", 0);
        return;
      }
      back_buffer->Release();

      default_.viewport.x = 0.0f;
      default_.viewport.y = 0.0f;
      default_.viewport.z = (float)width;
      default_.viewport.w = (float)height;
			setViewports(Vector<glm::vec4>{ default_.viewport });

			world_->getPostProcessManager().resize(glm::vec2(width, height) * render_scale_);
      
      setScissorRect(glm::vec4(0.0f, 0.0f, world_->getWindow()->getSize()));

      world_->getShaderVariableManager().setVariable(
        platform::ShaderVariable(Name("screen_size"),
          screen_size_)
      );
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::update(const double& delta_time)
    {
      delta_time_ = delta_time;
      total_time_ += delta_time_;
      setShaderVariable(
        platform::ShaderVariable(Name("delta_time"), (float)delta_time_)
      );
      setShaderVariable(
        platform::ShaderVariable(Name("total_time"), (float)total_time_)
      );

      /*static bool is_open = false;
      if (world_->getImGUI()->imBegin("MemStats", is_open, 
      glm::vec2(0.0f, 0.0f), glm::vec2(200.0f, 200.0f)))
      {
        world_->getImGUI()->imText("GPU Vtx MEM: " + 
        toString(TO_MB((float)memory_stats_.vertex)) + " MiB");
        world_->getImGUI()->imText("GPU Idx MEM: " + 
        toString(TO_MB((float)memory_stats_.index)) + " MiB");
        world_->getImGUI()->imText("GPU Uni MEM: " + 
        toString(TO_MB((float)memory_stats_.constant)) + " MiB");
        world_->getImGUI()->imText("GPU Tex MEM: " + 
        toString(TO_MB((float)memory_stats_.texture)) + " MiB");
        world_->getImGUI()->imText("GPU RT  MEM: " + 
        toString(TO_MB((float)memory_stats_.render_target)) + " MiB");
        world_->getImGUI()->imText("GPU Tmp MEM: " +
        toString(TO_MB(foundation::GetFrameHeap()->currentHeapSize())) + 
        " MiB");
      }
      world_->getImGUI()->imEnd();*/
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::startFrame()
    {
      // Clear everything.
      beginTimer("Clear Everything");
      pushMarker("Clear Everything");
      glm::vec4 colour(0.0f);
      for (const auto& target : world_->getPostProcessManager().getAllTargets())
      {
        if (target.second.getTexture()->getLayer(0u).getFlags()
          & kTextureFlagClear)
        {
          clearRenderTarget(target.second.getTexture(), colour);
        }
      }
      popMarker();
      endTimer("Clear Everything");

      // Reset everything back to normal.
      state_manager_.bindTopology(asset::Topology::kTriangles);
      setSamplerState(platform::SamplerState::LinearWrap(), 0u);
      setRasterizerState(platform::RasterizerState::SolidFront());

      setMesh(asset::MeshHandle());
      setShader(asset::VioletShaderHandle());
      setSubMesh(0);
      for (unsigned char i = 0u; i < 8u; ++i)
      {
        setTexture(asset::VioletTextureHandle(), i);
      }

      // Handle markers.
#if GPU_TIMERS
      while (context_.context->GetData(timer_disjoint_.Get(), NULL, 0, 0) 
        == S_FALSE)
        Sleep(1);

      D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timestamp_disjoint;
      context_.context->GetData(timer_disjoint_.Get(), &timestamp_disjoint, 
        sizeof(timestamp_disjoint), 0);
      
      for (auto& timer : gpu_timers_)
      {
        uint64_t push, pop;
        context_.context->GetData(timer.second.push[gpu_timer_idx].Get(),
          &push, sizeof(uint64_t), 0);
        context_.context->GetData(timer.second.pop[gpu_timer_idx].Get(), 
          &pop, sizeof(uint64_t), 0);
        if (pop <= push)
          timer.second.micro_seconds = 0u;
        else
          timer.second.micro_seconds = (pop - push) / 
          (timestamp_disjoint.Frequency / 1000000u);
      }
      gpu_timer_idx = gpu_timer_idx == 0u ? 1u : 0u;
      context_.context->Begin(timer_disjoint_.Get());
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::endFrame(bool display)
    {
      setMesh(full_screen_quad_.mesh);
      setSubMesh(0u);
      setSamplerState(platform::SamplerState::PointClamp(),         6u);
      setSamplerState(platform::SamplerState::LinearClamp(),        7u);
      setSamplerState(platform::SamplerState::AnisotrophicClamp(),  8u);
      setSamplerState(platform::SamplerState::PointBorder(),        9u);
			setSamplerState(platform::SamplerState::LinearBorder(),       10u);
			setSamplerState(platform::SamplerState::AnisotrophicBorder(), 11u);
      setSamplerState(platform::SamplerState::PointWrap(),          12u);
			setSamplerState(platform::SamplerState::LinearWrap(),         13u);
			setSamplerState(platform::SamplerState::AnisotrophicWrap(),   14u);
      setRasterizerState(platform::RasterizerState::SolidBack());
      setBlendState(platform::BlendState::Default());
      setDepthStencilState(platform::DepthStencilState::Default());

      beginTimer("Post Processing");
      pushMarker("Post Processing");
      for (auto& pass : world_->getPostProcessManager().getPasses())
      {
        if (pass.getEnabled())
        {
          pushMarker(pass.getName().getName());
          bindShaderPass(pass);
          draw();
          popMarker();
        }
        else
        {
          setMarker(pass.getName().getName() + " - Disabled");
        }
      }
      popMarker();
      endTimer("Post Processing");

      beginTimer("Debug Rendering");
      pushMarker("Debug Rendering");
      world_->getDebugRenderer().Render(world_);
      popMarker();
      endTimer("Debug Rendering");

      beginTimer("Copy To Screen");
      pushMarker("Copy To Screen");
      copyToScreen(
        world_->getPostProcessManager().getTarget(
          world_->getPostProcessManager().getFinalTarget()
        ).getTexture()
      );
      popMarker();
      endTimer("Copy To Screen");

      beginTimer("ImGUI");
      pushMarker("ImGUI");
      // Update ImGUI.
      world_->getImGUI()->endFrame();
      platform::ImGUICommandList command_list = world_->getImGUI()->getCommandList();
      if(command_list.commands.size() > 0u)
      {
        platform::BlendState blend_state(
          false,
          true,
          platform::BlendState::BlendMode::kSrcAlpha,
          platform::BlendState::BlendMode::kInvSrcAlpha,
          platform::BlendState::BlendOp::kAdd,
          platform::BlendState::BlendMode::kInvSrcAlpha,
          platform::BlendState::BlendMode::kZero,
          platform::BlendState::BlendOp::kAdd,
          (uint8_t)platform::BlendState::WriteMode::kColourWriteEnableRGBA
        );
        platform::RasterizerState rasterizer_state(
          platform::RasterizerState::FillMode::kSolid,
          platform::RasterizerState::CullMode::kNone
        );
        platform::SamplerState sampler_state(
          platform::SamplerState::SampleMode::kLinear,
          platform::SamplerState::ClampMode::kClamp
        );

        setBlendState(blend_state);
        setRasterizerState(rasterizer_state);
        setSamplerState(sampler_state, 0u);
        setSamplerState(sampler_state, 1u);

        setShader(command_list.shader);
        setMesh(command_list.mesh);
        setShaderVariable(
          platform::ShaderVariable(
            Name("projection_matrix"), 
            command_list.projection
          )
        );
        context_.context->OMSetRenderTargets(
          1u, 
          context_.backbuffer.GetAddressOf(), 
          nullptr
        );
        setViewports({ default_.viewport });
        
        for (size_t i = 0u; i < command_list.commands.size(); ++i)
        {
          if (i >= command_list.mesh->getSubMeshes().size())
            continue;
          setScissorRect(command_list.commands.at(i).scissor_rect);
          setSubMesh((uint32_t)i);
          setTexture(command_list.mesh->getAttachedTextures().at(i), 0u);
          draw();
        } 
        setScissorRect(glm::vec4(0.0f, 0.0f, world_->getWindow()->getSize()));
      }
      world_->getImGUI()->startFrame();
      popMarker();
      endTimer("ImGUI");

      pushMarker("Present");
      present();
      popMarker();

#if GPU_TIMERS
      context_.context->End(timer_disjoint_.Get());
#endif

      asset_manager_.deleteNotReffedAssets((float)delta_time_);
    }














    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::draw()
    {
      draw(nullptr);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::drawInstanced(const Vector<glm::mat4>& matrices)
    {
      D3D11_BUFFER_DESC instance_buffer_desc;
      D3D11_SUBRESOURCE_DATA instance_data;
      ID3D11Buffer* instance_buffer = nullptr;

	    // set up the description of the instance buffer.
      instance_buffer_desc.Usage               = D3D11_USAGE_DEFAULT;
      instance_buffer_desc.ByteWidth           = 
        sizeof(matrices.at(0u)) * (UINT)matrices.size();
      instance_buffer_desc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
      instance_buffer_desc.CPUAccessFlags      = 0;
      instance_buffer_desc.MiscFlags           = 0;
      instance_buffer_desc.StructureByteStride = 0;

	    // Give the subresource structure a pointer to the instance data.
      instance_data.pSysMem          = matrices.data();
      instance_data.SysMemPitch      = 0;
      instance_data.SysMemSlicePitch = 0;

	    // Create the instance buffer.
	    context_.device->CreateBuffer(
        &instance_buffer_desc,
        &instance_data,
        &instance_buffer
      );

      draw(instance_buffer);

      instance_buffer->Release();
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setRasterizerState(
      const platform::RasterizerState& rasterizer_state)
    {
      state_manager_.bindRasterizerState(rasterizer_state);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setBlendState(const platform::BlendState& blend_state)
    {
      state_manager_.bindBlendState(blend_state);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setDepthStencilState(
      const platform::DepthStencilState& depth_stencil_state)
    {
      state_manager_.bindDepthStencilState(depth_stencil_state);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setSamplerState(
      const platform::SamplerState& sampler_state, unsigned char slot)
    {
      state_manager_.bindSamplerState(sampler_state, slot);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::generateMipMaps(
      const asset::VioletTextureHandle& input_texture)
    {
      asset::VioletTextureHandle texture = input_texture;

      if (!texture)
        return;

      D3D11RenderTexture* tex = asset_manager_.getTexture(texture);
      tex->getTexture()->generateMips(context_.context.Get());
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::copyToScreen(const asset::VioletTextureHandle& texture)
    {
      if (!texture)
        return;

      ID3D11RasterizerState* rs_state;
      context_.context->RSGetState(&rs_state);
      static const unsigned int NUM_VIEWPORTS = 1u;
      D3D11_VIEWPORT viewports[NUM_VIEWPORTS];
      unsigned int num_viewports = NUM_VIEWPORTS;
      context_.context->RSGetViewports(&num_viewports, viewports);

      setViewports({ default_.viewport });
      setRasterizerState(platform::RasterizerState::SolidBack());
      context_.context->OMSetRenderTargets(
        1u, 
        context_.backbuffer.GetAddressOf(), 
        nullptr
      );
      setScissorRect(glm::vec4(0.0f, 0.0f, world_->getWindow()->getSize()));
      setMesh(full_screen_quad_.mesh);
      setShader(full_screen_quad_.shader);
      setSubMesh(0u);
      setTexture(texture, 0u);
      draw();

      context_.context->RSSetViewports(num_viewports, viewports);
      context_.context->RSSetState(rs_state);
    }
  
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::bindShaderPass(const platform::ShaderPass& shader_pass)
    {
      float dynamic_resolution_scale = 
        world_->getShaderVariableManager().getShaderVariable(
          Name("dynamic_resolution_scale")
        ).data.at(0);
      
      context_.context->OMSetRenderTargets(0, nullptr, nullptr);

      for (unsigned char i = 
        /*(unsigned char)shader_pass.getInputs().size()*/0u; 
        i < highest_bound_texture_; ++i)
      {
        bound_.textures[i] = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;
        context_.context->PSSetShaderResources(i, 1u, &srv);
      }

      // set shader and related stuff.
      setShader(shader_pass.getShader());
      
      // Inputs.
      for (unsigned char i = 0; i < shader_pass.getInputs().size(); ++i)
      {
        auto& input = shader_pass.getInputs().at(i);
        LMB_ASSERT(false == input.isBackBuffer(), "TODO (Hilze): Fill in");
        setTexture(input.getTexture(), i);
      }

      // Outputs.
      Vector<ID3D11RenderTargetView*> rtvs;
      ID3D11DepthStencilView* dsv = nullptr;
      Vector<glm::vec4> viewports;
      Vector<D3D11_RECT> scissor_rects;

      for (auto& output : shader_pass.getOutputs())
      {
        if (output.isBackBuffer())
        {
          viewports.push_back(default_.viewport);
          rtvs.push_back(context_.backbuffer.Get());
          scissor_rects.push_back({
            0u, 
            0u, 
            (LONG)default_.viewport.z, 
            (LONG)default_.viewport.w 
          });
        }
        else if (output.getTexture()->getLayer(0u).getFormat() == 
          TextureFormat::kR24G8)
        {
          dsv = getDepthStencilView(output.getTexture());
        }
        else
        {
          for (const auto& input : shader_pass.getInputs())
          {
            if (input.getName() == output.getName())
            {
              asset_manager_.getTexture(
                output.getTexture()
              )->getTexture()->pingPong();
            }
          }

          rtvs.push_back(getRenderTargetView(
            output.getTexture(),
            -1,
            output.getMipMap())
          );

          viewports.push_back({
            0.0f,
            0.0f,
            (float)(output.getTexture()->getLayer(0u).getWidth() 
            >> output.getMipMap()),
            (float)(output.getTexture()->getLayer(0u).getHeight() 
              >> output.getMipMap())
          });

          if (output.getTexture()->getLayer(0u).getFlags() &
            kTextureFlagDynamicScale)
            viewports.back() *= dynamic_resolution_scale;

          scissor_rects.push_back({
            0u, 
            0u,
            (LONG)viewports.back().z,
            (LONG)viewports.back().w
          });
        }
      }

      if (viewports.empty() && dsv)
      {
        viewports.push_back({
          0.0f,
          0.0f,
          (float)(
            shader_pass.getOutputs()[0u].getTexture()->getLayer(0u).getWidth()  
            >> shader_pass.getOutputs()[0u].getMipMap()),
          (float)(
            shader_pass.getOutputs()[0u].getTexture()->getLayer(0u).getHeight() 
            >> shader_pass.getOutputs()[0u].getMipMap())
        });

        if (shader_pass.getOutputs()[0u].getTexture()->getLayer(0u).getFlags() 
          & kTextureFlagDynamicScale)
          viewports.back() *= dynamic_resolution_scale;
       
        scissor_rects.push_back({ 
          0u,
          0u, 
          (LONG)viewports.back().z, 
          (LONG)viewports.back().w 
        });
      }

      // Shader.
      context_.context->RSSetScissorRects(
        (UINT)scissor_rects.size(),
        scissor_rects.data()
      );
      setViewports(viewports);
      context_.context->OMSetRenderTargets(
        (UINT)rtvs.size(), 
        rtvs.data(),
        dsv
      );
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::clearRenderTarget(
      asset::VioletTextureHandle texture, 
      const glm::vec4& colour)
    {
      if (!texture)
        return;
      
      if (texture->getLayer(0u).getFormat() == TextureFormat::kR24G8)
      {
        for (int i = 0; i < 2; ++i)
          context_.context->ClearDepthStencilView(
            getDepthStencilView(texture, i),
            D3D11_CLEAR_DEPTH /*| D3D11_CLEAR_STENCIL*/, 
            1.0f, 
            0
          );
      }
      else
      {
        float c[]{ colour.x, colour.y, colour.z, colour.w };
        for (int i = 0; i < 2; ++i)
        {
          context_.context->ClearRenderTargetView(
            getRenderTargetView(texture, i), 
            c
          );
        }
      }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setScissorRect(const glm::vec4& rect)
    {
      D3D11_RECT scissor_rect;
      scissor_rect.left   = (LONG)rect.x;
      scissor_rect.right  = (LONG)rect.x + (LONG)rect.z;
      scissor_rect.top    = (LONG)rect.y;
      scissor_rect.bottom = (LONG)rect.y + (LONG)rect.w;

      context_.context->RSSetScissorRects(1u,&scissor_rect);
    }
  
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setViewports(const Vector<glm::vec4>& rects)
    {
      Vector<D3D11_VIEWPORT> viewports(rects.size());

      for (uint8_t i = 0u; i < rects.size(); ++i)
      {
        D3D11_VIEWPORT& viewport = viewports.at(i);
        const::glm::vec4& rect = rects.at(i);
        viewport.TopLeftX = rect.x;
        viewport.TopLeftY = rect.y;
        viewport.Width    = rect.z;
        viewport.Height   = rect.w;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
      }

      context_.context->RSSetViewports(
        (UINT)viewports.size(), 
        viewports.data()
      );
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setMesh(asset::MeshHandle mesh)
    {
      mesh_ = mesh;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setSubMesh(const uint32_t& sub_mesh_idx)
    {
      sub_mesh_idx_ = sub_mesh_idx;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setShader(asset::VioletShaderHandle shader)
    {
      if (shader_ == shader)
        return;
			
			if (shader_)
				asset_manager_.getShader(shader_)->unbind();
			
			shader_ = shader;

			if (!shader)
				return;

			D3D11Shader* d3d11_shader = asset_manager_.getShader(shader);

      for (const auto& variable : shader->getQueuedShaderVariables())
        d3d11_shader->updateShaderVariable(variable);

      // Bind the shader.
      bound_.shader = d3d11_shader;
      
			if (d3d11_shader)
        d3d11_shader->bind();
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setTexture(
      asset::VioletTextureHandle texture, 
      uint8_t slot)
    {
      if (!texture)
        return;

      if (slot + 1 > highest_bound_texture_)
        highest_bound_texture_ = slot + 1;

      if (slot == 0)
        world_->getShaderVariableManager().setVariable(
          platform::ShaderVariable(
            Name("inv_texture_size"), 
            1.0f / glm::vec2(
              (float)texture->getLayer(0u).getWidth(),
              (float)texture->getLayer(0u).getHeight()
            ))
        );

      assert(slot < MAX_TEXTURE_COUNT);
      texture_contains_alpha_[slot] = texture->getLayer(0u).containsAlpha();
      textures_[slot] = 
        asset_manager_.getTexture(texture)->getTexture()->getSRV();
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::pushMarker(const String& name)
    {
#if GPU_MARKERS
      std::wstring wname(name.begin(), name.end());
      user_defined_annotation_->BeginEvent(wname.c_str());
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setMarker(const String& name)
    {
#if GPU_MARKERS
      std::wstring wname(name.begin(), name.end());
      user_defined_annotation_->SetMarker(wname.c_str());
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::popMarker()
    {
#if GPU_MARKERS
      user_defined_annotation_->EndEvent();
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::beginTimer(const String& name)
    {
#if GPU_TIMERS
      if (gpu_timers_.find(name) == gpu_timers_.end())
      {
        D3D11TimestampQuery query;

        D3D11_QUERY_DESC query_desc{};
        query_desc.Query = D3D11_QUERY_TIMESTAMP;
        
        context_.device->CreateQuery(
          &query_desc, 
          query.push[0u].ReleaseAndGetAddressOf()
        );

        context_.device->CreateQuery(
          &query_desc, 
          query.push[1u].ReleaseAndGetAddressOf()
        );
        
        context_.device->CreateQuery(
          &query_desc, 
          query.pop[0u].ReleaseAndGetAddressOf()
        );
        
        context_.device->CreateQuery(
          &query_desc, 
          query.pop[1u].ReleaseAndGetAddressOf()
        );
        
        gpu_timers_.insert(eastl::make_pair(name, query));
      }
      D3D11TimestampQuery& query = gpu_timers_[name];
      context_.context->End(query.push[gpu_timer_idx].Get());
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::endTimer(const String& name)
    {
#if GPU_TIMERS
      if (gpu_timers_.find(name) == gpu_timers_.end())
      {
        D3D11TimestampQuery query;

        D3D11_QUERY_DESC query_desc{};
        query_desc.Query = D3D11_QUERY_TIMESTAMP;

        context_.device->CreateQuery(
          &query_desc, 
          query.push[0u].ReleaseAndGetAddressOf()
        );

        context_.device->CreateQuery(
          &query_desc, 
          query.push[1u].ReleaseAndGetAddressOf()
        );
        
        context_.device->CreateQuery(
          &query_desc, 
          query.pop[0u].ReleaseAndGetAddressOf()
        );
        
        context_.device->CreateQuery(
          &query_desc, 
          query.pop[1u].ReleaseAndGetAddressOf()
        );

        gpu_timers_.insert(eastl::make_pair(name, query));
      }
      D3D11TimestampQuery& query = gpu_timers_[name];
      context_.context->End(query.pop[gpu_timer_idx].Get());
#endif
    }
    ///////////////////////////////////////////////////////////////////////////
    uint64_t D3D11Context::getTimerMicroSeconds(const String& name)
    {
#if GPU_TIMERS
      if (gpu_timers_.find(name) == gpu_timers_.end())
      {
        D3D11TimestampQuery query;

        D3D11_QUERY_DESC query_desc{};
        query_desc.Query = D3D11_QUERY_TIMESTAMP;
        
        context_.device->CreateQuery(
          &query_desc, 
          query.push[0u].ReleaseAndGetAddressOf()
        );

        context_.device->CreateQuery(
          &query_desc, 
          query.push[1u].ReleaseAndGetAddressOf()
        );
        
        context_.device->CreateQuery(
          &query_desc, 
          query.pop[0u].ReleaseAndGetAddressOf()
        );
        
        context_.device->CreateQuery(
          &query_desc, 
          query.pop[1u].ReleaseAndGetAddressOf()
        );
        
        gpu_timers_.insert(eastl::make_pair(name, query));
      }
      return gpu_timers_[name].micro_seconds;
#else
      return 0u;
#endif
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setShaderVariable(
      const platform::ShaderVariable& variable)
    {
      world_->getShaderVariableManager().setVariable(variable);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setRenderScale(const float& render_scale)
    {
      if (render_scale != render_scale_)
      {
        render_scale_ = render_scale;
        resize();
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    float D3D11Context::getRenderScale()
    {
      return render_scale_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setVSync(bool vsync)
    {
      context_.vsync = vsync;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool D3D11Context::getVSync() const
    {
      return context_.vsync;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::destroyAsset(
      foundation::SharedPointer<asset::IAsset> asset)
    {
      /*if (static_cast<D3D11Texture*>(asset->gpuData().get()))
      {
        render_target_manager_.destroy((D3D11Texture*)asset->gpuData().get());
      }*/
    }
    
    ///////////////////////////////////////////////////////////////////////////
    ID3D11RenderTargetView* D3D11Context::getRenderTargetView(
      asset::VioletTextureHandle texture, 
      int idx, 
      int mip_map)
    {
      D3D11RenderTexture* tex = asset_manager_.getTexture(texture);
      if (idx == -1)
        idx = tex->getTexture()->pingPongIdx();
      return tex->getTexture()->getRTV(idx, mip_map);
    }

    ///////////////////////////////////////////////////////////////////////////
    ID3D11DepthStencilView* D3D11Context::getDepthStencilView(
      asset::VioletTextureHandle texture, 
      int idx)
    {
      D3D11RenderTexture* tex = asset_manager_.getTexture(texture);
      if(idx == -1)
        idx = tex->getTexture()->pingPongIdx();
      return tex->getTexture()->getDSV(idx);
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::draw(ID3D11Buffer* buffer)
    {
      for (unsigned char i = 0; i < highest_bound_texture_; ++i)
      {
        if (textures_[i] != nullptr&& bound_.textures[i] != textures_[i])
          bound_.textures[i] = textures_[i];
        context_.context->PSSetShaderResources(i, 1u, &bound_.textures[i]);
      }

      D3D11Mesh* mesh = asset_manager_.getMesh(mesh_);

      // Bind the mesh.
      if (mesh != bound_.mesh || sub_mesh_idx_ != bound_.sub_mesh_idx)
      {
        state_manager_.bindTopology(mesh_->getTopology());

        bound_.mesh = mesh;
        bound_.sub_mesh_idx = sub_mesh_idx_;
        Vector<uint32_t> stages = bound_.shader->getStages();
        mesh->bind(stages, mesh_, sub_mesh_idx_);
        mesh_->updated();
      }

			if (shader_)
			{
				D3D11Shader* shader = asset_manager_.getShader(shader_);

				for (auto& buffer : shader->getPsBuffers())
					world_->getShaderVariableManager().updateBuffer(buffer);
				for (auto& buffer : shader->getVsBuffers())
					world_->getShaderVariableManager().updateBuffer(buffer);
				for (auto& buffer : shader->getGsBuffers())
					world_->getShaderVariableManager().updateBuffer(buffer);

				if (buffer == nullptr)
				{
					shader->bindBuffers();
					mesh->draw(mesh_, sub_mesh_idx_);
				}
				else
				{
					shader->bindBuffers();
					mesh->draw(mesh_, sub_mesh_idx_);
				}
			}
    }
 
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::present()
    {
      context_.swap_chain->Present(context_.vsync, 0);
    }




    ///////////////////////////////////////////////////////////////////////////
#include <thread>
    std::mutex k_flush_mutex;
    static Vector<IRenderAction*> k_flush_actions;
  
    ///////////////////////////////////////////////////////////////////////////
    void flush(D3D11Context* context)
    {
      while (true)
      {
        bool can_continue = false;
        while (can_continue)
        {
          k_flush_mutex.lock();
          can_continue = !k_flush_actions.empty();
          k_flush_mutex.unlock();
          if (!can_continue)
            std::this_thread::sleep_for(std::chrono::microseconds(10u));
        }
        k_flush_mutex.lock();
        for (uint32_t i = 0u; i < k_flush_actions.size(); ++i)
        {
          IRenderAction* action = k_flush_actions[i];
          action->execute(context);
          action->~IRenderAction();
        }
        k_flush_actions.clear();
        k_flush_mutex.unlock();
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::flush(Vector<IRenderAction*> actions)
    {
      static std::thread thead;
      if (!thead.joinable())
      {
        thead = std::thread(lambda::windows::flush, this);
      }
      bool can_continue = false;
      while (can_continue)
      {
        k_flush_mutex.lock();
        can_continue = k_flush_actions.empty();
        k_flush_mutex.unlock();
        if (!can_continue)
          std::this_thread::sleep_for(std::chrono::microseconds(10u));
      }
      k_flush_mutex.lock();
      k_flush_actions = eastl::move(actions);
      k_flush_mutex.unlock();
    }

    ///////////////////////////////////////////////////////////////////////////
    D3D11Context::D3D11AssetManager::~D3D11AssetManager()
    {
      for (const auto& it : textures_)
        removeTexture(it.first);
    }

    ///////////////////////////////////////////////////////////////////////////
    D3D11RenderTexture* D3D11Context::D3D11AssetManager::getTexture(
      asset::VioletTextureHandle texture)
    {
			// TODO (Hilze): Have some sort of check to make sure that the size and format still match.

      auto it = textures_.find(texture.getHash());
      
      if (it == textures_.end())
      {
        D3D11RenderTexture* d3d11_texture = 
          static_cast<D3D11RenderTexture*>(
            d3d11_context_->allocRenderTexture(texture)
          );
        textures_.insert(
          eastl::make_pair(
            texture.getHash(), 
            RefType<D3D11RenderTexture>{ d3d11_texture, 0u, 0.0f }
          )
        );
        it = textures_.find(texture.getHash());
				texture->clean();
      }
			else if (texture->isDirty())
			{
				it->second.t->getTexture()->update(
					texture,
					device_,
					context_
				);
			}
        
      it->second.ref++;
      return it->second.t;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::removeTexture(
      asset::VioletTextureHandle texture)
    {
      removeTexture(texture.getHash());
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::removeTexture(size_t texture)
    {
      auto it = textures_.find(texture);
      if (it != textures_.end())
      {
        platform::IRenderTexture* tex = it->second.t;
        d3d11_context_->freeRenderTexture(tex);
        textures_.erase(it);
      }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    D3D11Mesh* D3D11Context::D3D11AssetManager::getMesh(asset::MeshHandle mesh)
    {
      auto it = meshes_.find(mesh.id);

      if (it == meshes_.end())
      {
        D3D11Mesh* d3d11_mesh = 
          foundation::Memory::construct<D3D11Mesh>(d3d11_context_);
        
        meshes_.insert(
          eastl::make_pair(
            mesh.id, 
            RefType<D3D11Mesh>{ d3d11_mesh, 0u, 0.0f }
          )
        );

        it = meshes_.find(mesh.id);
      }

      it->second.ref++;
      return it->second.t;
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::removeMesh(asset::MeshHandle mesh)
    {
      removeMesh(mesh.id);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::removeMesh(size_t mesh)
    {
      auto it = meshes_.find(mesh);
      if (it != meshes_.end())
      {
        foundation::Memory::destruct(it->second.t);
        meshes_.erase(it);
      }
    }

		///////////////////////////////////////////////////////////////////////////
		D3D11Shader* D3D11Context::D3D11AssetManager::getShader(
			asset::VioletShaderHandle shader)
		{
			auto it = shaders_.find(shader.getHash());

			if (it == shaders_.end())
			{
				D3D11Shader* d3d11_shader =
					foundation::Memory::construct<D3D11Shader>(shader, d3d11_context_);
				shaders_.insert(
					eastl::make_pair(
						shader.getHash(),
						RefType<D3D11Shader>{ d3d11_shader, 0u, 0.0f }
				)
				);
				it = shaders_.find(shader.getHash());
			}

			it->second.ref++;
			return it->second.t;
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::D3D11AssetManager::removeShader(
			asset::VioletShaderHandle shader)
		{
			removeShader(shader.getHash());
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::D3D11AssetManager::removeShader(size_t shader)
		{
			auto it = shaders_.find(shader);
			if (it != shaders_.end())
			{
				D3D11Shader* shad = it->second.t;
				foundation::Memory::destruct(shad);
				shaders_.erase(it);
			}
		}
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::setD3D11Context(
      D3D11Context* context)
    {
      d3d11_context_ = context;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::setDevice(ID3D11Device* device)
    {
      device_ = device;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::setContext(
      ID3D11DeviceContext* context)
    {
      context_ = context;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::deleteNotReffedAssets(
      const float& dt)
    {
      // Textures.
      {
        Vector<uint64_t> marked_for_delete;
        for (auto& it : textures_)
        {
          if (it.second.ref == 0u)
          {
            it.second.time += dt;
            if (it.second.time >= kMaxTime)
              marked_for_delete.push_back(it.first);
          }
          else
          {
            it.second.time = 0.0f;
            it.second.ref = 0u;
          }
        }
        for (const auto& to_delete : marked_for_delete)
          removeTexture(to_delete);
      }
      // Meshes.
      {
        Vector<uint64_t> marked_for_delete;
        for (auto& it : meshes_)
        {
          if (it.second.ref == 0u)
          {
            it.second.time += dt;
            if (it.second.time >= kMaxTime)
              marked_for_delete.push_back(it.first);
          }
          else
          {
            it.second.time = 0.0f;
            it.second.ref = 0u;
          }
        }
        for (const auto& to_delete : marked_for_delete)
          removeMesh(to_delete);
      }
			// Shaders.
			{
				Vector<uint64_t> marked_for_delete;
				for (auto& it : shaders_)
				{
					if (it.second.ref == 0u)
					{
						it.second.time += dt;
						if (it.second.time >= kMaxTime)
							marked_for_delete.push_back(it.first);
					}
					else
					{
						it.second.time = 0.0f;
						it.second.ref = 0u;
					}
				}
				for (const auto& to_delete : marked_for_delete)
					removeShader(to_delete);
			}
    }
  }
}