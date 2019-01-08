#pragma once
#include "assets/mesh.h"

namespace lambda
{
  namespace platform
  {
    class MeshDecimator
    {
    public:
      void decimate(asset::Mesh* input, asset::Mesh* output, float reduction = 0.5f, float target_error = 1.0f);
      void decimateOld(asset::Mesh* input, asset::Mesh* output, float reduction = 0.5f, float target_error = 1.0f);

    private:
    };
  }
}