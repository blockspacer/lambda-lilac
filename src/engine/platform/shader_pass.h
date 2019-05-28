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
        asset::VioletShaderHandle shader,
        Vector<RenderTarget> inputs,
        Vector<RenderTarget> outputs
      );
	  ~ShaderPass() {};

      asset::VioletShaderHandle getShader() const;
      const Vector<RenderTarget>& getInputs() const;
      const Vector<RenderTarget>& getOutputs() const;

      Name getName() const;
      void setEnabled(bool enabled);
      bool getEnabled() const;

			void metaSetName(Name name);
			void metaSetShader(asset::VioletShaderHandle shader);
			void metaSetInputs(const Vector<RenderTarget>& inputs);
			void metaSetOutputs(const Vector<RenderTarget>& outputs);

    private:
      bool enabled_;
      Name name_;
      asset::VioletShaderHandle shader_;
      Vector<RenderTarget> inputs_;
      Vector<RenderTarget> outputs_;
    };
  }
}
