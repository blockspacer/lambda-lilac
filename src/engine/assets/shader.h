#pragma once
#include "assets/asset_handle.h"
#include <containers/containers.h>
#include <assets/shader_manager.h>

namespace lambda
{
	namespace platform
	{
		class IRenderer;
	}
	namespace asset
	{
		///////////////////////////////////////////////////////////////////////////
		class Shader
		{
		public:
			Shader();
			Shader(const Shader& Shader);
			Shader(VioletShader Shader);
			~Shader();

			String getFilePath() const;
			Vector<char> getByteCode(ShaderStages stage, int type) const;
			Vector<VioletShaderResource> getResources(ShaderStages stage) const;

			bool getKeepInMemory() const;
			void setKeepInMemory(bool keep_in_memory);

			static void release(Shader* shader, const size_t& hash);

		protected:
			VioletShader& getVioletShader() { return shader_; }
			const VioletShader& getVioletShader() const { return shader_; }
			friend class ShaderManager;

		private:
			bool keep_in_memory_;
			VioletShader shader_;
		};
		typedef VioletHandle<Shader> VioletShaderHandle;

		///////////////////////////////////////////////////////////////////////////
		class ShaderManager
		{
		public:
			VioletShaderHandle create(Name name);
			VioletShaderHandle create(Name name, Shader Shader);
			VioletShaderHandle create(Name name, VioletShader Shader);
			VioletShaderHandle get(Name name);
			VioletShaderHandle get(uint64_t hash);
			void destroy(Shader* shader, const size_t& hash);
			Array<Array<Vector<char>, VIOLET_LANG_COUNT>, (int)ShaderStages::kCount>
			getData(VioletShaderHandle Shader);

			VioletShaderHandle getFromCache(Name name);
			VioletShaderHandle getFromCache(size_t hash);
			bool hasInCache(Name name);
			bool hasInCache(size_t hash);

		public:
			static ShaderManager* getInstance();
			static void setRenderer(platform::IRenderer* renderer);
			~ShaderManager();

		protected:
			VioletShaderManager& getManager();
			const VioletShaderManager& getManager() const;

		private:
			VioletShaderManager manager_;
			platform::IRenderer* renderer_;

			UnorderedMap<uint64_t, Shader*> shader_cache_;
			std::mutex mutex_;
		};
	}
}