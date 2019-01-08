#pragma once
#include "shader.h"

namespace lambda
{
  namespace io
  {
    class ShaderIO
    {
    public:
      struct Shader
      {
        Name file;
        Vector<char> data;
      };

    public:
      static Shader load(const String& path);
      static void save(const Shader& shader, const String& path);

      static asset::Shader asAsset(const Shader& shader);
    };
  }
}