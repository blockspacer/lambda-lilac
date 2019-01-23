#include "file_system.h"
#include <fstream>
#include "console.h"
#include <memory/memory.h>

#if VIOLET_WIN32
#include <experimental/filesystem>
#endif

namespace lambda
{
  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::s_base_dir_;

  //////////////////////////////////////////////////////////////////////////////
  FILE* FileSystem::fopen(const String& file, const String& mode)
  {
    String path = FullFilePath(file);
    Vector<char> buffer;
    FILE* fp;

#if VIOLET_OSX
      fp = std::fopen(path.c_str(), mode.c_str());
#else
      fopen_s(&fp, path.c_str(), mode.c_str());
#endif
    if (!fp)
    {
      foundation::Error("FileSystem: Could not open file: " + path + ".\n");
      return nullptr;
    }
    return fp;
  }

  //////////////////////////////////////////////////////////////////////////////
  void FileSystem::fclose(FILE* file)
  {
    if (file)
      std::fclose(file);
  }

  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::FileToString(FILE* file,
                           const char* header,
                           const size_t& header_size)
  {
    if (!file)
    {
      LMB_ASSERT(false, "FileSystem: Tried to use an empty file.\n");
      return "";
    }
    fseek(file, 0, SEEK_END);
    long ret = ftell(file);
    if (ret < 0)
    {
      fclose(file);
      LMB_ASSERT(false, "FileSystem: Tried to use an empty file.\n");
      return "";
    }

    String buffer(ret, '\0');
    fseek(file, 0, SEEK_SET);
    fread((void*)buffer.data(), 1u, ret, file);
    fclose(file);

    if (header != nullptr)
    {
      for (size_t i = 0u; i < header_size; ++i)
      {
        if (buffer[i] != header[i])
        {
          foundation::Error("FileSystem: Header mismatch!");
          return "";
        }
      }
      buffer.erase(buffer.begin(), buffer.begin() + header_size);
    }

    return buffer;
  }

  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::FileToString(const String& file,
                                  const char* header,
                                  const size_t& header_size)
  {
    FILE* fp = fopen(file);
    String data = FileToString(fp, header, header_size);
    fclose(fp);
    return data;
  }

  //////////////////////////////////////////////////////////////////////////////
  Vector<char> FileSystem::FileToVector(FILE* file,
                                        const char* header,
                                        const size_t& header_size)
  {
    if (!file)
    {
      LMB_ASSERT(false, "FileSystem: Tried to use an empty file.\n");
      return Vector<char>();
    }
    fseek(file , 0, SEEK_END);
    long ret = ftell(file);
    if (ret < 0)
    {
      fclose(file);
      LMB_ASSERT(false, "FileSystem: Tried to use an empty file.\n");
      return Vector<char>();
    }

    Vector<char> buffer(ret);
    fseek(file, 0, SEEK_SET);
    fread(buffer.data(), 1u, ret, file);
    fclose(file);

    if (header != nullptr)
    {
      for (size_t i = 0u; i < header_size; ++i)
      {
        if (buffer[i] != header[i])
        {
          foundation::Error("FileSystem: Header mismatch!");
          return Vector<char>();
        }
      }
      buffer.erase(buffer.begin(), buffer.begin() + header_size);
    }

    return eastl::move(buffer);
  }

  //////////////////////////////////////////////////////////////////////////////
  Vector<char> FileSystem::FileToVector(const String& file,
                                        const char* header,
                                        const size_t& header_size)
  {
    FILE* fp = fopen(file);
    Vector<char> data = FileToVector(fp, header, header_size);
    fclose(fp);
    return eastl::move(data);
  }

  //////////////////////////////////////////////////////////////////////////////
  void FileSystem::SetBaseDir(const String& base_dir)
  {
    s_base_dir_ = base_dir;
    eastl::replace(s_base_dir_.begin(), s_base_dir_.end(), '\\', '/');
    if (s_base_dir_.back() != '/')
      s_base_dir_ += '/';
  }

  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::GetBaseDir()
  {
    return s_base_dir_;
  }

  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::FullFilePath(const String& file)
  {
     return s_base_dir_ + file;
  }

  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::MakeRelative(const String& file)
  {
    String f = file;
    eastl::replace(f.begin(), f.end(), '\\', '/');
    
    // Early out.
    String base_dir = GetBaseDir();
    if (f.size() < base_dir.size())
      return f;

    for (uint32_t i = 0u; i < base_dir.size(); ++i)
      if (f[i] != base_dir[i])
        return f;
    
    return f.substr(base_dir.size());
  }

