#include "file_system.h"
#include <fstream>
#include "console.h"
#include <memory/memory.h>
#include <mutex>

#if VIOLET_WIN32
#include <experimental/filesystem>
#endif

namespace lambda
{
  //////////////////////////////////////////////////////////////////////////////
  const char* FileSystem::s_base_dir_ = nullptr;
  std::mutex k_mutex;

  //////////////////////////////////////////////////////////////////////////////
  FILE* FileSystem::fopen(const String& file, const String& mode)
  {
	  k_mutex.lock();
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
		k_mutex.unlock();
      foundation::Error("FileSystem: Could not open file: " + path + ".\n");
      return nullptr;
    }
	k_mutex.unlock();
    return fp;
  }

  //////////////////////////////////////////////////////////////////////////////
  void FileSystem::fclose(FILE* file)
  {
	  k_mutex.lock();
    if (file)
      std::fclose(file);
	k_mutex.unlock();
  }

  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::FileToString(FILE* file,
                           const char* header,
                           const size_t& header_size)
  {
	  k_mutex.lock();
    if (!file)
    {
      LMB_ASSERT(false, "FileSystem: Tried to use an empty file.\n");
      return "";
    }
    fseek(file, 0, SEEK_END);
    long ret = ftell(file);
    if (ret < 0)
    {
		k_mutex.unlock();
      fclose(file);
      LMB_ASSERT(false, "FileSystem: Tried to use an empty file.\n");
      return "";
    }

    String buffer(ret, '\0');
    fseek(file, 0, SEEK_SET);
    fread((void*)buffer.data(), 1u, ret, file);
	k_mutex.unlock();
	fclose(file);
	k_mutex.lock();

    if (header != nullptr)
    {
      for (size_t i = 0u; i < header_size; ++i)
      {
        if (buffer[i] != header[i])
        {
          foundation::Error("FileSystem: Header mismatch!");
		  k_mutex.unlock();
		  return "";
        }
      }
      buffer.erase(buffer.begin(), buffer.begin() + header_size);
    }

	k_mutex.unlock();
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
	  k_mutex.lock();
	  if (!file)
    {
      LMB_ASSERT(false, "FileSystem: Tried to use an empty file.\n");
      return Vector<char>();
    }
    fseek(file , 0, SEEK_END);
    long ret = ftell(file);
    if (ret < 0)
    {
		k_mutex.unlock();
		fclose(file);
      LMB_ASSERT(false, "FileSystem: Tried to use an empty file.\n");
      return Vector<char>();
    }

    Vector<char> buffer(ret);
    fseek(file, 0, SEEK_SET);
    fread(buffer.data(), 1u, ret, file);
	k_mutex.unlock();
	fclose(file);
	k_mutex.lock();

    if (header != nullptr)
    {
      for (size_t i = 0u; i < header_size; ++i)
      {
        if (buffer[i] != header[i])
        {
          foundation::Error("FileSystem: Header mismatch!");
		  k_mutex.unlock();
		  return Vector<char>();
        }
      }
      buffer.erase(buffer.begin(), buffer.begin() + header_size);
    }

	k_mutex.unlock();
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
	  if (s_base_dir_)
	  {
		  foundation::Memory::deallocate((void*)s_base_dir_);
		  s_base_dir_ = nullptr;
	  }

	  if (!base_dir.empty())
	  {
		  String str = FixFilePath(base_dir);
		  if (str.back() != '/')
			  str += '/';

		  s_base_dir_ = (const char*)foundation::Memory::allocate(str.size() + 1);
		  memcpy((void*)s_base_dir_, str.c_str(), str.size() + 1);
	  }
  }

  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::GetBaseDir()
  {
    return s_base_dir_;
  }

  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::FullFilePath(const String& file)
  {
     String f = FixFilePath(file);

     return s_base_dir_ + f;
  }

  //////////////////////////////////////////////////////////////////////////////
  String FileSystem::MakeRelative(const String& file)
  {
    String f = FixFilePath(file);
    
    // Early out.
    String base_dir = GetBaseDir();
    if (f.size() < base_dir.size())
      return f;

    for (uint32_t i = 0u; i < base_dir.size(); ++i)
      if (f[i] != base_dir[i])
        return f;
    
    return f.substr(base_dir.size());
  }

  String FileSystem::FixFilePath(const String& file)
  {
	  String f = file;
	  eastl::replace(f.begin(), f.end(), '\\', '/');

	  size_t idx = String::npos;
	  do
	  {
		  idx = f.find("..");
		  if (idx != String::npos)
		  {
			  size_t t_idx = f.substr(0, idx - 2).find_last_of("/");
			  size_t t_beg = t_idx;
			  size_t t_end = idx + 2;
			  String new_str = f.substr(0, t_beg) + f.substr(t_end);
			  f = new_str;
		  }
	  } while (idx != String::npos);

	  return f;
  }

	//////////////////////////////////////////////////////////////////////////////
	String FileSystem::RemoveName(const String& file)
	{
		String f = FixFilePath(file);
		
		f = f.substr(0, f.find_last_of('/'));

		if (f.back() != '/')
			f += "/";

		return f;
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
    fp = fopen(FullFilePath(file).c_str(), "wb");
#endif
    if (fp == NULL)
    {
      String errorMessage = "Package: Could not open file: " + file + ".\n";
      LMB_ASSERT(false, errorMessage.c_str());
    }
	k_mutex.lock();
	char* nullHeader = (char*)foundation::Memory::allocate(header_size);
    fwrite(nullHeader, header_size, 1u, fp);
    fwrite(data, data_size, 1u, fp);
    fseek(fp, 0, SEEK_SET);
    fwrite(header, header_size, 1u, fp);
    fclose(fp);
	foundation::Memory::deallocate(nullHeader);
	k_mutex.unlock();
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
	uint64_t FileSystem::GetFileSize(const String & file)
	{
		String full_path = FullFilePath(file);
		std::experimental::filesystem::path file_path(lambda::stlString(full_path));
		return (uint64_t)std::experimental::filesystem::file_size(file_path);
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
		String f = FixFilePath(file);

		eastl_size_t last_dot   = f.find_last_of(".");
		eastl_size_t last_slash = f.find_last_of("/");

		if (last_dot != String::npos && (last_dot > last_slash))
			return f.substr(last_dot + 1);
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
	uint64_t FileSystem::GetFileSize(const String & file)
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
