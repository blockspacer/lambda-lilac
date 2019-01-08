#include "shader_compiler.h"
#include <iostream> // TODO (Hilze): Remove.
#include "shader_includer.h"
#include "../glslang/SPIRV/GlslangToSpv.h"
#include "../glslang/glslang/Public/ShaderLang.h"
#include "spirv_cross.hpp"
#include "spirv_hlsl.hpp"
#include <d3dcompiler.h>
#include <utils/file_system.h>
#include <utils/console.h>

#define PS_SPIRV_VERSION glslang::EShTargetLanguageVersion::EShTargetSpv_1_0
#define PS_VULKAN_VERSION glslang::EShTargetClientVersion::EShTargetVulkan_1_0

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .limits = */ {
    /* .nonInductiveForLoops = */ 1,
    ///* .whileLoops = */ 1,
    ///* .doWhileLoops = */ 1,
    ///* .generalUniformIndexing = */ 1,
    ///* .generalAttributeMatrixVectorIndexing = */ 1,
    ///* .generalVaryingIndexing = */ 1,
    ///* .generalSamplerIndexing = */ 1,
    ///* .generalVariableIndexing = */ 1,
    ///* .generalConstantMatrixVectorIndexing = */ 1,
    }
};

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletShaderCompiler::VioletShaderCompiler() :
    VioletShaderManager()
  {
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletShaderCompiler::Compile(ShaderProgramCompileInfo compile_info)
  {
    // Invalid compile configuration.
    if (compile_info.compile_types.empty())
    {
      LMB_ASSERT(false, "Shader: No compile configuration was specified.");
      return;
    }

    bool to_spirv = (eastl::find(compile_info.compile_types.begin(), compile_info.compile_types.end(), VIOLET_SPIR_V) != compile_info.compile_types.end());
    bool to_hlsl = (eastl::find(compile_info.compile_types.begin(), compile_info.compile_types.end(), VIOLET_HLSL) != compile_info.compile_types.end());
    if (!to_spirv && !to_hlsl)
    {
      LMB_ASSERT(false, "Shader: No valid compile configuration was specified.");
      return;
    }

    Vector<char> src = FileSystem::FileToVector(compile_info.file);
    for (String define : compile_info.defines)
    {
      define = "#define " + define + "\n";
      src.insert(src.begin(), define.begin(), define.end());
    }

    VioletShaderProgram program;
    program.defines   = compile_info.defines;
    program.file_path = compile_info.file;
    program.stage     = compile_info.stage;
    program.hash      = GetHash(program.file_path, program.defines, program.stage);

    Vector<uint32_t> spirv = CompileSpirV(compile_info, src);

    if (to_spirv)
    {
      program.blobs[VIOLET_SPIR_V] = spirv;
    }
    if (to_hlsl)
    {
      program.blobs[VIOLET_HLSL] = CompileHLSL(compile_info, spirv);
    }
    program.reflection = Reflect(spirv);

    AddShaderProgram(program);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Vector<uint32_t> VioletShaderCompiler::CompileSpirV(ShaderProgramCompileInfo compile_info, Vector<char> hlsl)
  {
    Vector<uint32_t> data;

    glslang::InitializeProcess();

    EShLanguage language = EShLanguage::EShLangVertex;
    switch (compile_info.stage)
    {
    case ShaderStages::kCompute:
      language = EShLanguage::EShLangCompute;
      break;
    case ShaderStages::kDomain:
      language = EShLanguage::EShLangTessEvaluation;
      break;
    case ShaderStages::kGeometry:
      language = EShLanguage::EShLangGeometry;
      break;
    case ShaderStages::kHull:
      language = EShLanguage::EShLangTessControl;
      break;
    case ShaderStages::kPixel:
      language = EShLanguage::EShLangFragment;
      break;
    case ShaderStages::kVertex:
      language = EShLanguage::EShLangVertex;
      break;
    }

    glslang::TProgram* tprogram = new glslang::TProgram();
    glslang::TShader* tshader = new glslang::TShader(language);
    Vector<const char*> strings = {
      hlsl.data()
    };
    Vector<int> lengths = {
      (int)hlsl.size()
    };
    const char* name[] = { compile_info.entry.c_str() };

    tshader->setEntryPoint(compile_info.entry.c_str());
    tshader->setStringsWithLengthsAndNames(strings.data(), lengths.data(), name, (int)strings.size());

    tshader->setEnvInput(glslang::EShSource::EShSourceHlsl, language, glslang::EShClient::EShClientVulkan, PS_VULKAN_VERSION);
    tshader->setEnvClient(glslang::EShClient::EShClientVulkan, PS_VULKAN_VERSION);
    tshader->setEnvTarget(glslang::EShTargetLanguage::EshTargetSpv, PS_SPIRV_VERSION);

    std::string output;
    VioletGLSLangIncluder includer;
    EShMessages messages = EShMessages::EShMsgDefault;
    messages = (EShMessages)(messages | EShMsgSpvRules);
    messages = (EShMessages)(messages | EShMsgVulkanRules);
    messages = (EShMessages)(messages | EShMsgReadHlsl);
    messages = (EShMessages)(messages | EShMsgHlslOffsets);

    if (tshader->preprocess(&DefaultTBuiltInResource, 110, EProfile::ENoProfile, false, false, messages, &output, includer) == false)
    {
      foundation::Debug(tshader->getInfoLog());
      foundation::Debug(tshader->getInfoDebugLog());
      return data;
    }

    if (tshader->parse(&DefaultTBuiltInResource, 110, false, messages, includer) == false)
    {
      foundation::Debug(tshader->getInfoLog());
      foundation::Debug(tshader->getInfoDebugLog());
      return data;
    }

    tprogram->addShader(tshader);
    foundation::Debug(tprogram->getInfoLog());
    foundation::Debug(tprogram->getInfoDebugLog());

    tprogram->link(messages);
    foundation::Debug(tprogram->getInfoLog());
    foundation::Debug(tprogram->getInfoDebugLog());

    std::vector<unsigned int> tmp;
    if (tprogram->getIntermediate(language) != nullptr)
    {
      String warnings;
      spv::SpvBuildLogger logger;
      glslang::SpvOptions spv_options;
      spv_options.disableOptimizer  = true;
      spv_options.optimizeSize      = false;
      spv_options.generateDebugInfo = true;

      glslang::GlslangToSpv(*tprogram->getIntermediate(language), tmp, &logger, &spv_options);
      uint64_t size = tmp.size() * sizeof(unsigned int);
      data.resize(size / sizeof(uint32_t), '\0');
      memcpy_s(data.data(), size, tmp.data(), size);
    }

    delete tshader;
    delete tprogram;
    glslang::FinalizeProcess();

    return data;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Vector<uint32_t> VioletShaderCompiler::CompileHLSL(ShaderProgramCompileInfo compile_info, Vector<uint32_t> spirv)
  {
		// Get the correct shader stage.
		spv::ExecutionModel execution_model;
		String target;
		switch (compile_info.stage)
		{
		case ShaderStages::kVertex:   target = "vs_5_1"; execution_model = spv::ExecutionModel::ExecutionModelVertex; break;
		case ShaderStages::kGeometry: target = "gs_5_1"; execution_model = spv::ExecutionModel::ExecutionModelGeometry; break;
		case ShaderStages::kPixel:    target = "ps_5_1"; execution_model = spv::ExecutionModel::ExecutionModelFragment; break;
		case ShaderStages::kCompute:  target = "cs_5_1"; execution_model = spv::ExecutionModel::ExecutionModelGLCompute; break;
		case ShaderStages::kDomain:   target = "ds_5_1"; execution_model = spv::ExecutionModel::ExecutionModelTessellationEvaluation; break;
		case ShaderStages::kHull:     target = "hs_5_1"; execution_model = spv::ExecutionModel::ExecutionModelTessellationControl; break;
		}

    Vector<uint32_t> data;
    String hlsl;

    // Convert SPIR-V to HLSL.
    spirv_cross::CompilerHLSL compilers(spirv.data(), spirv.size());
    compilers.set_entry_point(compile_info.entry.c_str(), execution_model);
    spirv_cross::CompilerHLSL::Options options;
    options.shader_model = 600;
    compilers.set_hlsl_options(options);
    hlsl = compilers.compile().c_str();

    // Compile the HLSL.
    ID3D10Blob* hlsl_blob, *error_blob;
    if (FAILED(D3DCompile(reinterpret_cast<LPCVOID>(hlsl.data()), hlsl.size(), NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", target.c_str(), 0, 0, &hlsl_blob, &error_blob)))
    {
      // Something failed.
      String error(error_blob->GetBufferSize(), '\0');
      memcpy((void*)error.data(), error_blob->GetBufferPointer(), error_blob->GetBufferSize());
      error = "HLSL Compile error: " + error;
      LMB_ASSERT(false, error.c_str());
    }
    else
    {
      // Add the newly generated blob to the back of the blobs.
      data.resize(hlsl_blob->GetBufferSize() / sizeof(uint32_t));
      memcpy(data.data(), hlsl_blob->GetBufferPointer(), hlsl_blob->GetBufferSize());
    }

    // Release the blobs.
    if (hlsl_blob  != NULL) hlsl_blob->Release();
    if (error_blob != NULL) error_blob->Release();

    return data;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletShaderReflection VioletShaderCompiler::Reflect(Vector<uint32_t> spirv)
  {
    spirv_cross::Compiler compiler(spirv.data(), spirv.size());
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    auto spir_v_type_conversion = [](spirv_cross::SPIRType type) -> VioletShaderReflection::Types
    {
      switch (type.basetype)
      {
      case spirv_cross::SPIRType::BaseType::AtomicCounter: return VioletShaderReflection::Types::kAtomicCounter;
      case spirv_cross::SPIRType::BaseType::Boolean:       return VioletShaderReflection::Types::kBoolean;
      case spirv_cross::SPIRType::BaseType::Char:          return VioletShaderReflection::Types::kChar;
      case spirv_cross::SPIRType::BaseType::Double:        return VioletShaderReflection::Types::kDouble;
      case spirv_cross::SPIRType::BaseType::Float:         return VioletShaderReflection::Types::kFloat;
      case spirv_cross::SPIRType::BaseType::Image:         return VioletShaderReflection::Types::kImage;
      case spirv_cross::SPIRType::BaseType::Int:           return VioletShaderReflection::Types::kInt;
      case spirv_cross::SPIRType::BaseType::Int64:         return VioletShaderReflection::Types::kInt64;
      case spirv_cross::SPIRType::BaseType::SampledImage:  return VioletShaderReflection::Types::kSampledImage;
      case spirv_cross::SPIRType::BaseType::Sampler:       return VioletShaderReflection::Types::kSampler;
      case spirv_cross::SPIRType::BaseType::Struct:        return VioletShaderReflection::Types::kStruct;
      case spirv_cross::SPIRType::BaseType::UInt:          return VioletShaderReflection::Types::kUInt;
      case spirv_cross::SPIRType::BaseType::UInt64:        return VioletShaderReflection::Types::kUInt64;
      case spirv_cross::SPIRType::BaseType::Void:          return VioletShaderReflection::Types::kVoid;
      default:                                                    
      case spirv_cross::SPIRType::BaseType::Unknown:       return VioletShaderReflection::Types::kUnknown;
      }
    };

    VioletShaderReflection reflection;
    for (const auto& buffer : resources.uniform_buffers)
    {
      if (compiler.get_active_buffer_ranges(buffer.id).size() > 0)
      {
        VioletShaderReflection::Buffer reflected_buffer;
        spirv_cross::SPIRType spv_type = compiler.get_type(buffer.type_id);
        reflected_buffer.name = buffer.name.c_str();
        reflected_buffer.slot = (uint8_t)compiler.get_decoration(buffer.id, spv::DecorationBinding);
        reflected_buffer.size = (uint32_t)compiler.get_declared_struct_size(spv_type);

        for (uint32_t i = 0; i < spv_type.member_types.size(); ++i)
        {
          VioletShaderReflection::Variable reflected_variable;
          auto type = compiler.get_type(spv_type.member_types[i]);
          reflected_variable.type = spir_v_type_conversion(type);
          reflected_variable.name = (compiler.get_member_name(spv_type.self, i).c_str());
          reflected_variable.size = (uint32_t)compiler.get_declared_struct_member_size(spv_type, i);
          reflected_variable.offset = (uint32_t)compiler.type_struct_member_offset(spv_type, i);
          reflected_variable.rows = type.vecsize;
          reflected_variable.cols = type.columns;
          reflected_buffer.variables.push_back(reflected_variable);
        }
        reflection.buffers.push_back(reflected_buffer);
      }
    }
    std::unordered_set<uint32_t> active_elements = compiler.get_active_interface_variables();
    for (const auto& image : resources.separate_images)
    {
      if (std::find(active_elements.begin(), active_elements.end(), image.id) != active_elements.end())
      {
        VioletShaderReflection::Texture reflected_texture;
        reflected_texture.name = image.name.c_str();
        reflected_texture.slot = (uint8_t)compiler.get_decoration(image.id, spv::DecorationBinding);
        reflection.textures.push_back(reflected_texture);
      }
    }
    for (const auto& sampler : resources.separate_samplers)
    {
      if (std::find(active_elements.begin(), active_elements.end(), sampler.id) != active_elements.end())
      {
        VioletShaderReflection::Sampler reflected_sampler;
        reflected_sampler.name = sampler.name.c_str();
        reflected_sampler.slot = (uint8_t)compiler.get_decoration(sampler.id, spv::DecorationBinding);
        reflection.samplers.push_back(reflected_sampler);
      }
    }
    for (const auto& input : resources.stage_inputs)
    {
      auto type = compiler.get_type(input.type_id);
      VioletShaderReflection::Variable reflected_variable;
      reflected_variable.name = input.name.substr(input.name.find_first_of('_') + 1u).c_str();
      reflected_variable.type = spir_v_type_conversion(type);
      reflected_variable.rows = type.vecsize;
      reflected_variable.cols = type.columns;
      reflection.inputs.push_back(reflected_variable);
    }
    for (const auto& output : resources.stage_outputs)
    {
      auto type = compiler.get_type(output.type_id);
      VioletShaderReflection::Variable reflected_variable;
      reflected_variable.name = output.name.substr(output.name.find('_', 1u) + 1u).c_str();
      reflected_variable.type = spir_v_type_conversion(type);
      reflected_variable.rows = type.vecsize;
      reflected_variable.cols = type.columns;
      reflection.outputs.push_back(reflected_variable);
    }

    return reflection;
  }
}