#include "texture_manager.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <utils/console.h>

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletTextureManager::VioletTextureManager()
  {
    SetMagicNumber("tex");
    SetFilePathIndex("generated/textures");
    SetGeneratedFilePath("generated/");
    Load();
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t VioletTextureManager::GetHash(String texture_name)
  {
    return hash(texture_name);
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletTextureManager::AddTexture(VioletTexture texture)
  {
    SaveData(TextureToJSon(texture), texture.hash);
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletTexture VioletTextureManager::GetTexture(uint64_t hash)
  {
    return JSonToTexture(GetData(hash));
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletTextureManager::RemoveTexture(uint64_t hash)
  {
    RemoveData(hash);
  }

  String rapidjsonErrortoString(rapidjson::ParseErrorCode error)
  {
    switch (error)
    {
    default:
    case rapidjson::kParseErrorNone: return "kParseErrorNone";
    case rapidjson::kParseErrorDocumentEmpty: return "kParseErrorDocumentEmpty";
    case rapidjson::kParseErrorDocumentRootNotSingular: return "kParseErrorDocumentRootNotSingular";
    case rapidjson::kParseErrorValueInvalid: return "kParseErrorValueInvalid";
    case rapidjson::kParseErrorObjectMissName: return "kParseErrorObjectMissName";
    case rapidjson::kParseErrorObjectMissColon: return "kParseErrorObjectMissColon";
    case rapidjson::kParseErrorObjectMissCommaOrCurlyBracket: return "kParseErrorObjectMissCommaOrCurlyBracket";
    case rapidjson::kParseErrorArrayMissCommaOrSquareBracket: return "kParseErrorArrayMissCommaOrSquareBracket";
    case rapidjson::kParseErrorStringUnicodeEscapeInvalidHex: return "kParseErrorStringUnicodeEscapeInvalidHex";
    case rapidjson::kParseErrorStringUnicodeSurrogateInvalid: return "kParseErrorStringUnicodeSurrogateInvalid";
    case rapidjson::kParseErrorStringEscapeInvalid: return "kParseErrorStringEscapeInvalid";
    case rapidjson::kParseErrorStringMissQuotationMark: return "kParseErrorStringMissQuotationMark";
    case rapidjson::kParseErrorStringInvalidEncoding: return "kParseErrorStringInvalidEncoding";
    case rapidjson::kParseErrorNumberTooBig: return "kParseErrorNumberTooBig";
    case rapidjson::kParseErrorNumberMissFraction: return "kParseErrorNumberMissFraction";
    case rapidjson::kParseErrorNumberMissExponent: return "kParseErrorNumberMissExponent";
    case rapidjson::kParseErrorTermination: return "kParseErrorTermination";
    case rapidjson::kParseErrorUnspecificSyntaxError: return "kParseErrorUnspecificSyntaxError";
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletTexture VioletTextureManager::JSonToTexture(Vector<char> data)
  {
    rapidjson::Document doc;
    const auto& parse_error = doc.Parse(data.data(), data.size());
    LMB_ASSERT(!parse_error.HasParseError(), rapidjsonErrortoString(parse_error.GetParseError()).c_str());

    VioletTexture texture;
    texture.hash      = doc["hash"].GetUint64();
    texture.file      = lmbString(doc["file"].GetString());
    texture.width     = doc["width"].GetUint();
    texture.height    = doc["height"].GetUint();
    texture.flags     = doc["flags"].GetUint();
    texture.mip_count = doc["mip count"].GetUint();
    texture.format    = (TextureFormat)doc["format"].GetUint();
    texture.data.resize(doc["data"].Size());
    for (uint32_t i = 0u; i < texture.data.size(); ++i)
      texture.data[i] = doc["data"][i].GetUint();

    return texture;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Vector<char> VioletTextureManager::TextureToJSon(VioletTexture texture)
  {
    rapidjson::Document doc;
    doc.SetObject();

    doc.AddMember("hash",      texture.hash, doc.GetAllocator());
    doc.AddMember("file",      rapidjson::StringRef(texture.file.c_str()), doc.GetAllocator());
    doc.AddMember("width",     texture.width, doc.GetAllocator());
    doc.AddMember("height",    texture.height, doc.GetAllocator());
    doc.AddMember("flags",     texture.flags, doc.GetAllocator());
    doc.AddMember("mip count", texture.mip_count, doc.GetAllocator());
    doc.AddMember("format",    (uint8_t)texture.format, doc.GetAllocator());
    doc.AddMember("data",      rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());
    for (uint32_t i = 0u; i < texture.data.size(); ++i)
      doc["data"].PushBack(texture.data[i], doc.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::string string = buffer.GetString();

    Vector<char> data(string.size());
    memcpy(data.data(), string.data(), string.size());
    return data;
  }
}
