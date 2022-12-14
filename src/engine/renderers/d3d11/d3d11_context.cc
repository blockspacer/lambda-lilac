#include "d3d11_context.h"
#include <assert.h>
#include <d3d11.h>
#include "assets/shader.h"
#include "d3d11_shader.h"
#include "d3d11_mesh.h"
#include "d3d11_texture.h"
#include "utils/decompose_matrix.h"
#include <utils/console.h>
#include "interfaces/iimgui.h"
#include <utils/utilities.h>
#include "d3d11_renderer.h"
#include <memory/frame_heap.h>
#include <thread>
#include <atomic>
#include <platform/scene.h>
#include <gui/gui.h>

namespace lambda
{
	namespace windows
	{
		///////////////////////////////////////////////////////////////////////////
		D3D11RenderBuffer::D3D11RenderBuffer(
			uint32_t size,
			uint32_t flags,
			ID3D11Buffer* buffer,
			D3D11Context* context)
			: size_(size)
			, flags_(flags)
			, buffer_(buffer)
			, context_(context)
			, changed_(true)
			, locked_(false)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void* D3D11RenderBuffer::lock()
		{
			LMB_ASSERT(flags_ & kFlagDynamic, "D3D11 CONTEXT: Tried to lock lock a non dynamic render buffer");
			LMB_ASSERT(!locked_, "D3D11 CONTEXT: Tried to lock a locked render buffer");
			locked_ = true;

			LMB_ASSERT(context_, "D3D11 CONTEXT: No context was specified");

			HRESULT result;
			D3D11_MAPPED_SUBRESOURCE resource;
			result = context_->getD3D11Context()->Map(buffer_, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &resource);
			LMB_ASSERT(SUCCEEDED(result), "D3D11 CONTEXT: Could not map render buffer | %llu", result);
			return resource.pData;
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11RenderBuffer::unlock()
		{
			LMB_ASSERT(flags_ & kFlagDynamic, "D3D11 CONTEXT: Tried to lock unlock a non dynamic render buffer");
			LMB_ASSERT(locked_, "D3D11 CONTEXT: Tried to unlock a not locked render buffer");
			locked_ = false;

			LMB_ASSERT(context_, "D3D11 CONTEXT: No context was specified");

			context_->getD3D11Context()->Unmap(buffer_, 0u);
			changed_ = true;
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
			LMB_ASSERT(!locked_, "D3D11 CONTEXT: Tried to get the platform buffer of a locked render buffer");
			return buffer_;
		}

		///////////////////////////////////////////////////////////////////////////
		bool D3D11RenderBuffer::getChanged() const
		{
			return changed_;
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11RenderBuffer::setChanged(bool changed)
		{
			changed_ = changed;
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
			D3D11Context* context)
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
			//LMB_ASSERT(flags_ & IRenderTexture::kFlagDynamic, "TODO (Hilze): Fill in");

			LMB_ASSERT(context_, "D3D11 CONTEXT: No context was specified");

			HRESULT result;
			D3D11_MAPPED_SUBRESOURCE resource;
			result = context_->getD3D11Context()->Map(
				texture_->getTexture(),
				level,
				D3D11_MAP_WRITE_DISCARD,
				0u,
				&resource
			);
			LMB_ASSERT(SUCCEEDED(result), "D3D11 CONTEXT: Could not map render texture | %llu", result);

			return resource.pData;
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11RenderTexture::unlock(uint32_t level)
		{
			LMB_ASSERT(context_, "D3D11 CONTEXT: No context was specified");

			//LMB_ASSERT(flags_ & IRenderTexture::kFlagDynamic, "TODO (Hilze): Fill in");

			context_->getD3D11Context()->Unmap(texture_->getTexture(), level);
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
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			const bool is_dynamic =
				(flags & platform::IRenderBuffer::kFlagDynamic) ? true : false;
			const bool is_staging =
				(flags & platform::IRenderBuffer::kFlagStaging) ? true : false;
			const bool is_immutable =
				(flags & platform::IRenderBuffer::kFlagImmutable) ? true : false;
			const bool is_vertex =
				(flags & platform::IRenderBuffer::kFlagVertex) ? true : false;
			const bool is_index =
				(flags & platform::IRenderBuffer::kFlagIndex) ? true : false;
			const bool is_constant =
				(flags & platform::IRenderBuffer::kFlagConstant) ? true : false;

			LMB_ASSERT(is_vertex || is_index || is_constant, "D3D11 CONTEXT: Tried to create a buffer of unknown type");

			if (is_constant)
				size = (size + 15) / 16 * 16;

			D3D11_BUFFER_DESC buffer_desc{};
			buffer_desc.ByteWidth = size;
			buffer_desc.Usage = is_dynamic ? D3D11_USAGE_DYNAMIC :
				(is_staging ? D3D11_USAGE_STAGING :
				(is_immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT));
			buffer_desc.BindFlags = is_vertex ? D3D11_BIND_VERTEX_BUFFER :
				(is_index ? D3D11_BIND_INDEX_BUFFER :
				(is_constant ? D3D11_BIND_CONSTANT_BUFFER : 0u));
			buffer_desc.CPUAccessFlags = is_dynamic ? D3D11_CPU_ACCESS_WRITE :
				(is_staging ? D3D11_CPU_ACCESS_WRITE : 0u);
			buffer_desc.MiscFlags = 0u;
			buffer_desc.StructureByteStride = 0u;

			D3D11_SUBRESOURCE_DATA subresource{};
			subresource.pSysMem = data;
			
			LMB_ASSERT(context_.device, "D3D11 CONTEXT: No device was specified");

			ID3D11Buffer* buffer;
			context_.device->CreateBuffer(
				&buffer_desc,
				data ? &subresource : nullptr,
				&buffer
			);

			if (is_vertex)
				memory_stats_.vertex += size;
			if (is_index)
				memory_stats_.index += size;
			if (is_constant)
				memory_stats_.constant += size;

			D3D11RenderBuffer* constant_buffer = foundation::Memory::construct<D3D11RenderBuffer>(
				size,
				flags,
				buffer,
				this
				);

			if ((flags & platform::IRenderBuffer::kFlagTransient))
				transient_render_buffers_.push_back(constant_buffer);

			return constant_buffer;
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::freeRenderBuffer(platform::IRenderBuffer*& buffer)
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			const bool is_vertex =
				(buffer->getFlags() & platform::IRenderBuffer::kFlagVertex)
				? true : false;
			const bool is_index =
				(buffer->getFlags() & platform::IRenderBuffer::kFlagIndex)
				? true : false;
			const bool is_constant =
				(buffer->getFlags() & platform::IRenderBuffer::kFlagConstant)
				? true : false;
			
			LMB_ASSERT(is_vertex || is_index || is_constant, "D3D11 CONTEXT: Tried to free a buffer of unknown type");

			if (is_vertex)
				memory_stats_.vertex -= buffer->getSize();
			if (is_index)
				memory_stats_.index -= buffer->getSize();
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
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

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
						getD3D11Context()
						),
					this
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
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

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
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");
			LMB_ASSERT(context_.context.Get(), "D3D11 CONTEXT: No context was specified");
			return context_.context.Get();
		}

		///////////////////////////////////////////////////////////////////////////
		ID3D11Device* D3D11Context::getD3D11Device() const
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");
			LMB_ASSERT(context_.device.Get(), "D3D11 CONTEXT: No device was specified");
			return context_.device.Get();
		}

