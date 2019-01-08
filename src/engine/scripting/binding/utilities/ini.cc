#include "ini.h"
#include <INIReader.h>
#include <utils/file_system.h>
#include <utils/console.h>

namespace lambda
{
  namespace scripting
  {
    namespace utilities
    {
      namespace ini
      {
        UnorderedMap<uint64_t, INIReader> g_ini_readers;
        
        uint64_t Load(const String& file_path)
        {
          String fp = FileSystem::GetBaseDir() + file_path;
          uint64_t ini_id = hash(fp);
          INIReader ini_reader(stlString(fp));
          int error = ini_reader.ParseError();
          String error_message = (error == -1 ? "Could not open file." : (error > 0 ? ("Line was unreadable: " + toString(error)) : ""));
          if (false == error_message.empty())
          {
            foundation::Error("INI: '" + error_message + "' occurred while opening '" + file_path + "'");
          }
          g_ini_readers.insert(eastl::make_pair(ini_id, ini_reader));
          return ini_id;
        }
        void Release(const uint64_t& ini_id)
        {
          g_ini_readers.erase(ini_id);
        }
        float GetVariableFloat(const uint64_t& ini_id, const String& section, const String& name)
        {
          return (float)g_ini_readers.at(ini_id).GetReal(stlString(section), stlString(name), 0.0);
        }
        double GetVariableDouble(const uint64_t& ini_id, const String& section, const String& name)
        {
          return g_ini_readers.at(ini_id).GetReal(stlString(section), stlString(name), 0.0);
        }
        bool GetVariableBool(const uint64_t& ini_id, const String& section, const String& name)
        {
          return g_ini_readers.at(ini_id).GetBoolean(stlString(section), stlString(name), false);
        }
        int32_t GetVariableInt(const uint64_t& ini_id, const String& section, const String& name)
        {
          return (int32_t)g_ini_readers.at(ini_id).GetInteger(stlString(section), stlString(name), 0);
        }
        String GetVariableString(const uint64_t& ini_id, const String& section, const String& name)
        {
          return lmbString(g_ini_readers.at(ini_id).Get(stlString(section), stlString(name), ""));
        }

        Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          return Map<lambda::String, void*>{
            { "uint64 Violet_Utilities_Ini::Load(const String&in)",                                                (void*)Load },
            { "void Violet_Utilities_Ini::Release(const uint64&in)",                                               (void*)Release },
            { "bool Violet_Utilities_Ini::GetVariableBool(const uint64&in, const String&in, const String&in)",     (void*)GetVariableBool },
            { "uint32 Violet_Utilities_Ini::GetVariableInt(const uint64&in, const String&in, const String&in)",    (void*)GetVariableInt },
            { "float Violet_Utilities_Ini::GetVariableFloat(const uint64&in, const String&in, const String&in)",   (void*)GetVariableFloat },
            { "double Violet_Utilities_Ini::GetVariableDouble(const uint64&in, const String&in, const String&in)", (void*)GetVariableDouble },
            { "String Violet_Utilities_Ini::GetVariableString(const uint64&in, const String&in, const String&in)", (void*)GetVariableString },
          };
        }
        void Unbind()
        {
          g_ini_readers.clear();
        }
      }
    }
  }
}
