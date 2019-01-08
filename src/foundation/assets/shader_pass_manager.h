#pragma once
#include "base_asset_manager.h"
#include <containers/containers.h>
#include "enums.h"

namespace lambda
{
  struct VioletShaderPass
  {
    uint64_t hash;
    String name;
    bool depth_enabled;
    bool blend_enabled;
    ShaderDepthFunc depth_func;
    ShaderBlendMode blend_mode;
    Vector<uint64_t> shaders;
  };

  class VioletShaderPassManager : public VioletBaseAssetManager
  {
  public:
    VioletShaderPassManager();
    
    uint64_t GetHash(String pass_name);

    void AddPass(VioletShaderPass pass);
    VioletShaderPass GetPass(uint64_t hash);

  private:
    VioletShaderPass JSonToShaderPass(Vector<char> json);
    Vector<char> ShaderPassToJSon(VioletShaderPass shader_pass);
  };
}