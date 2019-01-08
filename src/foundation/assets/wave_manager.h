#pragma once
#include "base_asset_manager.h"
#include <containers/containers.h>
#include "enums.h"

namespace lambda
{
  struct VioletWave
  {
    uint64_t hash;
    String file;
    float length;
    Vector<uint32_t> data;
  };

  class VioletWaveManager : public VioletBaseAssetManager
  {
  public:
    VioletWaveManager();
    
    uint64_t GetHash(String wave_name);

    void AddWave(VioletWave wave);
    VioletWave GetWave(uint64_t hash);
    void RemoveWave(uint64_t hash);

  private:
    VioletWave JSonToWave(Vector<char> json);
    Vector<char> WaveToJSon(VioletWave wave);
  };
}