		///////////////////////////////////////////////////////////////////////////
		D3D11Context::D3D11Context()
			: render_scale_(1.0f)
			, vsync_(false)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		D3D11Context::~D3D11Context()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setWindow(platform::IWindow* window)
		{
			LMB_ASSERT(window, "D3D11 CONTEXT: No window was specified");

			memset(&state_, 0, sizeof(state_));
			state_.sub_mesh = UINT32_MAX;
			memset(&dx_state_, 0, sizeof(dx_state_));
			invalidateAll();

			asset_manager_ = D3D11AssetManager();

			// Create device and context.
			{
				HRESULT result;
				DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
				IDXGIAdapter* adapter = nullptr;

				swap_chain_desc.BufferDesc.Width = window->getSize().x;
				swap_chain_desc.BufferDesc.Height = window->getSize().y;
				swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
				swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
				swap_chain_desc.BufferDesc.ScanlineOrdering =
					DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

				swap_chain_desc.BufferCount = 1;
				swap_chain_desc.OutputWindow = (HWND)window->getWindow();
				swap_chain_desc.SampleDesc.Count = 1;
				swap_chain_desc.SampleDesc.Quality = 0;
				swap_chain_desc.Windowed = true;
				swap_chain_desc.BufferUsage =
					DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
				swap_chain_desc.SwapEffect =
					DXGI_SWAP_EFFECT_DISCARD;// DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
				swap_chain_desc.Flags =
					DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

#ifdef ENUM_ADAPTERS
				IDXGIFactory* factory = nullptr;
				CreateDXGIFactory(IID_PPV_ARGS(&factory));

				int idx = -1;
				IDXGIAdapter* selected_adapter = 0;
				size_t dedicated_memory = 0;
				while (SUCCEEDED(factory->EnumAdapters(++idx, &adapter)))
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
#if VIOLET_DEBUG
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
						foundation::Info("D3D11 CONTEXT: Using feature level " +
							toString(supported_level) + "\n");
						foundation::Info("D3D11 CONTEXT: Using driver type " +
							toString(i) + "\n");
						break;
					}
				}

				LMB_ASSERT(SUCCEEDED(result), "D3D11 CONTEXT: Could not create device and swapchain | %llu", result);
			}

#if GPU_MARKERS
			HRESULT result = context_.context->QueryInterface(
				IID_PPV_ARGS(user_defined_annotation_.ReleaseAndGetAddressOf())
			);

			LMB_ASSERT(SUCCEEDED(result), "D3D11 CONTEXT: Could not get user defined annotations | %llu", result);
#endif

#if GPU_TIMERS
			D3D11_QUERY_DESC query_desc{};
			query_desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
			context_.device->CreateQuery(
				&query_desc,
				timer_disjoint_.ReleaseAndGetAddressOf()
			);
#endif

			state_manager_.initialize(this);
			asset_manager_.setD3D11Context(this);

