#pragma once
#include "base_asset_manager.h"
#include <containers/containers.h>
#include "enums.h"

namespace lambda
{
  struct VioletShaderReflection
  {
    enum Types : uint8_t
    {
      kAtomicCounter,
      kBoolean,
      kChar,
      kDouble,
      kFloat,
      kImage,
      kInt,
      kInt64,
      kSampledImage,
      kSampler,
      kStruct,
      kUInt,
      kUInt64,
      kVoid,
      kUnknown
    };
    struct Variable
    {
      String   name;
      Types    type;
      uint32_t size;
      uint32_t offset;
      uint8_t  rows;
      uint8_t  cols;
    };
    struct Buffer
    {
      String   name;
      uint32_t size;
      uint8_t  slot;
      Vector<Variable> variables;
    };
    struct Texture
    {
      String  name;
      uint8_t slot;
    };
    struct Sampler
    {
      String  name;
      uint8_t slot;
    };
    Vector<Variable> inputs;
    Vector<Variable> outputs;
    Vector<Buffer>   buffers;
    Vector<Texture>  textures;
    Vector<Sampler>  samplers;
  };

  struct VioletShaderProgram
  {
    uint64_t hash;
    String file_path;
    ShaderStages stage;
    UnorderedMap<String, Vector<uint32_t>> blobs;
    Vector<String> defines;
    VioletShaderReflection reflection;
  };

  class VioletShaderManager : public VioletBaseAssetManager
  {
  public:
    VioletShaderManager();
    
    uint64_t GetHash(String file_path, Vector<String> defines, ShaderStages stage);

    void AddShaderProgram(VioletShaderProgram shader_program);
    VioletShaderProgram GetShaderProgram(uint64_t hash);

  private:
    Vector<char> ShaderProgramToJSon(VioletShaderProgram shader_program);
    VioletShaderProgram JSonToShaderProgram(Vector<char> json);
  };
}