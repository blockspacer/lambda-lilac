#pragma once
#include "assets/asset_handle.h"
#include <containers/containers.h>
#include <assets/wave_manager.h>

namespace SoLoud
{
  class Wav;
}

namespace lambda
{
  namespace asset
  {
    ///////////////////////////////////////////////////////////////////////////
    class Wave
    {
    public:
      Wave();
      Wave(const Wave& wave);
      Wave(VioletWave wave);
      ~Wave();

      void setBuffer(SoLoud::Wav* buffer);
      SoLoud::Wav* getBuffer() const;

    private:
      SoLoud::Wav* buffer_;
    };
    typedef VioletHandle<Wave> VioletWaveHandle;

    ///////////////////////////////////////////////////////////////////////////
    class WaveManager
    {
    public:
      VioletWaveHandle create(Name name);
      VioletWaveHandle create(Name name, Wave wave);
      VioletWaveHandle create(Name name, VioletWave wave);
      VioletWaveHandle get(Name name);
      VioletWaveHandle get(uint64_t hash);
      void destroy(VioletWaveHandle Wave);

    public:
      static WaveManager* getInstance();

    protected:
      VioletWaveManager& getManager();
      const VioletWaveManager& getManager() const;

    private:
      VioletWaveManager manager_;
    };
  }
}