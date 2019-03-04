#include "wave_manager.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <utils/console.h>

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletWaveManager::VioletWaveManager()
  {
    SetMagicNumber("wav");
    SetGeneratedFilePath("generated/");
    Load();
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t VioletWaveManager::GetHash(String wave_name)
  {
    return hash(wave_name);
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletWaveManager::AddWave(VioletWave wave)
  {
	SaveHeader(WaveHeaderToJSon(wave), wave.hash);
	SaveData(wave.data, wave.hash);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletWave VioletWaveManager::GetWave(uint64_t hash, bool get_data)
  {
    VioletWave wave = JSonToWaveHeader(GetHeader(hash));
    if (get_data)
      wave.data = GetData(hash);
    return wave;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletWaveManager::RemoveWave(uint64_t hash)
  {
	  RemoveData(hash);
	  RemoveHeader(hash);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletWave VioletWaveManager::JSonToWaveHeader(Vector<char> data)
  {
    rapidjson::Document doc;
    const auto& parse_error = doc.Parse(data.data(), data.size());
    LMB_ASSERT(!parse_error.HasParseError(), "TODO (Hilze): Fill in!");

    VioletWave wave;
    wave.hash   = doc["hash"].GetUint64();
    wave.file   = lmbString(doc["file"].GetString());
    wave.length = doc["length"].GetFloat();
    
    return wave;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Vector<char> VioletWaveManager::WaveHeaderToJSon(VioletWave wave)
  {
    rapidjson::Document doc;
    doc.SetObject();

    doc.AddMember("hash", wave.hash, doc.GetAllocator());
    doc.AddMember("file", rapidjson::StringRef(wave.file.c_str()), doc.GetAllocator());
    doc.AddMember("length", wave.length, doc.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::string string = buffer.GetString();

    Vector<char> data(string.size());
    memcpy(data.data(), string.data(), string.size());
    return data;
  }
}