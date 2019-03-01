#pragma once
#include <assets/shader_manager.h>

namespace lambda
{
	struct ShaderCompileInfo
	{
		String file;
	};

	class VioletShaderCompiler : public VioletShaderManager
	{
	public:
		VioletShaderCompiler();
		bool Compile(ShaderCompileInfo compile_info);
	};
}