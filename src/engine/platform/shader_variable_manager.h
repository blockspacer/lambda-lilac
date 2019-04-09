#pragma once
#include "shader_variable.h"
#include <memory/memory.h>

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    struct BufferVariable
    {
      size_t count;
      size_t size;
      Name   name;
      void*  data;
    };

    ///////////////////////////////////////////////////////////////////////////
    class ShaderBuffer
    {
    public:
      ShaderBuffer();
      ShaderBuffer(
        const Name& name, 
        const Vector<BufferVariable>& variables, 
        foundation::SharedPointer<void> data
      );
      const Vector<BufferVariable>& getVariables() const;
      Vector<BufferVariable>& getVariables();
      void setChanged(const bool& has_changed);
      bool getChanged() const;
      void* getData() const;
      Name getName() const;

    private:
      Name name_;
      bool changed_;
      Vector<BufferVariable> variables_;
      foundation::SharedPointer<void> data_;
    };

    ///////////////////////////////////////////////////////////////////////////
    class ShaderVariableManager
    {
    public:
			void operator=(const ShaderVariableManager& other);
      void setVariable(const ShaderVariable& variable);
      ShaderVariable getShaderVariable(const Name& name) const;
      bool hasShaderVariable(const Name& name) const;
      size_t getCount(const Name& name) const;
      void updateBuffer(ShaderBuffer& buffer);

    private:
      const ShaderVariable& getShaderVariable(const size_t& idx) const;
      const size_t& getCount(const size_t& idx) const;
      const size_t& getIdx(const Name& name);

    private:
      size_t last_name_;
      size_t last_idx_;
      Map<size_t, size_t> name_to_idx_;
      Vector<ShaderVariable> variables_;
      Vector<size_t> counters_;
    };
  }
}