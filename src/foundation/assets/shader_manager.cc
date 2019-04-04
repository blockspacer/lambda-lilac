#include "shader_manager.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <utils/console.h>
#include <vector>

namespace lambda
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VioletShaderManager::VioletShaderManager()
	{
		SetMagicNumber("shader");
		SetGeneratedFilePath("generated/");
		Load();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	uint64_t VioletShaderManager::GetHash(String file_path)
	{
		return hash(file_path);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void VioletShaderManager::AddShader(VioletShader shader_program)
	{
		SaveHeader(ShaderHeaderToJSon(shader_program), shader_program.hash);

		uint32_t size = 0;
		for (uint32_t l = 0; l < VIOLET_LANG_COUNT; ++l)
			for (uint32_t s = 0; s < (uint32_t)ShaderStages::kCount; ++s)
				size += (uint32_t)shader_program.blobs[s][l].size();
		Vector<char> data(size);
		
		uint32_t offset = 0;
		for (uint32_t l = 0; l < VIOLET_LANG_COUNT; ++l)
		{
			for (uint32_t s = 0; s < (uint32_t)ShaderStages::kCount; ++s)
			{
				uint32_t size = (uint32_t)shader_program.blobs[s][l].size();
				memcpy(data.data() + offset, shader_program.blobs[s][l].data(), size);
				offset += size;
			}
		}

		SaveData(data, shader_program.hash);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VioletShader VioletShaderManager::GetShader(uint64_t hash, bool get_data)
	{
		VioletShader shader = JSonToShaderHeader(GetHeader(hash));
		if (get_data)
			getBlobs(shader);
		return shader;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void VioletShaderManager::RemoveShader(uint64_t hash)
	{
		RemoveHeader(hash);
		RemoveData(hash);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void VioletShaderManager::getBlobs(VioletShader& shader_program)
	{
		Vector<char> data = GetData(shader_program.hash);
		for (uint32_t l = 0; l < VIOLET_LANG_COUNT; ++l)
		{
			for (uint32_t s = 0; s < (uint32_t)ShaderStages::kCount; ++s)
			{
				shader_program.blobs[s][l].resize(shader_program.blob_sizes[s][l].second);
				memcpy(
					shader_program.blobs[s][l].data(),
					data.data() + shader_program.blob_sizes[s][l].first,
					shader_program.blob_sizes[s][l].second
				);
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	String shaderStageToString(ShaderStages stage)
	{
		switch (stage)
		{
		case ShaderStages::kVertex:   return "Vertex";
		case ShaderStages::kGeometry: return "Geometry";
		case ShaderStages::kPixel:    return "Pixel,";
		case ShaderStages::kCompute:  return "Compute";
		case ShaderStages::kDomain:   return "Domain";
		case ShaderStages::kHull:     return "Hull";
		default:                      return "";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ShaderStages stringToShaderStage(String str)
	{
		if (str == "Vertex")
			return ShaderStages::kVertex;
		else if (str == "Geometry")
			return ShaderStages::kGeometry;
		else if (str == "Pixel")
			return ShaderStages::kPixel;
		else if (str == "Compute")
			return ShaderStages::kCompute;
		else if (str == "Domain")
			return ShaderStages::kDomain;
		else if (str == "Hull")
			return ShaderStages::kHull;
		else
			return ShaderStages::kCount;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Vector<char> VioletShaderManager::ShaderHeaderToJSon(VioletShader shader_program)
	{
		rapidjson::Document doc;
		doc.SetObject();

		doc.AddMember("hash", shader_program.hash, doc.GetAllocator());
		doc.AddMember("file path", rapidjson::StringRef(shader_program.file_path.c_str()), doc.GetAllocator());

		// Blobs.
		rapidjson::Value lang_array(rapidjson::kArrayType);

		for (int lang_int = 0; lang_int < VIOLET_LANG_COUNT; ++lang_int)
		{
			String str;

			for (int stage_int = 0; stage_int < (int)ShaderStages::kCount; ++stage_int)
			{
				// Size.
				if (!str.empty())
					str += "|";
				uint32_t size = (uint32_t)shader_program.blobs[stage_int][lang_int].size();
				str += toString(size);
			}
			rapidjson::Value stage(rapidjson::Type::kStringType);
			stage.SetString(str.c_str(), (rapidjson::SizeType)str.size(), doc.GetAllocator());
			lang_array.PushBack(stage, doc.GetAllocator());
		}

		doc.AddMember("languages", lang_array, doc.GetAllocator());

		// Resources.
		rapidjson::Value res_array(rapidjson::kArrayType);

		for (int stage_int = 0; stage_int < (int)ShaderStages::kCount; ++stage_int)
		{
			String str;
			for (const VioletShaderResource& resource : shader_program.resources[stage_int])
			{
				if (!str.empty())
					str += ",";
				str += resource.name + "|" + toString((uint32_t)resource.type) + "|" + toString((uint32_t)resource.stage) + "|" + toString((uint32_t)resource.slot) + "|" + toString(resource.size);

				for (const auto& item : resource.items)
					str += "|" + item.name + "|" + toString(item.offset) + "|" + toString(item.size);
			}

			rapidjson::Value stage(rapidjson::Type::kStringType);
			stage.SetString(str.c_str(), (rapidjson::SizeType)str.size(), doc.GetAllocator());
			res_array.PushBack(stage, doc.GetAllocator());
		}
		
		doc.AddMember("resources", res_array, doc.GetAllocator());

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		std::string string = buffer.GetString();

		Vector<char> data(string.size());
		memcpy(data.data(), string.data(), string.size());
		return data;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VioletShader VioletShaderManager::JSonToShaderHeader(Vector<char> data)
	{
		rapidjson::Document doc;
		const auto& parse_error = doc.Parse(data.data(), data.size());
		LMB_ASSERT(!parse_error.HasParseError(), "TODO (Hilze): Fill in!");

		VioletShader program;
		program.hash = doc["hash"].GetUint64();
		program.file_path = lmbString(doc["file path"].GetString());

		const auto& languages = doc["languages"].GetArray();

		uint32_t offset = 0;
		for (int lang_int = 0; lang_int < VIOLET_LANG_COUNT; ++lang_int)
		{
			Vector<String> indices = split(languages[lang_int].GetString(), '|');
			uint32_t idx = 0;

			for (int stage_int = 0; stage_int < (int)ShaderStages::kCount; ++stage_int)
			{
				uint32_t size = std::stoul(indices[idx++].c_str());

				program.blob_sizes[stage_int][lang_int].first  = offset;
				program.blob_sizes[stage_int][lang_int].second = size;
				offset += size;
			}
		}

		const auto& resources = doc["resources"].GetArray();

		for (int stage_int = 0; stage_int < (int)ShaderStages::kCount; ++stage_int)
		{
			Vector<String> res = split(resources[stage_int].GetString(), ',');
			
			if (res.size() == 1 && res[0].empty())
				continue;

			for (const String& r : res)
			{
				Vector<String> indices = split(r, '|');
				std::vector<String> v(indices.begin(), indices.end());
				VioletShaderResource resource;

				uint8_t idx = 0;

				resource.name  = indices[idx++];
				resource.type  = (VioletShaderResourceType)std::stoul(indices[idx++].c_str());
				resource.stage = (ShaderStages)std::stoul(indices[idx++].c_str());
				resource.slot  = (uint8_t)std::stoul(indices[idx++].c_str());
				resource.size  = (uint32_t)std::stoul(indices[idx++].c_str());

				while (idx < indices.size())
				{
					VioletShaderResource::Item item;
					item.name   = indices[idx++];
					item.offset = (uint32_t)std::stoul(indices[idx++].c_str());
					item.size   = (uint32_t)std::stoul(indices[idx++].c_str());
					resource.items.push_back(item);
				}

				program.resources[stage_int].push_back(resource);
			}
		}
		
		return program;
	}
}
