#pragma once
#include <assets/wave_manager.h>

namespace lambda
{
  struct WaveCompileInfo
  {
    String file;
  };

  class VioletWaveCompiler : public VioletWaveManager
  {
  public:
    VioletWaveCompiler();
    void Compile(WaveCompileInfo wave_info);
  };
}