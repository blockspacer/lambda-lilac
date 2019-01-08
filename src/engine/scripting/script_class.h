#pragma once
#include "script_function.h"
#include <utils/console.h>

namespace lambda
{
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    class IScriptProperty
    {
    public:
      enum AccessType
      {
        kPublic,
        kPrivate,
        kProtected
      };

    public:
      IScriptProperty(
        String name, 
        int type, 
        AccessType access_type, 
        bool is_reference) 
        : name_(name)
        , type_(type)
        , access_type_(access_type)
        , is_reference_(is_reference) {}

      void print() const
      {
        String str = "Property: " + String(name_.c_str()) + 
          " Type: " + toString(type_) + 
          " AccessType: " + toString(access_type_) + 
          " Reference: " + toString(is_reference_) + "\n";
        foundation::Info(str);
      }

    private:
      String name_;
      int type_;
      AccessType access_type_;
      bool is_reference_;
    };

    ///////////////////////////////////////////////////////////////////////////
    class IScriptClass
    {
    public:
      IScriptClass(
        String name, 
        String ns, 
        Vector<IScriptFunction> functions, 
        Vector<IScriptProperty> properties) 
        : name_(name)
        , namespace_(ns)
        , functions_(functions)
        , properties_(properties) {}

      void print() const
      {
        foundation::Info("Class: " + namespace_ + " " + name_ + "\n");

        for (const IScriptFunction& function : functions_)
        {
          foundation::Info("  ");
          function.print();
        }

        for (const IScriptProperty& property : properties_)
        {
          foundation::Info("  ");
          property.print();
        }
      }

    private:
      String name_;
      String namespace_;
      Vector<IScriptFunction> functions_;
      Vector<IScriptProperty> properties_;
    };
  }
}