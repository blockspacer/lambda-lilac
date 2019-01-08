#pragma once
#include <containers/containers.h>
#include "script_value.h"
#include <utils/console.h>

namespace lambda
{
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    class IScriptParameter
    {
    public:
      IScriptParameter(String name, ScriptValue::Type type) :
        name_(name), type_(type) {}

    public:
      void print() const
      {
        foundation::Info("Parameter: " + name_ + 
          " Type: " + toString(type_) + "\n");
      }

      const String& getName() const
      {
        return name_;
      }
      const ScriptValue::Type& getType() const
      {
        return type_;
      }

    private:
      String name_;
      ScriptValue::Type type_;
    };

    ///////////////////////////////////////////////////////////////////////////
    class IScriptFunction
    {
    public:
      IScriptFunction(
        String name, 
        String ns, 
        Vector<IScriptParameter> parameters, 
        ScriptValue::Type return_type)
        : name_(name)
        , namespace_(ns)
        , parameters_(parameters)
        , return_type_(return_type) {}

      void print() const
      {
        foundation::Info("Function: " + namespace_ + " " 
          + name_ + " Return " + toString(return_type_) + "\n");
     
        for (const auto& input : parameters_)
        {
          foundation::Info("    ");
          input.print();
        }
      }

      const String& getName() const
      {
        return name_;
      }
      const String& getNamespace() const
      {
        return namespace_;
      }
      const Vector<IScriptParameter>& getParameters() const
      {
        return parameters_;
      }
      const ScriptValue::Type& getReturnType() const
      {
        return return_type_;
      }

    protected:
      String name_;
      String namespace_;
      Vector<IScriptParameter> parameters_;
      ScriptValue::Type return_type_;
    };
  }
}