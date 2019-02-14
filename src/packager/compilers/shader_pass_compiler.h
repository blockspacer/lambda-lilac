//#pragma once
//#include <assets/shader_pass_manager.h>
//#include "shader_compiler.h"
//
//namespace lambda
//{
//  struct ShaderPassCompileInfo
//  {
//    struct Shader
//    {
//      String file;
//      String entry;
//      ShaderStages stage;
//      Vector<String> defines;
//    };
//
//    String name;
//    Vector<String> compile_types;
//    bool depth_enabled;
//    bool blend_enabled;
//    ShaderDepthFunc depth_func;
//    ShaderBlendMode blend_mode;
//    Vector<String> defines;
//    Vector<Shader> shaders;
//  };
//
//  class VioletShaderPassCompiler : public VioletShaderPassManager
//  {
//  public:
//    VioletShaderPassCompiler();
//    void SetCompiler(VioletShaderCompiler* shader_compiler);
//    void Compile(ShaderPassCompileInfo pass_info);
//
//  private:
//    VioletShaderCompiler* shader_compiler_ = nullptr;
//  };
//}