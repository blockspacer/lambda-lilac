#include "wave_compiler.h"
#include <utils/file_system.h>
#include <utils/utilities.h>
#include <soloud_wav.h>
#include <utils/console.h>

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletWaveCompiler::VioletWaveCompiler()
		: VioletWaveManager()
  {
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  bool VioletWaveCompiler::Compile(WaveCompileInfo wave_info)
  {
    SoLoud::Wav wav;
	switch (wav.load(FileSystem::FullFilePath(wave_info.file).c_str()))
	{
	case SoLoud::SOLOUD_ERRORS::SO_NO_ERROR:
		break;
	case SoLoud::SOLOUD_ERRORS::INVALID_PARAMETER:
		foundation::Error("SoLoud: Invalid parameter\n");
		return false;
	case SoLoud::SOLOUD_ERRORS::FILE_NOT_FOUND:
		foundation::Error("SoLoud: File not found\n");
		return false;
	case SoLoud::SOLOUD_ERRORS::FILE_LOAD_FAILED:
		foundation::Error("SoLoud: Load failed\n");
		return false;
	case SoLoud::SOLOUD_ERRORS::DLL_NOT_FOUND:
		foundation::Error("SoLoud: DLL not found\n");
		return false;
	case SoLoud::SOLOUD_ERRORS::OUT_OF_MEMORY:
		foundation::Error("SoLoud: Out of memory\n");
		return false;
	case SoLoud::SOLOUD_ERRORS::NOT_IMPLEMENTED:
		foundation::Error("SoLoud: Not implemented\n");
		return false;
	case SoLoud::SOLOUD_ERRORS::UNKNOWN_ERROR:
		foundation::Error("SoLoud: Unknown error\n");
		return false;
	}

    VioletWave wave;
    wave.hash   = GetHash(wave_info.file);
    wave.file   = wave_info.file;
    wave.length = (float)wav.getLength();
    wave.data   = FileSystem::FileToVector(wave_info.file);
    AddWave(wave);

	return true;
  }
}