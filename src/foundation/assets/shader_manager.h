#pragma once
#include "base_asset_manager.h"
#include <containers/containers.h>
#include "enums.h"

namespace lambda
{
#define VIOLET_HLSL 0
#define VIOLET_DXIL 1
#define VIOLET_SPIRV 2
#define VIOLET_METAL 3
#define VIOLET_LANG_COUNT 4

	struct VioletShader
	{
		uint64_t hash;
		String file_path;
		Vector<Vector<Vector<uint32_t>>> blobs;
	};

	class VioletShaderManager : public VioletBaseAssetManager
	{
	public:
		VioletShaderManager();

		uint64_t GetHash(String file_path);

		void AddShader(VioletShader shader_program);
		VioletShader GetShader(uint64_t hash);
		void RemoveShader(uint64_t hash);

	private:
		Vector<char> ShaderToJSon(VioletShader shader_program);
		VioletShader JSonToShader(Vector<char> json);
	};
}