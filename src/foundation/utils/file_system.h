#pragma once
#include "containers/containers.h"

namespace lambda
{
  class FileSystem
  {
  public:
    static void SetBaseDir(const String& base_dir);
    static String GetBaseDir();
    
    static String FullFilePath(const String& file);
    static String MakeRelative(const String& file);

    static FILE* fopen(const String& file, const String& mode = "rb");
    static void fclose(FILE* file);
    
    static String FileToString(FILE* file, const char* header = nullptr, const size_t& header_size = 0u);
    static String FileToString(const String& file, const char* header = nullptr, const size_t& header_size = 0u);
    static Vector<char> FileToVector(FILE* file, const char* header = nullptr, const size_t& header_size = 0u);
    static Vector<char> FileToVector(const String& file, const char* header = nullptr, const size_t& header_size = 0u);
    
    static void WriteFile(const String& file, const Vector<char>& data, const Vector<char>& header = Vector<char>());
    static void WriteFile(const String& file, const char* data, const size_t& data_size, const char* header = nullptr, const size_t& header_size = 0u);
    
    static uint64_t GetTimeStamp(const String& file);
    static Vector<String> GetAllFilesInFolder(String folder = "", String extension = "");
    static Vector<String> GetAllFilesInFolderRecursive(String folder = "", String extension = "");
    static String GetExtension(const String& file);
    static bool DoesFileExist(const String& file);
    static void RemoveFile(const String& file);

  private:
    static String s_base_dir_;
  };
}