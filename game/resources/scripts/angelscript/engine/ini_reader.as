/** @file ini_reader.as */

namespace Utility //! Namespace containing all utility classes, enums and functions
{
  /**
  * @class Utility::IniReader
  * @brief Helper class for the reading of .ini files
  * @author Hilze Vonck
  **/
  class IniReader
  {
    /**
    * @brief Constructor which will load a .ini file based on the specified file path
    * @param file_path (const String) The .inif file that needs to be loaded
    * @public
    **/
    IniReader(const String&in file_path)
    {
      id = Violet_Utilities_Ini::Load(file_path);
    }
    /**
    * @brief Releases the currently loaded .ini file
    * @public
    **/
    void Release()
    {
      Violet_Utilities_Ini::Release(id);
    }
    /**
    * @brief Returns a boolean value gotten from the specified section and name
    * @param section (const String) The section where it needs to look
    * @param name (const String) The name of the value that it needs to find in the section
    * @return (bool) The requested boolean value
    * @public
    **/
    bool GetBool(const String&in section, const String&in name)
    {
      return Violet_Utilities_Ini::GetVariableBool(id, section, name);
    }
    /**
    * @brief Returns an int32 value gotten from the specified section and name
    * @param section (const String) The section where it needs to look
    * @param name (const String) The name of the value that it needs to find in the section
    * @return (int32) The requested int32 value
    * @public
    **/
    int32 GetInt(const String&in section, const String&in name)
    {
      return Violet_Utilities_Ini::GetVariableInt(id, section, name);
    }
    /**
    * @brief Returns a float value gotten from the specified section and name
    * @param section (const String) The section where it needs to look
    * @param name (const String) The name of the value that it needs to find in the section
    * @return (float) The requested float value
    * @public
    **/
    float GetFloat(const String&in section, const String&in name)
    {
      return Violet_Utilities_Ini::GetVariableFloat(id, section, name);
    }
    /**
    * @brief Returns a double value gotten from the specified section and name
    * @param section (const String) The section where it needs to look
    * @param name (const String) The name of the value that it needs to find in the section
    * @return (double) The requested double value
    * @public
    **/
    double GetDouble(const String&in section, const String&in name)
    {
      return Violet_Utilities_Ini::GetVariableDouble(id, section, name);
    }
    /**
    * @brief Returns a String value gotten from the specified section and name
    * @param section (const String) The section where it needs to look
    * @param name (const String) The name of the value that it needs to find in the section
    * @return (String) The requested String value
    * @public
    **/
    String GetString(const String&in section, const String&in name)
    {
      return Violet_Utilities_Ini::GetVariableString(id, section, name);
    }

    private uint64 id; //!< The engine id of the loaded .ini file
  }
}
