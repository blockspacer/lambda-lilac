#include "Shader.h"
#include <utils/file_system.h>
#include <memory/memory.h>
#include <soloud_wav.h>

namespace lambda
{
	namespace asset
	{
		///////////////////////////////////////////////////////////////////////////
		Shader::Shader()
			: keep_in_memory_(false)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		Shader::Shader(const Shader& shader)
			: shader_(shader.shader_)
			, queued_shader_variables_(shader.queued_shader_variables_)
			, keep_in_memory_(shader.keep_in_memory_)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		Shader::Shader(VioletShader shader)
			: shader_(shader)
			, keep_in_memory_(false)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		Shader::~Shader()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		String Shader::getFilePath() const
		{
			return shader_.file_path;
		}

		///////////////////////////////////////////////////////////////////////////
		Vector<char> Shader::getByteCode(ShaderStages stage, int type) const
		{
			return shader_.blobs[(int)stage][type];
		}

		///////////////////////////////////////////////////////////////////////////
		Vector<VioletShaderResource> Shader::getResources(ShaderStages stage) const
		{
			return shader_.resources[stage];
		}

		///////////////////////////////////////////////////////////////////////////
		void Shader::setShaderVariable(const platform::ShaderVariable& variable)
		{
			queued_shader_variables_.push_back(variable);
		}

		///////////////////////////////////////////////////////////////////////////
		Vector<platform::ShaderVariable> Shader::getQueuedShaderVariables()
		{
			return eastl::move(queued_shader_variables_);
		}

		///////////////////////////////////////////////////////////////////////////
		bool Shader::getKeepInMemory() const
		{
			return keep_in_memory_;
		}

		///////////////////////////////////////////////////////////////////////////
		void Shader::setKeepInMemory(bool keep_in_memory)
		{
			keep_in_memory_ = keep_in_memory;
		}










		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::create(Name name)
		{
			return VioletShaderHandle(foundation::Memory::construct<Shader>(), name);
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::create(Name name, Shader shader)
		{
			return VioletShaderHandle(foundation::Memory::construct<Shader>(shader), name);
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::create(Name name, VioletShader shader)
		{
			return VioletShaderHandle(foundation::Memory::construct<Shader>(shader), name);
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::get(Name name)
		{
			return get(manager_.GetHash(FileSystem::MakeRelative(name.getName())));
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::get(uint64_t hash)
		{
			VioletShader shader = manager_.GetShader(hash);
			return create(shader.file_path, shader);
		}

		///////////////////////////////////////////////////////////////////////////
		void ShaderManager::destroy(VioletShaderHandle shader)
		{
			foundation::Memory::destruct<Shader>(shader.get());
		}

		///////////////////////////////////////////////////////////////////////////
		Array<Array<Vector<char>, VIOLET_LANG_COUNT>, (int)ShaderStages::kCount>
			ShaderManager::getData(const VioletShaderHandle& Shader)
		{
			VioletShader s = Shader->getVioletShader();
			getManager().getBlobs(s);
			return s.blobs;
		}

		///////////////////////////////////////////////////////////////////////////
		ShaderManager* ShaderManager::getInstance()
		{
			static ShaderManager* s_instance =
				foundation::Memory::construct<ShaderManager>();

			return s_instance;
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderManager& ShaderManager::getManager()
		{
			return manager_;
		}

		///////////////////////////////////////////////////////////////////////////
		const VioletShaderManager& ShaderManager::getManager() const
		{
			return manager_;
		}
	}
}