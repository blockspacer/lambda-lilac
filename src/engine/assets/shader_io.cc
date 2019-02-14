//#include "shader_io.h"
//#include <fstream>
//#include <utils/console.h>
//#include <utils/file_system.h>
//
//namespace lambda
//{
//  namespace io
//  {
//    ShaderIO::Shader ShaderIO::load(const String& path)
//    {
//      String _path(path.size(), '\0');
//      memcpy((void*)_path.c_str(), path.c_str(), path.size());
//
//      Shader shader;
//      shader.file = _path;
//
//      Vector<char> data = FileSystem::FileToVector(_path);
//      if(data.empty())
//      {
//        foundation::Error("ShaderIO: Failed to load shader: " + _path + "\n");
//        assert(true);
//      }
//      shader.data = eastl::move(data);
//      return shader;
//    }
//    void ShaderIO::save(const Shader& shader, const String& path)
//    {
//      std::ofstream fout(path.c_str());
//      fout.clear();
//      fout.write(shader.data.data(), shader.data.size());
//      fout.close();
//    }
//    asset::Shader ShaderIO::asAsset(const Shader& shader)
//    {
//      return asset::Shader(shader.file, shader.data);
//    }
//  }
//}