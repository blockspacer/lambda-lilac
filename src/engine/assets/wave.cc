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
    VioletWaveHandle WaveManager::create(Name name)
    {
      return VioletWaveHandle(foundation::Memory::construct<Wave>(), name);
    }

    ///////////////////////////////////////////////////////////////////////////
    VioletWaveHandle WaveManager::create(Name name, Wave wave)
    {
      return VioletWaveHandle(foundation::Memory::construct<Wave>(wave), name);
    }

    ///////////////////////////////////////////////////////////////////////////
    VioletWaveHandle WaveManager::create(Name name, VioletWave wave)
    {
      return VioletWaveHandle(foundation::Memory::construct<Wave>(wave), name);
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
    void WaveManager::destroy(VioletWaveHandle wave)
    {
      foundation::Memory::destruct<Wave>(wave.get());
    }
    
    ///////////////////////////////////////////////////////////////////////////
    WaveManager* WaveManager::getInstance()
    {
      static WaveManager* s_instance = 
        foundation::Memory::construct<WaveManager>();
      
      return s_instance;
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