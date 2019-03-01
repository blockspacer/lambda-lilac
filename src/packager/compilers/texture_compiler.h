#pragma once
#include <assets/texture_manager.h>

namespace lambda
{
  struct TextureCompileInfo
  {
    String file;
  };

  class VioletTextureCompiler : public VioletTextureManager
  {
  public:
    VioletTextureCompiler();
    bool Compile(TextureCompileInfo texture_info);
  };
}