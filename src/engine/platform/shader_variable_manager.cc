#include "shader_variable_manager.h"

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    void ShaderVariableManager::setVariable(const ShaderVariable& variable)
    {
      size_t idx = getIdx(variable.name);
      if (idx == UINT64_MAX)
      {
        idx = variables_.size();
        name_to_idx_.insert(eastl::make_pair(variable.name.getHash(), idx));
        variables_.push_back(variable);
        counters_.push_back(0u);
      }

      variables_.at(idx).update(variable);
      counters_.at(idx)++;
    }
   
    ///////////////////////////////////////////////////////////////////////////
    ShaderVariable ShaderVariableManager::getShaderVariable(
      const Name& name) const
    {
      return getShaderVariable(name_to_idx_.at(name.getHash()));
    }

    ///////////////////////////////////////////////////////////////////////////
    bool ShaderVariableManager::hasShaderVariable(const Name& name) const
    {
      return (name_to_idx_.find(name.getHash()) != name_to_idx_.end());
    }

    ///////////////////////////////////////////////////////////////////////////
    const ShaderVariable& ShaderVariableManager::getShaderVariable(
      const size_t& idx) const
    {
      return variables_.at(idx);
    }

    ///////////////////////////////////////////////////////////////////////////
    size_t ShaderVariableManager::getCount(const Name& name) const
    {
      return getCount(name_to_idx_.at(name.getHash()));
    }

    ///////////////////////////////////////////////////////////////////////////
    const size_t& ShaderVariableManager::getCount(const size_t& idx) const
    {
      return counters_.at(idx);
    }

    ///////////////////////////////////////////////////////////////////////////
    void ShaderVariableManager::updateBuffer(ShaderBuffer& buffer)
    {
      for (BufferVariable& variable : buffer.getVariables())
      {
        size_t idx = getIdx(variable.name);
		auto count = getCount(idx);
        if (idx != UINT64_MAX && variable.count < count)
        {
          variable.count = getCount(idx);
          memcpy(
            variable.data, 
            getShaderVariable(idx).data.data(), 
            variable.size
          );
          buffer.setChanged(true);
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    const size_t& ShaderVariableManager::getIdx(const Name& name)
    {
      if (last_name_ != name.getHash())
      {
        last_name_ = name.getHash();
        auto it = name_to_idx_.find(name.getHash());
        if (it == name_to_idx_.end())
          last_idx_ = UINT64_MAX;
        else
          last_idx_ = it->second;
      }

      return last_idx_;
    }

    ///////////////////////////////////////////////////////////////////////////
    ShaderBuffer::ShaderBuffer()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    ShaderBuffer::ShaderBuffer(
      const Name& name, 
      const Vector<BufferVariable>& variables, 
      foundation::SharedPointer<void> data)
      : name_(name)
      , changed_(true)
      , variables_(variables)
      , data_(data)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    const Vector<BufferVariable>& ShaderBuffer::getVariables() const
    {
      return variables_;
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<BufferVariable>& ShaderBuffer::getVariables()
    {
      return variables_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void ShaderBuffer::setChanged(const bool& has_changed)
    {
      changed_ = has_changed;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool ShaderBuffer::getChanged() const
    {
      return changed_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void* ShaderBuffer::getData() const
    {
      return data_.get();
    }

    ///////////////////////////////////////////////////////////////////////////
    Name ShaderBuffer::getName() const
    {
      return name_;
    }
  }
}
