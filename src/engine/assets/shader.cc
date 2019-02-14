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
		{
		}

		///////////////////////////////////////////////////////////////////////////
		Shader::Shader(const Shader& shader)
			: shader_(shader.shader_)
			, queued_shader_variables_(queued_shader_variables_)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		Shader::Shader(VioletShader shader)
			: shader_(shader)
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
		Vector<uint32_t> Shader::getByteCode(ShaderStages stage, int type) const
		{
			return shader_.blobs[(int)stage][type];
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