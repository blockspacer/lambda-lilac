#pragma once
#include "assets/shader.h"
#include "platform/render_target.h"

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    class ShaderPass
    {
    public:
      ShaderPass();
      ShaderPass(
        Name name,
        asset::ShaderHandle shader,
        Vector<RenderTarget> inputs,
        Vector<RenderTarget> outputs
      );

      asset::ShaderHandle getShader() const;
      const Vector<RenderTarget>& getInputs() const;
      const Vector<RenderTarget>& getOutputs() const;

      Name getName() const;
      void setEnabled(const bool& enabled);
      bool getEnabled() const;

    private:
      bool enabled_;
      Name name_;
      asset::ShaderHandle shader_;
      Vector<RenderTarget> inputs_;
      Vector<RenderTarget> outputs_;
    };
  }
}