			resize();
			setVSync(vsync_);
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setOverrideScene(scene::Scene* scene)
		{
			override_scene_ = scene;
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::initialize(scene::Scene& scene)
		{
			scene_ = &scene;

			full_screen_quad_.mesh = asset::MeshManager::getInstance()->create(Name("__full_screen_quad__"), asset::Mesh::createScreenQuad());
			full_screen_quad_.shader = asset::ShaderManager::getInstance()->get(Name("resources/shaders/full_screen_quad.fx"));

			memset(&state_, 0, sizeof(state_));
			state_.sub_mesh = UINT32_MAX;
			memset(&dx_state_, 0, sizeof(dx_state_));
			invalidateAll();
		}
		void D3D11Context::deinitialize()
		{
			state_manager_.deinitialize();
			asset_manager_.deleteAllAssets();
			memset(&state_, 0, sizeof(state_));
			context_.backbuffer = nullptr;
			context_.swap_chain = nullptr;
			context_.context = nullptr;
			context_.device = nullptr;
		}
		void D3D11Context::resize()
		{
			queued_resize_ = true;
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::update(const double& delta_time)
		{
			queued_update_ = true;
			queued_delta_time_ = (float)delta_time;
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::startFrame()
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			setSamplerState(platform::SamplerState::PointClamp(), 6u);
			setSamplerState(platform::SamplerState::LinearClamp(), 7u);
			setSamplerState(platform::SamplerState::AnisotrophicClamp(), 8u);
			setSamplerState(platform::SamplerState::PointBorder(), 9u);
			setSamplerState(platform::SamplerState::LinearBorder(), 10u);
			setSamplerState(platform::SamplerState::AnisotrophicBorder(), 11u);
			setSamplerState(platform::SamplerState::PointWrap(), 12u);
			setSamplerState(platform::SamplerState::LinearWrap(), 13u);
			setSamplerState(platform::SamplerState::AnisotrophicWrap(), 14u);

			if (queued_update_)
			{
				queued_update_ = false;
				delta_time_ = queued_delta_time_;
				total_time_ += delta_time_;
			}

			if (queued_resize_)
			{
				queued_resize_ = false;
				resizeImpl();
			}

			// Clear everything.
			beginTimer("Clear Everything");
			pushMarker("Clear Everything");
			glm::vec4 colour(0.0f);
			for (const auto& target : getScene()->post_process_manager->getAllTargets())
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

			memset(&state_, 0, sizeof(state_));
			state_.sub_mesh = UINT32_MAX;
			memset(&dx_state_, 0, sizeof(dx_state_));
			invalidateAll();
			state_manager_.reset();

			if (!cbs_.drs) cbs_.drs = (D3D11RenderBuffer*)allocRenderBuffer(sizeof(float), platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagDynamic, nullptr);
			float drs_data[] = { dynamic_resolution_scale_ };
			memcpy(cbs_.drs->lock(), drs_data, sizeof(drs_data));
			cbs_.drs->unlock();
			setConstantBuffer(cbs_.drs, cbDynamicResolutionIdx);

			if (!cbs_.per_frame) cbs_.per_frame = (D3D11RenderBuffer*)allocRenderBuffer(sizeof(float) * 5, platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagDynamic, nullptr);
			float per_frame_data[] = { screen_size_.x, screen_size_.y, delta_time_, total_time_, (float)frame_index_++ };
			memcpy(cbs_.per_frame->lock(), per_frame_data, sizeof(per_frame_data));
			cbs_.per_frame->unlock();
			setConstantBuffer(cbs_.per_frame, cbPerFrameIdx);

			// Handle markers.
#if GPU_TIMERS
			while (context_.context->GetData(timer_disjoint_.Get(), NULL, 0, 0) == S_FALSE)
				std::this_thread::sleep_for(std::chrono::microseconds(1));

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
			pushMarker("Present");
			present();
			popMarker();
				
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

#if GPU_TIMERS
			context_.context->End(timer_disjoint_.Get());
#endif
			asset_manager_.deleteNotReffedAssets((float)delta_time_);

			for (D3D11RenderBuffer* buffer : transient_render_buffers_)
				freeRenderBuffer((platform::IRenderBuffer*&)buffer);
			transient_render_buffers_.clear();

			foundation::GetFrameHeap()->update();
		}

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::draw(uint32_t instance_count)
    {
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");
			LMB_ASSERT(instance_count, "D3D11 CONTEXT: Tried to render zero instances");

			D3D11Shader* shader = dx_state_.shader;
			D3D11Mesh*   mesh = dx_state_.mesh;

			LMB_ASSERT(shader, "D3D11 CONTEXT: There was no valid shader bound");
			LMB_ASSERT(mesh, "D3D11 CONTEXT: There was no valid mesh bound");

			if (isDirty(DirtyStates::kShader))
			{
				shader->bind();
			}

			if (isDirty(DirtyStates::kViewports))
			{
				getD3D11Context()->RSSetViewports(state_.num_viewports, dx_state_.viewports);
			}

			if (isDirty(DirtyStates::kScissorRects))
			{
				getD3D11Context()->RSSetScissorRects(state_.num_scissor_rects, dx_state_.scissor_rects);
			}

			if (isDirty(DirtyStates::kRenderTargets))
			{
				getD3D11Context()->OMSetRenderTargets(state_.num_render_targets, dx_state_.render_targets, dx_state_.depth_target);
			}

			// TODO (Hilze): Add support for isDirty here. Should probably have a dirty state per stage.
			if (isDirty(DirtyStates::kTextures))
			{
				for (uint16_t i = 0; i < 16 && state_.dirty_textures != 0; ++i)
				{
					if ((state_.dirty_textures & (1 << i)))
					{
						getD3D11Context()->PSSetShaderResources(i, 1, &dx_state_.textures[i]);
						state_.dirty_textures &= ~(1 << i);
					}
				}
			}

			if (shader)
			{
				for (const auto& buffer : shader->getBuffers())
				{
					bool changed = (state_.dirty_constant_buffers[(int)buffer.stage] & (1 << buffer.slot)) ? true : false;

					if (!changed && (state_.constant_buffers[buffer.slot] && ((D3D11RenderBuffer*)state_.constant_buffers[buffer.slot])->getChanged()))
					{
						((D3D11RenderBuffer*)state_.constant_buffers[buffer.slot])->setChanged(false);
						changed = true;
					}

					if (changed)
					{
						state_.dirty_constant_buffers[(int)buffer.stage] &= ~(1 << buffer.slot);

						switch (buffer.stage)
						{
						case ShaderStages::kVertex:
							getD3D11Context()->VSSetConstantBuffers(
								(UINT)buffer.slot,
								1u,
								&dx_state_.constant_buffers[buffer.slot]
							);
							break;
						case ShaderStages::kPixel:
							getD3D11Context()->PSSetConstantBuffers(
								(UINT)buffer.slot,
								1u,
								&dx_state_.constant_buffers[buffer.slot]
							);
							break;
						case ShaderStages::kGeometry:
							getD3D11Context()->GSSetConstantBuffers(
								(UINT)buffer.slot,
								1u,
								&dx_state_.constant_buffers[buffer.slot]
							);
							break;
						}
					}
				}
			}

			if (isDirty(DirtyStates::kMesh) || isDirty(DirtyStates::kShader))
			{
				state_manager_.bindTopology(state_.mesh->getTopology());

				Vector<uint32_t> stages = shader->getStages();
				mesh->bind(stages, state_.mesh, state_.sub_mesh);
				state_.mesh->updated();
			}

			mesh->draw(state_.mesh, state_.sub_mesh, instance_count);

			cleanAll();
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setRasterizerState(
      const platform::RasterizerState& rasterizer_state)
    {
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

      state_manager_.bindRasterizerState(rasterizer_state);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setBlendState(const platform::BlendState& blend_state)
    {
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

      state_manager_.bindBlendState(blend_state);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setDepthStencilState(
      const platform::DepthStencilState& depth_stencil_state)
    {
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

      state_manager_.bindDepthStencilState(depth_stencil_state);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setSamplerState(
      const platform::SamplerState& sampler_state, unsigned char slot)
    {
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

      state_manager_.bindSamplerState(sampler_state, slot);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::generateMipMaps(
      const asset::VioletTextureHandle& input_texture)
    {
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

      asset::VioletTextureHandle texture = input_texture;

      if (!texture)
        return;

      D3D11RenderTexture* tex = asset_manager_.getTexture(texture);
      tex->getTexture()->generateMips(getD3D11Context());
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::copyToScreen(const asset::VioletTextureHandle& texture)
    {
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

      if (!texture)
        return;

	  Vector<glm::vec4> scissor_rects(state_.scissor_rects, state_.scissor_rects + state_.num_scissor_rects);
	  Vector<glm::vec4> viewports(state_.viewports, state_.viewports + state_.num_viewports);
	  Vector<asset::VioletTextureHandle> render_targets(state_.render_targets, state_.render_targets + state_.num_render_targets);
	  Vector<asset::VioletTextureHandle> textures(state_.textures, state_.textures + MAX_TEXTURE_COUNT);
	  asset::VioletShaderHandle  shader   = state_.shader;
	  asset::VioletMeshHandle    mesh     = state_.mesh;
	  uint32_t                   sub_mesh = state_.sub_mesh;
	  glm::vec4                  ud       = cbs_.user_data[0];

	  ID3D11ShaderResourceView* srv = nullptr;
	  for (uint32_t i = 0; i < MAX_TEXTURE_COUNT; ++i)
	  {
		  setTexture(asset::VioletTextureHandle(), i);
		  getD3D11Context()->PSSetShaderResources(i, 1, &srv);
	  }

      setViewports({ default_.viewport });
	  setScissorRects({ { 0.0f, 0.0f, getScene()->window->getSize().x, getScene()->window->getSize().y } });
	  setMesh(full_screen_quad_.mesh);
	  setShader(full_screen_quad_.shader);
	  setSubMesh(0u);
	  setTexture(texture, 0u);
	  setRenderTargets({ context_.backbuffer.Get() }, nullptr);
	  setRasterizerState(platform::RasterizerState::SolidBack());

	  bool src_use_drs = (texture->getLayer(0u).getFlags() & kTextureFlagDynamicScale) != 0u;
	  setUserData(glm::vec4(src_use_drs ? dynamic_resolution_scale_ : 1.0f, 0.0f, 0.0f, 0.0f), 0);

	  draw();

	  setUserData(ud, 0);
	  setViewports(viewports);
	  setScissorRects(scissor_rects);
	  setMesh(mesh);
	  setShader(shader);
	  setSubMesh(sub_mesh);
	  for (uint32_t i = 0; i < MAX_TEXTURE_COUNT; ++i)
		setTexture(textures[i], i);
	  setRenderTargets(render_targets, state_.depth_target);
    }
  

	///////////////////////////////////////////////////////////////////////////
	void D3D11Context::copyToTexture(const asset::VioletTextureHandle& src,
		const asset::VioletTextureHandle& dst)
	{
		LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

		if (!src || !dst)
			return;

		D3D11_BOX box = {};
		box.left = 0u;
		box.top = 0u;
		box.front = 0u;
		box.right = (UINT)src->getLayer(0u).getWidth();
		box.bottom = (UINT)src->getLayer(0u).getHeight();
		box.back = 1u;

		auto src_tex = asset_manager_.getTexture(src)->getTexture();
		auto dst_tex = asset_manager_.getTexture(dst)->getTexture();
		getD3D11Context()->CopySubresourceRegion(
			dst_tex->getTexture(),
			0u,
			0u,
			0u,
			0u,
			src_tex->getTexture(),
			0u,
			&box
		);
	}

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::bindShaderPass(const platform::ShaderPass& shader_pass)
    {
      LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

      // set shader and related stuff.
      setShader(shader_pass.getShader());
      
      // Inputs.
	  for (uint32_t i = 0u; i < MAX_TEXTURE_COUNT; ++i)
	  {
		  if (state_.textures[i])
		  {
			  setTexture(asset::VioletTextureHandle(), i);
			  ID3D11ShaderResourceView* srv = nullptr;
			  getD3D11Context()->PSSetShaderResources(i, 1, &srv);
		  }
	  }
      
	  for (uint32_t i = 0; i < shader_pass.getInputs().size(); ++i)
      {
        auto& input = shader_pass.getInputs().at(i);
        LMB_ASSERT(!input.isBackBuffer(), "D3D11 CONTEXT: Tried to bind the backbuffer as input");
        setTexture(input.getTexture(), i);
      }

      // Outputs.
      Vector<ID3D11RenderTargetView*> rtvs;
      ID3D11DepthStencilView* dsv = nullptr;
      Vector<glm::vec4> viewports;
      Vector<glm::vec4> scissor_rects;

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
        else if (output.getTexture()->getLayer(0u).getFormat() == TextureFormat::kR24G8 || 
          output.getTexture()->getLayer(0u).getFormat() ==  TextureFormat::kD32)
        {
          dsv = getDSV(
            output.getTexture(),
            output.getLayer(),
            output.getMipMap()
		  );
        }
        else
        {
          for (const auto& input : shader_pass.getInputs())
            LMB_ASSERT(input.getName() != output.getName(), "D3D11 RENDERER: Tried to bind \"%s\" as both input and output in pass \"%s\"", input.getName().getName().c_str(), shader_pass.getName().getName().c_str());

          rtvs.push_back(getRTV(
            output.getTexture(),
            output.getLayer(),
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

          if ((output.getTexture()->getLayer(0u).getFlags() & kTextureFlagDynamicScale) != 0)
            viewports.back() *= dynamic_resolution_scale_;

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
          (float)(shader_pass.getOutputs()[0u].getTexture()->getLayer(0u).getWidth()  >> shader_pass.getOutputs()[0u].getMipMap()),
          (float)(shader_pass.getOutputs()[0u].getTexture()->getLayer(0u).getHeight() >> shader_pass.getOutputs()[0u].getMipMap())
        });

        if ((shader_pass.getOutputs()[0u].getTexture()->getLayer(0u).getFlags() & kTextureFlagDynamicScale) != 0)
          viewports.back() *= dynamic_resolution_scale_;
       
        scissor_rects.push_back({ 
          0u,
          0u, 
          (LONG)viewports.back().z, 
          (LONG)viewports.back().w 
        });
      }

      // Shader.
	  setScissorRects(scissor_rects);
      setViewports(viewports);
	  setRenderTargets(rtvs, dsv);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::clearRenderTarget(
      asset::VioletTextureHandle texture, 
      const glm::vec4& colour)
    {
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

      if (!texture)
        return;
      
      if (texture->getLayer(0u).getFormat() == TextureFormat::kR24G8 ||
		  texture->getLayer(0u).getFormat() == TextureFormat::kD32)
      {
			for (int l = 0; l < (int)texture->getLayerCount(); ++l)
			{
				getD3D11Context()->ClearDepthStencilView(
					getDSV(texture, l, 0),
					D3D11_CLEAR_DEPTH /*| D3D11_CLEAR_STENCIL*/,
					1.0f,
					0
				);
			}
      }
      else
      {
        float c[]{ colour.x, colour.y, colour.z, colour.w };
		for (int l = 0; l < (int)texture->getLayerCount(); ++l)
		{
			getD3D11Context()->ClearRenderTargetView(
				getRTV(texture, l, 0),
				c
			);
		}
      }
    }
    
    ///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setScissorRects(const Vector<glm::vec4>& rects)
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");
			LMB_ASSERT(rects.size(), "D3D11 CONTEXT: Tried to set zero scissor rects");

			bool is_dirty = rects.size() != state_.num_scissor_rects;

			for (uint8_t i = 0; i < rects.size() && !is_dirty; ++i)
				is_dirty = state_.scissor_rects[i] != rects[i];

			if (is_dirty)
			{
				state_.num_scissor_rects = (uint8_t)rects.size();

				for (uint8_t i = 0; i < state_.num_scissor_rects; ++i)
				{
					state_.scissor_rects[i] = rects[i];
					dx_state_.scissor_rects[i].left = (LONG)state_.scissor_rects[i].x;
					dx_state_.scissor_rects[i].right = (LONG)state_.scissor_rects[i].x + (LONG)state_.scissor_rects[i].z;
					dx_state_.scissor_rects[i].top = (LONG)state_.scissor_rects[i].y;
					dx_state_.scissor_rects[i].bottom = (LONG)state_.scissor_rects[i].y + (LONG)state_.scissor_rects[i].w;
				}
				makeDirty(DirtyStates::kScissorRects);
			}
		}
  
    ///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setViewports(const Vector<glm::vec4>& rects)
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");
			LMB_ASSERT(rects.size(), "D3D11 CONTEXT: Tried to set zero viewports");

			bool is_dirty = rects.size() != state_.num_viewports;

			for (uint8_t i = 0; i < rects.size() && !is_dirty; ++i)
				is_dirty = state_.viewports[i] != rects[i];

			if (is_dirty)
			{
				state_.num_viewports = (uint8_t)rects.size();

				for (uint8_t i = 0; i < state_.num_viewports; ++i)
				{
					state_.viewports[i] = rects[i];
					dx_state_.viewports[i].TopLeftX = state_.viewports[i].x;
					dx_state_.viewports[i].TopLeftY = state_.viewports[i].y;
					dx_state_.viewports[i].Width = state_.viewports[i].z;
					dx_state_.viewports[i].Height = state_.viewports[i].w;
					dx_state_.viewports[i].MinDepth = 0.0f;
					dx_state_.viewports[i].MaxDepth = 1.0f;
				}
				makeDirty(DirtyStates::kViewports);
			}
		}

    ///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setMesh(asset::VioletMeshHandle mesh)
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			if (state_.mesh == mesh)
				return;

			state_.mesh = mesh;
			dx_state_.mesh = asset_manager_.getMesh(mesh);
			makeDirty(DirtyStates::kMesh);
		}

    ///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setSubMesh(const uint32_t& sub_mesh_idx)
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			if (sub_mesh_idx == state_.sub_mesh)
				return;

			state_.sub_mesh = sub_mesh_idx;
			makeDirty(DirtyStates::kMesh);
		}

    ///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setShader(asset::VioletShaderHandle shader)
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			if (state_.shader == shader)
				return;

			state_.shader = shader;
			dx_state_.shader = asset_manager_.getShader(shader);
			makeDirty(DirtyStates::kShader);
		}
   
    ///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setTexture(
			asset::VioletTextureHandle texture,
			uint8_t slot)
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			//if (texture == state_.textures[slot])
			//	return;

			LMB_ASSERT(slot < MAX_TEXTURE_COUNT, "D3D11 CONTEXT: Tried to bind a texture out of range");

			if (slot == 0)
			{
				glm::vec2 val(1.0f);
				if (texture)
					val = 1.0f / glm::vec2((float)texture->getLayer(0u).getWidth(), (float)texture->getLayer(0u).getHeight());
				
				setUserData(glm::vec4(val.x, val.y, 0.0f, 0.0f), 15);
			}

			state_.textures[slot] = texture;
			dx_state_.textures[slot] = (state_.textures[slot]) ? asset_manager_.getTexture(state_.textures[slot])->getTexture()->getSRV() : nullptr;
			makeDirty(DirtyStates::kTextures);
			state_.dirty_textures |= 1ull << slot;
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setConstantBuffer(platform::IRenderBuffer* constant_buffer, uint8_t slot)
		{
			LMB_ASSERT(slot < MAX_CONSTANT_BUFFER_COUNT, "D3D11 CONTEXT: Tried to bind a constant buffer outside of range");

			if (state_.constant_buffers[slot] == constant_buffer)
				return;

			state_.constant_buffers[slot] = constant_buffer;
			dx_state_.constant_buffers[slot] = constant_buffer ? ((D3D11RenderBuffer*)constant_buffer)->getBuffer() : nullptr;
			makeDirty(DirtyStates::kConstantBuffers);

			for (int i = 0; i < (int)ShaderStages::kCount; ++i)
				state_.dirty_constant_buffers[i] |= 1ull << slot;
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setUserData(glm::vec4 user_data, uint8_t slot)
		{
			LMB_ASSERT(slot < MAX_USER_DATA_COUNT, "D3D11 CONTEXT: Tried to bind user data outside of range");

			if (cbs_.user_data[slot] != user_data)
			{
				cbs_.user_data[slot] = user_data;

				if (!cbs_.cb_user_data)
					cbs_.cb_user_data = (D3D11RenderBuffer*)allocRenderBuffer(sizeof(cbs_.user_data), platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagDynamic, nullptr);
				
				memcpy(cbs_.cb_user_data->lock(), cbs_.user_data, sizeof(cbs_.user_data));
				cbs_.cb_user_data->unlock();
				
				setConstantBuffer(cbs_.cb_user_data, cbUserDataIdx);
			}
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setRenderTargets(
			Vector<asset::VioletTextureHandle> render_targets,
			asset::VioletTextureHandle depth_buffer)
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			bool is_dirty = (state_.num_render_targets != render_targets.size()) || (state_.depth_target != depth_buffer);

			for (uint8_t i = 0; i < (uint8_t)render_targets.size() && !is_dirty; ++i)
				is_dirty = state_.render_targets[i] != render_targets[i];

			if (is_dirty)
			{
				Vector<ID3D11RenderTargetView*> rtvs(render_targets.size());
				for (uint32_t i = 0; i < render_targets.size(); ++i)
				{
					state_.render_targets[i] = render_targets[i];
					rtvs[i] = getRTV(state_.render_targets[i]);
				}

				state_.depth_target = depth_buffer;

				setRenderTargets(rtvs, getDSV(depth_buffer));
			}
		}

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::setRenderTargets(
			Vector<ID3D11RenderTargetView*> render_targets,
			ID3D11DepthStencilView* depth_buffer)
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			memset(dx_state_.render_targets, 0, sizeof(dx_state_.render_targets));
			state_.num_render_targets = (uint8_t)render_targets.size();
			for (uint8_t i = 0; i < (uint8_t)render_targets.size(); ++i)
			{
				dx_state_.render_targets[i] = render_targets[i];
			}

			dx_state_.depth_target = depth_buffer;
			makeDirty(DirtyStates::kRenderTargets);
		}

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::pushMarker(const String& name)
    {
#if GPU_MARKERS
		LMB_ASSERT(name.size(), "D3D11 CONTEXT: Tried to push marker with empty name");
		std::wstring wname(name.begin(), name.end());
      user_defined_annotation_->BeginEvent(wname.c_str());
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::setMarker(const String& name)
    {
#if GPU_MARKERS
		LMB_ASSERT(name.size(), "D3D11 CONTEXT: Tried to set marker with empty name");
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
		LMB_ASSERT(name.size(), "D3D11 CONTEXT: Tried to begin timer with empty name");
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
		LMB_ASSERT(name.size(), "D3D11 CONTEXT: Tried to end timer with empty name");
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
		LMB_ASSERT(name.size(), "D3D11 CONTEXT: Tried to get timer with empty name");
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
    void D3D11Context::setRenderScale(const float& render_scale)
    {
		LMB_ASSERT(render_scale > 0 && render_scale < 2, "D3D11 CONTEXT: Tried to set invalid render scale | %f", render_scale);
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
			vsync_ = vsync;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool D3D11Context::getVSync() const
    {
      return vsync_;
    }

	///////////////////////////////////////////////////////////////////////////
	void D3D11Context::destroyTexture(const size_t& hash)
	{
		LMB_ASSERT(hash, "D3D11 CONTEXT: Tried to destroy invalid texture");
		asset_manager_.removeTexture(hash);
	}

	///////////////////////////////////////////////////////////////////////////
	void D3D11Context::destroyShader(const size_t& hash)
	{
		LMB_ASSERT(hash, "D3D11 CONTEXT: Tried to destroy invalid shader");
		asset_manager_.removeShader(hash);
	}

	///////////////////////////////////////////////////////////////////////////
	void D3D11Context::destroyMesh(const size_t& hash)
	{
		LMB_ASSERT(hash, "D3D11 CONTEXT: Tried to destroy invalid mesh");
		asset_manager_.removeMesh(hash);
	}
    
    ///////////////////////////////////////////////////////////////////////////
    ID3D11RenderTargetView* D3D11Context::getRTV(
      asset::VioletTextureHandle texture, 
      int layer,
      int mip_map)
    {
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

      if (!texture)
        return nullptr;

      D3D11RenderTexture* tex = asset_manager_.getTexture(texture);
      return tex->getTexture()->getRTV(layer, mip_map);
    }

    ///////////////////////////////////////////////////////////////////////////
		ID3D11DepthStencilView* D3D11Context::getDSV(
			asset::VioletTextureHandle texture,
			int layer,
			int mip_map)
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			if (!texture)
				return nullptr;

			D3D11RenderTexture* tex = asset_manager_.getTexture(texture);
			return tex->getTexture()->getDSV(layer, mip_map);
		}

		scene::Scene* D3D11Context::getScene() const
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");
			return override_scene_ ? override_scene_ : scene_;
		}

		void D3D11Context::resizeImpl()
		{
			LMB_ASSERT(override_scene_, "D3D11 CONTEXT: Tried to render outside of the flush thread");

			unsigned int width = 0;
			unsigned int height = 0;

			getD3D11Context()->OMSetRenderTargets(0, nullptr, nullptr);
			context_.backbuffer.Reset();

			glm::uvec2 render_size =
				(glm::uvec2)((glm::vec2)getScene()->window->getSize() *
					getScene()->window->getDPIMultiplier());
			context_.swap_chain->ResizeBuffers(
				1,
				render_size.x,
				render_size.y,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				0
			);

			HRESULT result;
			ID3D11Texture2D* back_buffer;
			result = context_.swap_chain->GetBuffer(
				0,
				__uuidof(ID3D11Texture2D),
				(void**)&back_buffer
			);
			LMB_ASSERT(SUCCEEDED(result), "D3D11 CONTEXT: Could not get swapchain buffer | %llu", result);

			D3D11_TEXTURE2D_DESC tex_desc{};
			back_buffer->GetDesc(&tex_desc);
			width = tex_desc.Width;
			height = tex_desc.Height;
			screen_size_.x = (float)width;
			screen_size_.y = (float)height;

			result = context_.device->CreateRenderTargetView(
				back_buffer,
				NULL,
				context_.backbuffer.ReleaseAndGetAddressOf()
			);

			LMB_ASSERT(SUCCEEDED(result), "D3D11 CONTEXT: Could not create render target view | %llu", result);
			back_buffer->Release();

			default_.viewport.x = 0.0f;
			default_.viewport.y = 0.0f;
			default_.viewport.z = (float)width;
			default_.viewport.w = (float)height;
			setViewports(Vector<glm::vec4>{ default_.viewport });

			getScene()->post_process_manager->resize(glm::vec2(width, height) * render_scale_);

			setScissorRects({ glm::vec4(0.0f, 0.0f, getScene()->window->getSize().x, getScene()->window->getSize().y) });
		}
 
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::present()
    {
      HRESULT result;
      result = context_.swap_chain->Present(vsync_, 0);
	  LMB_ASSERT(SUCCEEDED(result), "D3D11 CONTEXT: Could not present | %llu", result);
	}

    ///////////////////////////////////////////////////////////////////////////
    D3D11Context::D3D11AssetManager::~D3D11AssetManager()
    {
			deleteAllAssets();
    }

    ///////////////////////////////////////////////////////////////////////////
    D3D11RenderTexture* D3D11Context::D3D11AssetManager::getTexture(
      asset::VioletTextureHandle texture)
    {
      if (!texture)
        return nullptr;

      if (texture->isDirty() && (texture->getLayer(0u).getFlags() & kTextureFlagRecreate) != 0)
      {
        texture->getLayer(0u).setFlags(texture->getLayer(0u).getFlags() & ~kTextureFlagRecreate);
        removeTexture(texture);
        texture->clean();
      }

			// TODO (Hilze): Have some sort of check to make sure that the size and format still match.
      auto it = textures_.find(texture.getHash());
      
      if (it == textures_.end())
      {
        D3D11RenderTexture* d3d11_texture = 
          static_cast<D3D11RenderTexture*>(
						context_->allocRenderTexture(texture)
          );
        textures_.insert(
          eastl::make_pair(
            texture.getHash(), 
            RefType<D3D11RenderTexture>{ d3d11_texture, 0u, 0.0f, texture->getKeepInMemory() }
          )
        );
        it = textures_.find(texture.getHash());
				texture->clean();
      }
			else if (texture->isDirty())
			{
				it->second.t->getTexture()->update(
					texture,
					context_->getD3D11Device(),
					context_->getD3D11Context()
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
				foundation::Info("Released texture with hash \"" + toString(texture) + "\"\n");
        platform::IRenderTexture* tex = it->second.t;
				context_->freeRenderTexture(tex);
        textures_.erase(it);
      }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    D3D11Mesh* D3D11Context::D3D11AssetManager::getMesh(asset::VioletMeshHandle mesh)
    {
      auto it = meshes_.find(mesh.getHash());

      if (it == meshes_.end())
      {
        D3D11Mesh* d3d11_mesh = 
          foundation::Memory::construct<D3D11Mesh>(context_);
        
        meshes_.insert(
          eastl::make_pair(
            mesh.getHash(),
            RefType<D3D11Mesh>{ d3d11_mesh, 0u, 0.0f, false }
          )
        );

        it = meshes_.find(mesh.getHash());
      }

      it->second.ref++;
      return it->second.t;
    }
   
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::removeMesh(asset::VioletMeshHandle mesh)
    {
      removeMesh(mesh.getHash());
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::removeMesh(size_t mesh)
    {
      auto it = meshes_.find(mesh);
      if (it != meshes_.end())
      {
				foundation::Info("Released mesh with hash \"" + toString(mesh) + "\"\n");
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
					foundation::Memory::construct<D3D11Shader>(shader, context_);
				shaders_.insert(
					eastl::make_pair(
						shader.getHash(),
						RefType<D3D11Shader>{ d3d11_shader, 0u, 0.0f, shader->getKeepInMemory() }
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
				foundation::Info("Released shader with hash \"" + toString(shader) + "\"\n");
				D3D11Shader* shad = it->second.t;
				foundation::Memory::destruct(shad);
				shaders_.erase(it);
			}
		}
    
    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::setD3D11Context(
      D3D11Context* context)
    {
			context_ = context;
    }

    ///////////////////////////////////////////////////////////////////////////
    void D3D11Context::D3D11AssetManager::deleteNotReffedAssets(
      const float& dt)
    {
			return; // TODO (Hilze): Remove this asap.

      // Textures.
      {
        Vector<uint64_t> marked_for_delete;
        for (auto& it : textures_)
        {
          if (it.second.ref == 0u)
          {
            it.second.time += dt;
            if (it.second.time >= kMaxTime && !it.second.keep_in_memory)
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
            if (it.second.time >= kMaxTime && !it.second.keep_in_memory)
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
						if (it.second.time >= kMaxTime && !it.second.keep_in_memory)
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

		///////////////////////////////////////////////////////////////////////////
		void D3D11Context::D3D11AssetManager::deleteAllAssets()
		{
			while (!textures_.empty())
				removeTexture(textures_.begin()->first);
			while (!meshes_.empty())
				removeMesh(meshes_.begin()->first);
			while (!shaders_.empty())
				removeShader(shaders_.begin()->first);
		}
  }
}