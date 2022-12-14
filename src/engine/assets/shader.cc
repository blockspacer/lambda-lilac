#include "Shader.h"
#include <utils/file_system.h>
#include <memory/memory.h>
#include <interfaces/irenderer.h>

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
			return shader_.resources[(int)stage];
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
		void Shader::release(Shader* shader, const size_t& hash)
		{
			ShaderManager::getInstance()->destroy(shader, hash);
		}

		///////////////////////////////////////////////////////////////////////////
		VioletHandle<Shader> Shader::privMetaSet(const String& name)
		{
			return ShaderManager::getInstance()->getFromCache(Name(name));
		}










		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::create(Name name)
		{
			VioletShaderHandle handle;

			auto it = shader_cache_.find(name.getHash());
			if (it == shader_cache_.end())
			{
				handle = VioletShaderHandle(foundation::Memory::construct<Shader>(), name);
				shader_cache_.insert(eastl::make_pair(name.getHash(), handle.get()));
			}
			else
				handle = VioletShaderHandle(it->second, name);

			return handle;
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::create(Name name, Shader shader)
		{
			VioletShaderHandle handle;

			auto it = shader_cache_.find(name.getHash());
			if (it == shader_cache_.end())
			{
				handle = VioletShaderHandle(foundation::Memory::construct<Shader>(shader), name);
				shader_cache_.insert(eastl::make_pair(name.getHash(), handle.get()));
			}
			else
				handle = VioletShaderHandle(it->second, name);

			return handle;
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::create(Name name, VioletShader shader)
		{
			VioletShaderHandle handle;

			auto it = shader_cache_.find(name.getHash());
			if (it == shader_cache_.end())
			{
				handle = VioletShaderHandle(foundation::Memory::construct<Shader>(shader), name);
				shader_cache_.insert(eastl::make_pair(name.getHash(), handle.get()));
			}
			else
				handle = VioletShaderHandle(it->second, name);

			return handle;
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::get(Name name)
		{
			String str = FileSystem::MakeRelative(name.getName());
			if (str.find('|') == String::npos)
			{
				str += "|DEFAULT";
				name = str;
			}
			uint64_t hash = manager_.GetHash(str);
			LMB_ASSERT(manager_.HasHeader(hash), "Could not find shader: %s", name.getName().c_str());
			return get(hash);
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::get(uint64_t hash)
		{
			VioletShader shader = manager_.GetShader(hash);
			return create(shader.file_path, shader);
		}

		///////////////////////////////////////////////////////////////////////////
		void ShaderManager::destroy(Shader* shader, const size_t& hash)
		{
			if (shader_cache_.empty())
				return;

			auto it = shader_cache_.find(hash);
			if (it != shader_cache_.end())
				shader_cache_.erase(it);

			foundation::Memory::destruct<Shader>(shader);
			renderer_->destroyShader(hash);
		}

		///////////////////////////////////////////////////////////////////////////
		Array<Array<Vector<char>, VIOLET_LANG_COUNT>, (int)ShaderStages::kCount>
			ShaderManager::getData(VioletShaderHandle Shader)
		{
			VioletShader& s = Shader->getVioletShader();
			getManager().getBlobs(s);
			return s.blobs;
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::getFromCache(Name name)
		{
			auto it = shader_cache_.find(name.getHash());
			LMB_ASSERT(it != shader_cache_.end(), "Could not find shader %s in cache", name.getName().c_str());
			return VioletShaderHandle(it->second, name);
		}

		///////////////////////////////////////////////////////////////////////////
		VioletShaderHandle ShaderManager::getFromCache(size_t hash)
		{
			auto it = shader_cache_.find(hash);
			LMB_ASSERT(it != shader_cache_.end(), "Could not find shader with hash %llu in cache", hash);
			return VioletShaderHandle(it->second, Name("shader with unknown origin"));
		}

		///////////////////////////////////////////////////////////////////////////
		bool ShaderManager::hasInCache(Name name)
		{
			return hasInCache(name.getHash());
		}

		///////////////////////////////////////////////////////////////////////////
		bool ShaderManager::hasInCache(size_t hash)
		{
			auto it = shader_cache_.find(hash);
			return it != shader_cache_.end();
		}

		///////////////////////////////////////////////////////////////////////////
		ShaderManager* ShaderManager::getInstance()
		{
			static ShaderManager* s_instance =
				foundation::Memory::construct<ShaderManager>();

			return s_instance;
		}

		///////////////////////////////////////////////////////////////////////////
		void ShaderManager::setRenderer(platform::IRenderer* renderer)
		{
			getInstance()->renderer_ = renderer;
		}

		///////////////////////////////////////////////////////////////////////////
		ShaderManager::~ShaderManager()
		{
			while (!shader_cache_.empty())
				destroy(shader_cache_.begin()->second, shader_cache_.begin()->first);
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