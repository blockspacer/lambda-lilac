//#pragma once
//#include "assets/asset_handle.h"
//#include "containers/containers.h"
//#include "utils/bitset.h"
//#include "shader_program.h"
//#include "texture.h"
//
//namespace lambda
//{
//  namespace asset
//  {
//    class VioletShaderPass
//    {
//    public:
//      void addProgram(VioletShaderProgramHandle program);
//      void addInput(VioletTextureHandle input);
//      void addOutput(VioletTextureHandle output);
//      Vector<VioletShaderProgramHandle> getPrograms();
//      Vector<VioletTextureHandle> getInputs();
//      Vector<VioletTextureHandle> getOutputs();
//      const Vector<VioletShaderProgramHandle>& getPrograms() const;
//      const Vector<VioletTextureHandle>& getInputs() const;
//      const Vector<VioletTextureHandle>& getOutputs() const;
//
//    private:
//      Vector<VioletShaderProgramHandle> programs_;
//      Vector<VioletTextureHandle> inputs_;
//      Vector<VioletTextureHandle> outputs_;
//    };
//    using VioletShaderPassHandle = VioletHandle<VioletShaderPass>;
//
//    class VioletShaderPassManager
//    {
//    public:
//      VioletShaderPassHandle create(Name name);
//      void destroy(VioletShaderPassHandle shader_pass);
//    };
//  }
//}