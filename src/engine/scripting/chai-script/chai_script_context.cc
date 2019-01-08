#include "chai_script_context.h"
#include <memory/memory.h>

#pragma warning(disable: 4503)
#include "chai_script_binding.h"
#include <chaiscript/chaiscript.hpp>
#include <utils/file_system.h>

namespace lambda
{ 
  namespace scripting
  {
    bool ChaiScriptContext::initialize(const Map<String, void*>& functions)
    {
      context_ = foundation::Memory::construct<chaiscript::ChaiScript>();
      bind(context_);

      return true;
    }

    bool ChaiScriptContext::loadScripts(const Vector<String>& files)
    {
      for (const String& file : files)
      {
        try
        {
          context_->eval_file(FileSystem::FullFilePath(file).c_str());
        }
        catch (std::exception e)
        {
          LMB_ASSERT(false, e.what());
        }
      }
      
      return true;
    }

    bool ChaiScriptContext::terminate()
    {
      foundation::Memory::destruct(context_);
      context_ = nullptr;

      return true;
    }
    
    void ChaiScriptContext::collectGarbage()
    {
    }

    ScriptValue ChaiScriptContext::executeFunction(const String& declaration, const Vector<ScriptValue>& args)
    {
      if (declaration.find("Game::") != String::npos)
      {
        std::string decl = "game." + stlString(declaration).substr(strlen("Game::"));
        decl += "(";

        for (uint8_t i = 0u; i < args.size(); ++i)
        {
          switch (args.at(i).getType())
          {
          case ScriptValue::kBoolean: decl += std::to_string(args.at(i).getBool()); break;
          case ScriptValue::kInt8:    decl += std::to_string(args.at(i).getInt8()); break;
          case ScriptValue::kUint8:   decl += std::to_string(args.at(i).getInt16()); break;
          case ScriptValue::kInt16:   decl += std::to_string(args.at(i).getInt32()); break;
          case ScriptValue::kUint16:  decl += std::to_string(args.at(i).getInt64()); break;
          case ScriptValue::kInt32:   decl += std::to_string(args.at(i).getUint8()); break;
          case ScriptValue::kUint32:  decl += std::to_string(args.at(i).getUint16()); break;
          case ScriptValue::kInt64:   decl += std::to_string(args.at(i).getUint32()); break;
          case ScriptValue::kUint64:  decl += std::to_string(args.at(i).getUint64()); break;
          case ScriptValue::kFloat:   decl += std::to_string(args.at(i).getFloat()); break;
          case ScriptValue::kDouble:  decl += std::to_string(args.at(i).getDouble()); break;
          case ScriptValue::kString:  decl += stlString(args.at(i).getString()); break;
          case ScriptValue::kVec2:    decl += args.at(i).getVec2().ToStringStd(); break;
          case ScriptValue::kVec3:    decl += args.at(i).getVec3().ToStringStd(); break;
          case ScriptValue::kVec4:    decl += args.at(i).getVec4().ToStringStd(); break;
          case ScriptValue::kNull:    decl += std::to_string(NULL); break;
          }
          if (i != args.size() - 1u)
          {
            decl += ", ";
          }
        }

        decl += ")";
        
        try
        {
          context_->eval(decl);
        }
        catch (std::exception e)
        {
          LMB_ASSERT(false, e.what());
        }
      }
      /*else
      {
        LMB_ASSERT(false, "Don't know what to do??");
      }*/

      return ScriptValue();
    }

    void ChaiScriptContext::setBreakPoint(const String& file, const int16_t& line)
    {
    }

    ScriptArray ChaiScriptContext::scriptArray(const void* data)
    {
      return ScriptArray();
    }

    void ChaiScriptContext::setWorld(world::IWorld* world)
    {
      scripting::setWorld(world);
    }

    void ChaiScriptContext::ExecuteWithDebugger()
    {
    }
  }
}
