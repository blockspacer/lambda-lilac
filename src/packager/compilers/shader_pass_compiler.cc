#include "shader_pass_compiler.h"
#include <utils/console.h>

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletShaderPassCompiler::VioletShaderPassCompiler() :
    VioletShaderPassManager()
  {
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletShaderPassCompiler::SetCompiler(VioletShaderCompiler* shader_compiler)
  {
    shader_compiler_ = shader_compiler;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletShaderPassCompiler::Compile(ShaderPassCompileInfo pass_info)
  {
    VioletShaderPass shader_pass;
    shader_pass.hash          = GetHash(pass_info.name);
    shader_pass.name          = pass_info.name;
    shader_pass.depth_enabled = pass_info.depth_enabled;
    shader_pass.blend_enabled = pass_info.blend_enabled;
    shader_pass.depth_func    = pass_info.depth_func;
    shader_pass.blend_mode    = pass_info.blend_mode;

    for (const auto& shader : pass_info.shaders)
    {
      ShaderProgramCompileInfo compile_info;
      compile_info.entry         = shader.entry;
      compile_info.file          = shader.file;
      compile_info.defines       = shader.defines;
      compile_info.compile_types = pass_info.compile_types;
      compile_info.stage         = shader.stage;
      compile_info.defines.insert(compile_info.defines.end(), pass_info.defines.begin(), pass_info.defines.end());

      if (compile_info.file.empty() || compile_info.entry.empty())
        continue;
      
      uint64_t hash = shader_compiler_->GetHash(compile_info.file, compile_info.defines, compile_info.stage);
      if (shader_compiler_->GetData(hash).empty())
        shader_compiler_->Compile(compile_info);

      shader_pass.shaders.push_back(hash);
    }

    AddPass(shader_pass);
  }
}