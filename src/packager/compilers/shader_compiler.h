#pragma once
#include <assets/shader_manager.h>

namespace lambda
{
  struct ShaderProgramCompileInfo
  {
    String  file;
    String  entry;
    ShaderStages stage;
    Vector<String> compile_types;
    Vector<String> defines;
  };

#define VIOLET_SPIR_V "spir-v"
#define VIOLET_HLSL "hlsl"
  
  class VioletShaderCompiler : public VioletShaderManager
  {
  public:
    VioletShaderCompiler();
    void Compile(ShaderProgramCompileInfo compile_info);
    
  private:
    Vector<uint32_t> CompileSpirV(ShaderProgramCompileInfo compile_info, Vector<char> hlsl);
    Vector<uint32_t> CompileHLSL(ShaderProgramCompileInfo compile_info, Vector<uint32_t> spirv);
    VioletShaderReflection Reflect(Vector<uint32_t> spirv);
  };
}