//#pragma once
//#include "shader_includer.h"
//#include <algorithm>
//#include "../glslang/glslang/Public/ShaderLang.h"
//#include <utils/file_system.h>
//
//namespace lambda
//{
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  VioletGLSLangIncluder::VioletGLSLangIncluder() : external_local_directory_count_(0u) {};
//  
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  VioletGLSLangIncluder::~VioletGLSLangIncluder() {};
//  
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  VioletGLSLangIncluder::IncludeResult* VioletGLSLangIncluder::includeLocal(const char* header_name,
//                                      const char* includer_name,
//                                      uint64_t inclusion_depth)
//  {
//    return ReadLocalPath(header_name, includer_name, static_cast<int>(inclusion_depth));
//  }
//  
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  void VioletGLSLangIncluder::releaseInclude(IncludeResult* result)
//  {
//    if (result != nullptr)
//    {
//      delete[] static_cast<char*>(result->userData);
//      delete result;
//    }
//  }
//  
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  void VioletGLSLangIncluder::AddIncludeDirectory(const char* path)
//  {
//    ++external_local_directory_count_;
//    directory_stack_.push_back(path);
//  }
//  
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  VioletGLSLangIncluder::IncludeResult* VioletGLSLangIncluder::ReadLocalPath(const char* header_name, const char* includer_name, int depth)
//  {
//    // initialize our directory stack when parsing at lowest level
//    directory_stack_.resize(depth + external_local_directory_count_);
//    if (depth == 1)
//    {
//      directory_stack_.back() = getDirectory(includer_name);
//    }
//
//    for(int32_t i = (int32_t)directory_stack_.size() - 1; i >= 0; --i)
//    {
//      String path = directory_stack_.at(i) + '/' + header_name;
//      std::replace(path.begin(), path.end(), '\\', '/');
//
//      FILE* fp;
//      fopen_s(&fp, path.c_str(), "r");
//      if (fp != NULL)
//      {
//        directory_stack_.push_back(getDirectory(path));
//        return NewIncludeResult(path);
//      }
//      fclose(fp);
//    }
//    return nullptr;
//  }
//
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  VioletGLSLangIncluder::IncludeResult* VioletGLSLangIncluder::NewIncludeResult(const String& path) const
//  {
//    Vector<char> file = FileSystem::FileToVector(path);
//    if (file.empty())
//    {
//      return nullptr;
//    }
//    const char* ptr = (char*)malloc(file.size());
//    memcpy((void*)ptr, file.data(), file.size());
//    return new IncludeResult(path.c_str(), ptr, file.size(), nullptr);
//  }
//
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  String VioletGLSLangIncluder::getDirectory(const String& path) const
//  {
//    uint64_t last = path.find_last_of("/\\");
//    return last == String::npos ? "." : path.substr(0, last);
//  }
//}