#include "wave.h"
#include <utils/file_system.h>
#include <memory/memory.h>
#include <soloud_wav.h>

namespace lambda
{
	namespace asset
	{
		///////////////////////////////////////////////////////////////////////////
		Wave::Wave() :
			buffer_(nullptr)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		Wave::Wave(const Wave& wave) :
			buffer_(wave.buffer_)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		Wave::Wave(VioletWave wave)
		{
			SoLoud::Wav* wav = foundation::Memory::construct<SoLoud::Wav>();
			//wav->loadMem((unsigned char*)data.data(), (unsigned int)data.size());
			wav->load(FileSystem::FullFilePath(wave.file).c_str());
			setBuffer(wav);
		}

		///////////////////////////////////////////////////////////////////////////
		Wave::~Wave()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void Wave::setBuffer(SoLoud::Wav* buffer)
		{
			buffer_ = buffer;
		}

		///////////////////////////////////////////////////////////////////////////
		SoLoud::Wav* Wave::getBuffer() const
		{
			return buffer_;
		}

		///////////////////////////////////////////////////////////////////////////
		void Wave::release(Wave* wave, const size_t& hash)
		{
			WaveManager::getInstance()->destroy(wave, hash);
		}

		///////////////////////////////////////////////////////////////////////////
		VioletHandle<Wave> Wave::privMetaSet(const String& name)
		{
			return WaveManager::getInstance()->get(Name(name));
		}










		///////////////////////////////////////////////////////////////////////////
		VioletWaveHandle WaveManager::create(Name name)
		{
			VioletWaveHandle handle;

			auto it = wave_cache_.find(name.getHash());
			if (it == wave_cache_.end())
			{
				handle = VioletWaveHandle(foundation::Memory::construct<Wave>(), name);
				wave_cache_.insert(eastl::make_pair(name.getHash(), handle.get()));
			}
			else
				handle = VioletWaveHandle(it->second, name);

			return handle;
		}

		///////////////////////////////////////////////////////////////////////////
		VioletWaveHandle WaveManager::create(Name name, Wave wave)
		{
			VioletWaveHandle handle;

			auto it = wave_cache_.find(name.getHash());
			if (it == wave_cache_.end())
			{
				handle = VioletWaveHandle(foundation::Memory::construct<Wave>(wave), name);
				wave_cache_.insert(eastl::make_pair(name.getHash(), handle.get()));
			}
			else
				handle = VioletWaveHandle(it->second, name);

			return handle;
		}

		///////////////////////////////////////////////////////////////////////////
		VioletWaveHandle WaveManager::create(Name name, VioletWave wave)
		{
			VioletWaveHandle handle;

			auto it = wave_cache_.find(name.getHash());
			if (it == wave_cache_.end())
			{
				handle = VioletWaveHandle(foundation::Memory::construct<Wave>(wave), name);
				wave_cache_.insert(eastl::make_pair(name.getHash(), handle.get()));
			}
			else
				handle = VioletWaveHandle(it->second, name);

			return handle;
		}

		///////////////////////////////////////////////////////////////////////////
		VioletWaveHandle WaveManager::get(Name name)
		{
			return get(manager_.GetHash(FileSystem::MakeRelative(name.getName())));
		}

		///////////////////////////////////////////////////////////////////////////
		VioletWaveHandle WaveManager::get(uint64_t hash)
		{
			VioletWave wave = manager_.GetWave(hash);
			return create(wave.file, wave);
		}

		///////////////////////////////////////////////////////////////////////////
		void WaveManager::destroy(Wave* wave, const size_t& hash)
		{
			if (wave_cache_.empty())
				return;

			auto it = wave_cache_.find(hash);
			if (it != wave_cache_.end())
				wave_cache_.erase(it);

			foundation::Memory::destruct<Wave>(wave);
		}

		///////////////////////////////////////////////////////////////////////////
		WaveManager* WaveManager::getInstance()
		{
			static WaveManager* s_instance =
				foundation::Memory::construct<WaveManager>();

			return s_instance;
		}

		///////////////////////////////////////////////////////////////////////////
		WaveManager::~WaveManager()
		{
			while (!wave_cache_.empty())
				destroy(wave_cache_.begin()->second, wave_cache_.begin()->first);
		}

		///////////////////////////////////////////////////////////////////////////
		VioletWaveManager& WaveManager::getManager()
		{
			return manager_;
		}

		///////////////////////////////////////////////////////////////////////////
		const VioletWaveManager& WaveManager::getManager() const
		{
			return manager_;
		}
	}
}