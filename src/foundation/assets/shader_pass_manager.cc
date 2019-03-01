#include "shader_pass_manager.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <utils/console.h>

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletShaderPassManager::VioletShaderPassManager()
  {
    SetMagicNumber("pass");
    SetGeneratedFilePath("generated/");
    Load();
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t VioletShaderPassManager::GetHash(String pass_name)
  {
    return hash(pass_name);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletShaderPassManager::AddPass(VioletShaderPass pass)
  {
    SaveData(ShaderPassToJSon(pass), pass.hash);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletShaderPass VioletShaderPassManager::GetPass(uint64_t hash)
  {
    return JSonToShaderPass(GetData(hash));
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletShaderPass VioletShaderPassManager::JSonToShaderPass(Vector<char> data)
  {
    rapidjson::Document doc;
    const auto& parse_error = doc.Parse(data.data(), data.size());
    LMB_ASSERT(!parse_error.HasParseError(), "TODO (Hilze): Fill in!");

    VioletShaderPass pass;
    pass.hash          = doc["hash"].GetUint64();
    pass.name          = lmbString(doc["name"].GetString());
    pass.depth_enabled = doc["depth enabled"].GetBool();
    pass.blend_enabled = doc["blend enabled"].GetBool();
    pass.depth_func    = (ShaderDepthFunc)doc["depth function"].GetUint();
    pass.blend_mode    = (ShaderBlendMode)doc["blend mode"].GetUint();
    pass.shaders.resize(doc["shaders"].Size());
    for (uint32_t i = 0u; i < pass.shaders.size(); ++i)
      pass.shaders[i] = doc["shaders"][i].GetUint64();

    return pass;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Vector<char> VioletShaderPassManager::ShaderPassToJSon(VioletShaderPass shader_pass)
  {
    rapidjson::Document doc;
    doc.SetObject();
    
    doc.AddMember("hash", shader_pass.hash, doc.GetAllocator());
    doc.AddMember("name", rapidjson::StringRef(shader_pass.name.c_str()), doc.GetAllocator());
    doc.AddMember("depth enabled", shader_pass.depth_enabled, doc.GetAllocator());
    doc.AddMember("blend enabled", shader_pass.blend_enabled, doc.GetAllocator());
    doc.AddMember("depth function", (uint32_t)shader_pass.depth_func, doc.GetAllocator());
    doc.AddMember("blend mode", (uint32_t)shader_pass.blend_mode, doc.GetAllocator());
    doc.AddMember("shaders", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());

    for (uint32_t i = 0u; i < shader_pass.shaders.size(); ++i)
      doc["shaders"].PushBack(shader_pass.shaders[i], doc.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::string string = buffer.GetString();

    Vector<char> data(string.size());
    memcpy(data.data(), string.data(), string.size());
    return data;
  }
}