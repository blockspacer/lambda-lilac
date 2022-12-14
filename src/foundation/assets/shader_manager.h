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

	enum class VioletShaderResourceType : uint8_t
	{
		kTexture,
		kSampler,
		kConstantBuffer,
		kVertexInput,
	};

	enum class VioletShaderComponentType : uint8_t
	{
		kUnkown,
		kFloat1,
		kFloat2,
		kFloat3,
		kFloat4,
		kInt1,
		kInt2,
		kInt3,
		kInt4,
		kUint1,
		kUint2,
		kUint3,
		kUint4,
	};

	struct VioletShaderResource
	{
		String name;
		VioletShaderResourceType type;
		ShaderStages stage;
		uint32_t slot;
		uint32_t size;

		struct Item
		{
			String name;
			uint32_t size;
			uint32_t offset;
		};
		struct Input
		{
			String name;
			uint32_t semantic_index;
			uint32_t reg;
			VioletShaderComponentType type;
		};
		Vector<Item> items;
		Vector<Input> inputs;
	};

	struct VioletShader
	{
		VioletShader() {}
		~VioletShader() {}

		uint64_t hash;
		String file_path;
		Array<Array<Pair<uint32_t, uint32_t>, VIOLET_LANG_COUNT>, (size_t)ShaderStages::kCount> blob_sizes;
		Array<Array<Vector<char>, VIOLET_LANG_COUNT>, (size_t)ShaderStages::kCount> blobs;
		Array<Vector<VioletShaderResource>, (size_t)ShaderStages::kCount> resources;
	};

	class VioletShaderManager : public VioletBaseAssetManager
	{
	public:
		VioletShaderManager();

		uint64_t GetHash(String file_path);

		void AddShader(VioletShader shader_program);
		VioletShader GetShader(uint64_t hash, bool get_data = false);
		void RemoveShader(uint64_t hash);
		void getBlobs(VioletShader& shader_program);

	private:
		Vector<char> ShaderHeaderToJSon(VioletShader shader_program);
		VioletShader JSonToShaderHeader(Vector<char> json);
	};
}