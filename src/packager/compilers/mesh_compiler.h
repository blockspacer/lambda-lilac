#pragma once
#include <assets/mesh_manager.h>

namespace lambda
{
  struct MeshCompileInfo
  {
    String file;
  };

  class VioletMeshCompiler : public VioletMeshManager
  {
  public:
    VioletMeshCompiler();
    bool Compile(MeshCompileInfo mesh_info);
  };
}