  //////////////////////////////////////////////////////////////////////////////
  void FileSystem::WriteFile(const String& file,
                             const Vector<char>& data,
                             const Vector<char>& header)
  {
    WriteFile(file, data.data(), data.size(), header.data(), header.size());
  }

  //////////////////////////////////////////////////////////////////////////////
  void FileSystem::WriteFile(const String& file,
                             const char* data,
                             const size_t& data_size,
                             const char* header,
                             const size_t& header_size)
  {
    FILE* fp;
#if VIOLET_OSX
    fp = std::fopen(FullFilePath(file).c_str(), "wb");
#else
    fopen_s(&fp, FullFilePath(file).c_str(), "wb");
#endif
    if (fp == NULL)
    {
      String errorMessage = "Package: Could not open file: " + file + ".\n";
      LMB_ASSERT(false, errorMessage.c_str());
    }
    char* nullHeader = (char*)foundation::Memory::allocate(header_size);
    fwrite(nullHeader, header_size, 1u, fp);
    fwrite(data, data_size, 1u, fp);
    fseek(fp, 0, SEEK_SET);
    fwrite(header, header_size, 1u, fp);
    fclose(fp);
    foundation::Memory::deallocate(nullHeader);
  }

#ifdef VIOLET_WIN32
  //////////////////////////////////////////////////////////////////////////////
  uint64_t FileSystem::GetTimeStamp(const String& file)
  {
    String full_path = FullFilePath(file);
    std::experimental::filesystem::path file_path(lambda::stlString(full_path));
    std::experimental::filesystem::file_time_type time_stamp =
    std::experimental::filesystem::last_write_time(file_path);
    return (uint64_t)time_stamp.time_since_epoch().count();
  }
  
  //////////////////////////////////////////////////////////////////////////////
  Vector<String> FileSystem::GetAllFilesInFolder(String folder,
                                                 String extension)
  {
    Vector<String> files;
    std::string stl_extension = lambda::stlString(extension);
    
    for (auto& p : std::experimental::filesystem::directory_iterator(
                   FullFilePath(folder).c_str()))
    {
      if (stl_extension.empty() || p.path().extension() == stl_extension)
      {
        files.push_back(lambda::lmbString(p.path().string()));
      }
    }
    return files;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  Vector<String> FileSystem::GetAllFilesInFolderRecursive(String folder,
                                                          String extension)
  {
    Vector<String> files;
    std::string stl_extension = lambda::stlString(extension);
    
    for (auto& p : std::experimental::filesystem::recursive_directory_iterator(
                   FullFilePath(folder).c_str()))
    {
      if (stl_extension.empty() || p.path().extension() == stl_extension)
        files.push_back(lambda::lmbString(p.path().string()));
    }
    return files;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::GetExtension(const String& file)
  {
    if (file.find_last_of(".") != String::npos)
      return file.substr(file.find_last_of(".") + 1);
    return "";
  }
  
  //////////////////////////////////////////////////////////////////////////////
  bool FileSystem::DoesFileExist(const String& file)
  {
    return std::experimental::filesystem::exists(FullFilePath(file).c_str());
  }
  
  //////////////////////////////////////////////////////////////////////////////
  void FileSystem::RemoveFile(const String& file)
  {
    std::experimental::filesystem::remove(
      std::experimental::filesystem::path(FullFilePath(file).c_str())
    );
  }
#else
  //////////////////////////////////////////////////////////////////////////////
  uint64_t FileSystem::GetTimeStamp(const String& /*file*/)
  {
    return 0ul;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  Vector<String> FileSystem::GetAllFilesInFolder(String /*folder*/,
                                                 String /*extension*/)
  {
    return Vector<String>();
  }

  //////////////////////////////////////////////////////////////////////////////
  Vector<String> FileSystem::GetAllFilesInFolderRecursive(String /*folder*/,
                                                          String /*extension*/)
  {
    return Vector<String>();
  }

  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::GetExtension(const String& /*file*/)
  {
    return "";
  }
  
  //////////////////////////////////////////////////////////////////////////////
  bool FileSystem::DoesFileExist(const String& /*file*/)
  {
    return false;
  }

  //////////////////////////////////////////////////////////////////////////////
  void FileSystem::RemoveFile(const String& /*file*/)
  {
  }
#endif
}
