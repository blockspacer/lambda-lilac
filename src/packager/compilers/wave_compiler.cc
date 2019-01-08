#include "wave_compiler.h"
#include <utils/file_system.h>
#include <utils/utilities.h>
#include <soloud_wav.h>

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletWaveCompiler::VioletWaveCompiler() :
    VioletWaveManager()
  {
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletWaveCompiler::Compile(WaveCompileInfo wave_info)
  {
    SoLoud::Wav wav;
    wav.load(wave_info.file.c_str());
    
    VioletWave wave;
    wave.hash   = GetHash(wave_info.file);
    wave.file   = wave_info.file;
    wave.length = (float)wav.getLength();
    wave.data   = utilities::convertVec<char, uint32_t>(FileSystem::FileToVector(wave_info.file));
    AddWave(wave);
  }
}