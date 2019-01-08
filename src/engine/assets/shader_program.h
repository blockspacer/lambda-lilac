#pragma once
#include "assets/asset_handle.h"
#include "containers/containers.h"
#include "utils/bitset.h"
#include "assets/shader_manager.h"
#include <assets/enums.h>

namespace lambda
{
  namespace asset
  {
    class ShaderProgram
    {
    public:
      ShaderProgram(VioletShaderProgram program);
      void setFlag(const VioletFlags& flag, bool value);
      bool getFlag(const VioletFlags& flag) const;

      const VioletShaderProgram& getProgram() const;
      String getPath() const;
      Vector<String> getDefines() const;
      ShaderStages getStage() const;

    private:
      utilities::BitSet bit_set_;
      VioletShaderProgram program_;
    };
    using VioletShaderProgramHandle = VioletHandle<ShaderProgram>;

    class ShaderProgramManager
    {
    public:
      friend class ShaderProgram;
      VioletShaderProgramHandle create(Name name, ShaderProgram shader_program);
      VioletShaderProgramHandle get(uint64_t hash);
      void destroy(VioletShaderProgramHandle shader_program);

    public:
      static ShaderProgramManager* getInstance();

    protected:
      VioletShaderManager& getManager();
      const VioletShaderManager& getManager() const;
    
    private:
      VioletShaderManager manager_;
    };
  }
}