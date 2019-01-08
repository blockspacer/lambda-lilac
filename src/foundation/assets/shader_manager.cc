#include "shader_manager.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <utils/console.h>

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletShaderManager::VioletShaderManager()
  {
    SetMagicNumber("shader");
    SetFilePathIndex("shaders.vi");
    SetGeneratedFilePath("generated/");
    Load();
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t VioletShaderManager::GetHash(String file_path, Vector<String> defines, ShaderStages stage)
  {
    String str = file_path;
    
    for (const String& define : defines)
      str += define;

    str += toString((uint8_t)stage);

    return hash(str);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletShaderManager::AddShaderProgram(VioletShaderProgram shader_program)
  {
    SaveData(ShaderProgramToJSon(shader_program), shader_program.hash);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletShaderProgram VioletShaderManager::GetShaderProgram(uint64_t hash)
  {
    return JSonToShaderProgram(GetData(hash));
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Vector<char> VioletShaderManager::ShaderProgramToJSon(VioletShaderProgram shader_program)
  {
    rapidjson::Document doc;
    doc.SetObject();

    doc.AddMember("hash", shader_program.hash, doc.GetAllocator());
    doc.AddMember("file path", rapidjson::StringRef(shader_program.file_path.c_str()), doc.GetAllocator());
    doc.AddMember("stage", (uint32_t)shader_program.stage, doc.GetAllocator());
    doc.AddMember("defines", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());
    for (uint32_t i = 0u; i < shader_program.defines.size(); ++i)
      doc["defines"].PushBack(rapidjson::StringRef(shader_program.defines[i].c_str()), doc.GetAllocator());

    // Blobs.
    doc.AddMember("blobs", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());
    for (const auto& blob : shader_program.blobs)
    {
      doc["blobs"].AddMember(rapidjson::StringRef(blob.first.c_str()), rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());
      for (uint32_t i = 0u; i < blob.second.size(); ++i)
        doc["blobs"][blob.first.c_str()].PushBack(blob.second[i], doc.GetAllocator());
    }

    // Preparation
    doc.AddMember("reflection", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());
    doc["reflection"].AddMember("inputs", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());
    doc["reflection"].AddMember("outputs", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());
    doc["reflection"].AddMember("buffers", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());
    doc["reflection"].AddMember("textures", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());
    doc["reflection"].AddMember("samplers", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());

    // Shader reflection
    for (const auto& input : shader_program.reflection.inputs)
    {
      doc["reflection"]["inputs"].AddMember(rapidjson::StringRef(input.name.c_str()), rapidjson::Value(rapidjson::kObjectType), doc.GetAllocator());
      doc["reflection"]["inputs"][input.name.c_str()].AddMember("type", (uint32_t)input.type, doc.GetAllocator());
      doc["reflection"]["inputs"][input.name.c_str()].AddMember("size", input.size, doc.GetAllocator());
      doc["reflection"]["inputs"][input.name.c_str()].AddMember("offset", input.offset, doc.GetAllocator());
      doc["reflection"]["inputs"][input.name.c_str()].AddMember("rows", (uint32_t)input.rows, doc.GetAllocator());
      doc["reflection"]["inputs"][input.name.c_str()].AddMember("columns", (uint32_t)input.cols, doc.GetAllocator());
    }
    for (const auto& output : shader_program.reflection.outputs)
    {
      doc["reflection"]["outputs"].AddMember(rapidjson::StringRef(output.name.c_str()), rapidjson::Value(rapidjson::kObjectType), doc.GetAllocator());
      doc["reflection"]["outputs"][output.name.c_str()].AddMember("type", (uint32_t)output.type, doc.GetAllocator());
      doc["reflection"]["outputs"][output.name.c_str()].AddMember("size", output.size, doc.GetAllocator());
      doc["reflection"]["outputs"][output.name.c_str()].AddMember("offset", output.offset, doc.GetAllocator());
      doc["reflection"]["outputs"][output.name.c_str()].AddMember("rows", (uint32_t)output.rows, doc.GetAllocator());
      doc["reflection"]["outputs"][output.name.c_str()].AddMember("columns", (uint32_t)output.cols, doc.GetAllocator());
    }
    for (const auto& buffer : shader_program.reflection.buffers)
    {
      doc["reflection"]["buffers"].AddMember(rapidjson::StringRef(buffer.name.c_str()), rapidjson::Value(rapidjson::kObjectType), doc.GetAllocator());
      doc["reflection"]["buffers"][buffer.name.c_str()].AddMember("size", buffer.size, doc.GetAllocator());
      doc["reflection"]["buffers"][buffer.name.c_str()].AddMember("slot", (uint32_t)buffer.slot, doc.GetAllocator());
      doc["reflection"]["buffers"][buffer.name.c_str()].AddMember("variables", rapidjson::Value(rapidjson::kObjectType), doc.GetAllocator());

      for (const auto& variable : buffer.variables)
      {
        doc["reflection"]["buffers"][buffer.name.c_str()]["variables"].AddMember(rapidjson::StringRef(variable.name.c_str()), rapidjson::Value(rapidjson::kObjectType), doc.GetAllocator());
        doc["reflection"]["buffers"][buffer.name.c_str()]["variables"][variable.name.c_str()].AddMember("type", (uint32_t)variable.type, doc.GetAllocator());
        doc["reflection"]["buffers"][buffer.name.c_str()]["variables"][variable.name.c_str()].AddMember("size", variable.size, doc.GetAllocator());
        doc["reflection"]["buffers"][buffer.name.c_str()]["variables"][variable.name.c_str()].AddMember("offset", variable.offset, doc.GetAllocator());
        doc["reflection"]["buffers"][buffer.name.c_str()]["variables"][variable.name.c_str()].AddMember("rows", (uint32_t)variable.rows, doc.GetAllocator());
        doc["reflection"]["buffers"][buffer.name.c_str()]["variables"][variable.name.c_str()].AddMember("columns", (uint32_t)variable.cols, doc.GetAllocator());
      }
    }
    for (const auto& texture : shader_program.reflection.textures)
    {
      doc["reflection"]["textures"].AddMember(rapidjson::StringRef(texture.name.c_str()), rapidjson::Value(rapidjson::kObjectType), doc.GetAllocator());
      doc["reflection"]["textures"][texture.name.c_str()].AddMember("slot", (uint32_t)texture.slot, doc.GetAllocator());
    }
    for (const auto& sampler : shader_program.reflection.samplers)
    {
      doc["reflection"]["samplers"].AddMember(rapidjson::StringRef(sampler.name.c_str()), rapidjson::Value(rapidjson::kObjectType), doc.GetAllocator());
      doc["reflection"]["samplers"][sampler.name.c_str()].AddMember("slot", (uint32_t)sampler.slot, doc.GetAllocator());
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::string string = buffer.GetString();

    Vector<char> data(string.size());
    memcpy(data.data(), string.data(), string.size());
    return data;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletShaderProgram VioletShaderManager::JSonToShaderProgram(Vector<char> data)
  {
    rapidjson::Document doc;
    const auto& parse_error = doc.Parse(data.data(), data.size());
    LMB_ASSERT(!parse_error.HasParseError(), "TODO (Hilze): Fill in!");

    VioletShaderProgram program;
    program.hash      = doc["hash"].GetUint64();
    program.file_path = lmbString(doc["file path"].GetString());
    program.stage     = (ShaderStages)doc["stage"].GetUint();

    for (auto define = doc["defines"].MemberBegin(); define != doc["defines"].MemberEnd(); ++define)
      program.defines.push_back(lmbString(define->name.GetString()));

    for (auto blob = doc["blobs"].MemberBegin(); blob != doc["blobs"].MemberEnd(); ++blob)
    {
      Vector<uint32_t> blob_data(blob->value.Size(), 0u);
      for (uint32_t i = 0u; i < blob_data.size(); ++i)
        blob_data[i] = blob->value[i].GetUint();
      program.blobs.insert(eastl::make_pair(lmbString(blob->name.GetString()), blob_data));
    }

    // Shader reflection.
    for (auto var = doc["reflection"]["inputs"].MemberBegin(); var != doc["reflection"]["inputs"].MemberEnd(); ++var)
    {
      VioletShaderReflection::Variable variable;
      variable.name   = lmbString(var->value.GetString());
      variable.type   = (VioletShaderReflection::Types)var->value["type"].GetUint();
      variable.size   = var->value["size"].GetUint();
      variable.offset = var->value["offset"].GetUint();
      variable.rows   = var->value["rows"].GetUint();
      variable.cols   = var->value["columns"].GetUint();
      program.reflection.inputs.push_back(variable);
    }
    for (auto var = doc["reflection"]["outputs"].MemberBegin(); var != doc["reflection"]["outputs"].MemberEnd(); ++var)
    {
      VioletShaderReflection::Variable variable;
      variable.name   = lmbString(var->value.GetString());
      variable.type   = (VioletShaderReflection::Types)var->value["type"].GetUint();
      variable.size   = var->value["size"].GetUint();
      variable.offset = var->value["offset"].GetUint();
      variable.rows   = var->value["rows"].GetUint();
      variable.cols   = var->value["columns"].GetUint();
      program.reflection.outputs.push_back(variable);
    }
    for (auto buf = doc["reflection"]["buffers"].MemberBegin(); buf != doc["reflection"]["buffers"].MemberEnd(); ++buf)
    {
      VioletShaderReflection::Buffer buffer;
      buffer.size = buf->value["size"].GetUint();
      buffer.slot = buf->value["size"].GetUint();
      
      for (auto var = buf->value["variables"].MemberBegin(); var != buf->value["variables"].MemberEnd(); ++var)
      {
        VioletShaderReflection::Variable variable;
        variable.name   = lmbString(var->value.GetString());
        variable.type   = (VioletShaderReflection::Types)var->value["type"].GetUint();
        variable.size   = var->value["size"].GetUint();
        variable.offset = var->value["offset"].GetUint();
        variable.rows   = var->value["rows"].GetUint();
        variable.cols   = var->value["columns"].GetUint();
        buffer.variables.push_back(variable);
      }

      program.reflection.buffers.push_back(buffer);
    }
    for (auto tex = doc["reflection"]["textures"].MemberBegin(); tex != doc["reflection"]["textures"].MemberEnd(); ++tex)
    {
      VioletShaderReflection::Texture texture;
      texture.name = lmbString(tex->name.GetString());
      texture.slot = tex->value["slot"].GetUint();
      program.reflection.textures.push_back(texture);
    }
    for (auto sam = doc["reflection"]["samplers"].MemberBegin(); sam != doc["reflection"]["samplers"].MemberEnd(); ++sam)
    {
      VioletShaderReflection::Sampler sampler;
      sampler.name = lmbString(sam->name.GetString());
      sampler.slot = sam->value["slot"].GetUint();
      program.reflection.samplers.push_back(sampler);
    }

    return program;
  }
}
