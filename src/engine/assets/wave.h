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

	  static void release(Wave* wave, const size_t& hash);
		static VioletHandle<Wave> privMetaSet(const String& name);

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
      void destroy(Wave* wave, const size_t& hash);

    public:
      static WaveManager* getInstance();
			~WaveManager();

    protected:
      VioletWaveManager& getManager();
      const VioletWaveManager& getManager() const;

    private:
      VioletWaveManager manager_;
			UnorderedMap<uint64_t, Wave*> wave_cache_;
		};
  }
}
