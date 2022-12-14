#pragma once
#include "iwindow.h"
#include "assets/shader.h"
#include "assets/mesh.h"
#include "assets/texture.h"
#include <glm/glm.hpp>
#include <memory/memory.h>

#define cbUserDataIdx 0
#define cbPerFrameIdx 1
#define cbPerCameraIdx 2
#define cbPerMeshIdx 3
#define cbPerLightIdx 4
#define cbDynamicResolutionIdx 5
#define cbGuiIdx 6

namespace lambda
{
	namespace scene
	{
		struct Scene;
	}
	namespace platform
	{
		///////////////////////////////////////////////////////////////////////////
		class IRenderBuffer
		{
		public:
			static constexpr uint32_t kFlagDynamic   = 1u << 1u;
			static constexpr uint32_t kFlagStaging   = 1u << 2u;
			static constexpr uint32_t kFlagImmutable = 1u << 3u;
			static constexpr uint32_t kFlagVertex    = 1u << 4u;
			static constexpr uint32_t kFlagIndex     = 1u << 5u;
			static constexpr uint32_t kFlagConstant  = 1u << 6u;
			static constexpr uint32_t kFlagTransient = 1u << 7u;

			virtual void*    lock() = 0;
			virtual void     unlock() = 0;
			virtual uint32_t getFlags() const = 0;
			virtual uint32_t getSize()  const = 0;
		};

		///////////////////////////////////////////////////////////////////////////
		class IRenderTexture
		{
		public:
			virtual void*    lock(uint32_t level) = 0;
			virtual void     unlock(uint32_t level) = 0;
			virtual uint32_t getWidth()    const = 0;
			virtual uint32_t getHeight()   const = 0;
			virtual uint32_t getDepth()    const = 0;
			virtual uint32_t getMipCount() const = 0;
			virtual uint32_t getFlags()    const = 0;
			virtual TextureFormat getFormat() const = 0;
		};

		class PostProcessManager;
		class ShaderPass;
		class RasterizerState;
		class BlendState;
		class SamplerState;
		class DepthStencilState;
		class DebugRenderer;
		class IImGUI;
		class ShaderVariableManager;
		struct ShaderVariable;

		///////////////////////////////////////////////////////////////////////////
		class IRenderer
		{
		public:
			virtual platform::IRenderBuffer* allocRenderBuffer(uint32_t size, uint32_t flags, void* data = nullptr) = 0;
			virtual void freeRenderBuffer(platform::IRenderBuffer*& buffer) = 0;

			virtual ~IRenderer() = default;
			virtual void setWindow(platform::IWindow* window) = 0;
			virtual void setOverrideScene(scene::Scene* scene) = 0;
			virtual void initialize(scene::Scene& scene) = 0;
			virtual void deinitialize() = 0;
			virtual void resize() = 0;
			virtual void update(const double& delta_time) = 0;
			virtual void startFrame() = 0;
			virtual void endFrame(bool display = true) = 0;
			virtual void draw(uint32_t instance_count = 1ul) = 0;

			virtual void setRasterizerState(
				const RasterizerState& rasterizer_state
			) = 0;
			virtual void setBlendState(const BlendState& blend_state) = 0;
			virtual void setDepthStencilState(
				const DepthStencilState& depth_stencil_state
			) = 0;
			virtual void setSamplerState(
				const SamplerState& sampler_state,
				unsigned char slot
			) = 0;

			virtual void generateMipMaps(
				const asset::VioletTextureHandle& texture
			) = 0;
			virtual void copyToScreen(const asset::VioletTextureHandle& texture) = 0;
			virtual void copyToTexture(
				const asset::VioletTextureHandle& src,
				const asset::VioletTextureHandle& dst
			) = 0;
			virtual void bindShaderPass(const platform::ShaderPass& shader_pass) = 0;
			virtual void clearRenderTarget(
				asset::VioletTextureHandle texture,
				const glm::vec4& colour
			) = 0;

			virtual void setScissorRects(const Vector<glm::vec4>& rects) = 0;
			virtual void setViewports(const Vector<glm::vec4>& rects) = 0;

			virtual void setMesh(asset::VioletMeshHandle mesh) = 0;
			virtual void setSubMesh(const uint32_t& sub_mesh_idx) = 0;
			virtual void setShader(asset::VioletShaderHandle shader) = 0;
			virtual void setTexture(
				asset::VioletTextureHandle texture,
				uint8_t slot = 0
			) = 0;
			virtual void setConstantBuffer(
				IRenderBuffer* constant_buffer,
				uint8_t slot = 0
			) = 0;
			virtual void setUserData(glm::vec4 data, uint8_t slot = 0) = 0;
			virtual void setRenderTargets(
				Vector<asset::VioletTextureHandle> render_targets,
				asset::VioletTextureHandle depth_buffer
			) = 0;

			virtual void pushMarker(const String& name) = 0;
			virtual void setMarker(const String& name) = 0;
			virtual void popMarker() = 0;

			virtual void  beginTimer(const String& name) = 0;
			virtual void  endTimer(const String& name) = 0;
			virtual uint64_t getTimerMicroSeconds(const String& name) = 0;

			virtual void setRenderScale(const float& render_scale) = 0;
			virtual float getRenderScale() = 0;
			virtual void setVSync(bool vsync) = 0;
			virtual bool getVSync() const = 0;

			virtual void destroyTexture(const size_t& hash) = 0;
			virtual void destroyShader(const size_t& hash) = 0;
			virtual void destroyMesh(const size_t& hash) = 0;
		};
	}
}