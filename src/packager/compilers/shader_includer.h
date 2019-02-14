//#pragma once
//#include "../glslang/glslang/Public/ShaderLang.h"
//#include <containers/containers.h>
//
//namespace lambda
//{
//  class VioletGLSLangIncluder : public glslang::TShader::Includer
//  {
//  public:
//    VioletGLSLangIncluder();
//    virtual ~VioletGLSLangIncluder() override;
//    virtual IncludeResult* includeLocal(const char* header_name, const char* includer_name, uint64_t inclusion_depth) override;
//    virtual void releaseInclude(IncludeResult* result) override;
//    void AddIncludeDirectory(const char* path);
//
//  private:
//    IncludeResult* ReadLocalPath(const char* header_name, const char* includer_name, int depth);
//    IncludeResult* NewIncludeResult(const String& path) const;
//    String getDirectory(const String& path) const;
//
//  private:
//    Vector<String> directory_stack_;
//    int external_local_directory_count_;
//  };
//}