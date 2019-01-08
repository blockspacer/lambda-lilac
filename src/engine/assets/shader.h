#pragma once
#include "asset.h"
#include <containers/containers.h>
#include "platform/shader_variable.h"

namespace lambda
{
  namespace asset
  {
    class Shader : public IAsset
    {
    public:
      Shader();
      Shader(const Name& file, const Vector<char>& shader_bytecode);
      ~Shader();

      void setShaderVariable(const platform::ShaderVariable& variable);

      Vector<char> getBytecode() const;
      Name getFile() const;
      void clear();
      Vector<platform::ShaderVariable> getQueuedShaderVariables();

    private:
      Vector<platform::ShaderVariable> queued_shader_variables_;
      Vector<char> shader_bytecode_;
      Name file_;
    };
    typedef AssetHandle<Shader> ShaderHandle;
  }
}