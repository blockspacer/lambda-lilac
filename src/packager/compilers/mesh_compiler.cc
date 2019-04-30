#include "mesh_compiler.h"
#include <utils/file_system.h>
#include <utils/utilities.h>
#include <utils/console.h>
#include <memory/memory.h>
#include <algorithm>
#include <utils/decompose_matrix.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#if VIOLET_WIN32
#pragma warning(push, 0)
#endif
#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#define TINYGLTF_NO_STB_IMAGE_WRITE
//#define TINYGLTF_NO_STB_IMAGE
#include <tiny_gltf.h>
#if VIOLET_WIN32
#pragma warning(pop)
#endif

namespace lambda
{
	struct AccessorMemory
	{
		Vector<int> pos;
		Vector<int> nor;
		Vector<int> tan;
		Vector<int> col;
		Vector<int> tex;
		Vector<int> joi;
		Vector<int> wei;
		Vector<int> idx;
		Vector<int> tex_alb;
		Vector<int> tex_nor;
		Vector<int> tex_dmra;
		Vector<int> tex_emi;
	};
	struct AccessorMemoryConverter
	{
		UnorderedMap<int, int> pos;
		UnorderedMap<int, int> nor;
		UnorderedMap<int, int> tan;
		UnorderedMap<int, int> col;
		UnorderedMap<int, int> tex;
		UnorderedMap<int, int> idx;
		UnorderedMap<int, int> joi;
		UnorderedMap<int, int> wei;
		UnorderedMap<int, int> tex_alb;
		UnorderedMap<int, int> tex_nor;
		UnorderedMap<int, int> tex_dmra;
		UnorderedMap<int, int> tex_emi;
	};

	size_t getElementType(int type, int component_type)
	{
		size_t elem = 1;
		size_t scalar = 1;
		switch (type)
		{
		case TINYGLTF_TYPE_SCALAR:
			scalar = 1;
			break;
		case TINYGLTF_TYPE_VEC2:
			scalar = 2;
			break;
		case TINYGLTF_TYPE_VEC3:
			scalar = 3;
			break;
		case TINYGLTF_TYPE_VEC4:
			scalar = 4;
			break;
		case TINYGLTF_TYPE_MAT2:
			scalar = 4;
			break;
		case TINYGLTF_TYPE_MAT3:
			scalar = 9;
			break;
		case TINYGLTF_TYPE_MAT4:
			scalar = 16;
			break;
		case TINYGLTF_TYPE_VECTOR:
			foundation::Error("MeshIO: scalar type is vector. How do you handle this?\n");
			break;
		case TINYGLTF_TYPE_MATRIX:
			foundation::Error("MeshIO: scalar type is matrix. How do you handle this?\n");
			break;
		default:
			foundation::Error("MeshIO: No scalar was specified!\n");
			break;
		}
		switch (component_type)
		{
		case TINYGLTF_COMPONENT_TYPE_BYTE:
			elem = sizeof(char);
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			elem = sizeof(unsigned char);
			break;
		case TINYGLTF_COMPONENT_TYPE_SHORT:
			elem = sizeof(short);
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			elem = sizeof(unsigned short);
			break;
		case TINYGLTF_COMPONENT_TYPE_INT:
			elem = sizeof(int);
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			elem = sizeof(unsigned int);
			break;
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			elem = sizeof(float);
			break;
		case TINYGLTF_COMPONENT_TYPE_DOUBLE:
			elem = sizeof(double);
			break;
		default:
			foundation::Error("MeshIO: No type was specified!\n");
			break;
		}
		return elem * scalar;
	}
	template<typename T>
	void insert(Vector<T>& v, const T& t)
	{
		if (eastl::find(v.begin(), v.end(), t) == v.end())
		{
			v.push_back(t);
		}
	}
	template<typename V>
	V contains(const std::map<std::string, V>& m, const String& v)
	{
		if (m.empty())
		{
			return -1;
		}
		std::string std_str = stlString(v);
		auto it = m.find(std_str);
		if (it != m.end())
		{
			return it->second;
		}
		else
		{
			return -1;
		}
	}
	template<typename V>
	bool containsRaw(const std::map<std::string, V>& m, const String& v, V& ret)
	{
		if (m.empty())
		{
			return false;
		}
		std::string std_str = stlString(v);
		auto it = m.find(std_str);
		if (it != m.end())
		{
			ret = it->second;
			return true;
		}
		else
		{
			return false;
		}
	}
	template<typename T>
	void erase(Vector<T>& v, const T& t)
	{
		v.erase(eastl::find(v.begin(), v.end(), t));
	}
	template<typename T>
	void eraseIf(Vector<T>& v, const T& t)
	{
		const auto& it = eastl::find(v.begin(), v.end(), t);
		if (it != v.end())
		{
			v.erase(it);
		}
	}

	tinygltf::Model loadWorld(const String& path);
	void addNode(const tinygltf::Model& model, AccessorMemory& memory, const int& n);
	void addNode(const tinygltf::Model& model, const int& n, const int& current_mesh, Vector<VioletSubMesh>& meshes, const AccessorMemoryConverter& converter);
	void getData(const tinygltf::Model& model, const Vector<int>& accessors, VioletDataInfo& info);
	void getDataIdx(const tinygltf::Model& model, const Vector<int>& accessors, VioletDataInfo& info);
	void getTexture(const String& base_path, const String& file_name, const tinygltf::Model& model, const Vector<int>& textures, Vector<String>& handles);
	void removeAllTextures(const String& base_path, const String& file_name);

	VioletMesh loadMeshGLTF(String path)
	{
		VioletMesh converted_mesh;
		AccessorMemory memory;
		AccessorMemoryConverter converter;

		path = FileSystem::FullFilePath(path);
		
		// Get the world.
		tinygltf::Model model = loadWorld(path);

		// Fill the accessor memory by looping over all nodes.
		LMB_ASSERT(model.scenes.size() > 0, "GLTF file did not contain a scene");
		for (tinygltf::Scene scene : model.scenes)
			for (int node : scene.nodes)
				addNode(model, memory, node);

		String base_path = FileSystem::RemoveName(path);
		String file_name = FileSystem::FileName(path);

		// Erase the invalid accessors.
		eraseIf(memory.pos, -1);
		eraseIf(memory.nor, -1);
		eraseIf(memory.tan, -1);
		eraseIf(memory.col, -1);
		eraseIf(memory.tex, -1);
		eraseIf(memory.idx, -1);
		eraseIf(memory.joi, -1);
		eraseIf(memory.wei, -1);
		eraseIf(memory.tex_alb, -1);
		eraseIf(memory.tex_nor, -1);
		eraseIf(memory.tex_dmra, -1);
		eraseIf(memory.tex_emi, -1);

		// Get the used accessors.
		getData(model, memory.pos, converted_mesh.data.pos);
		getData(model, memory.nor, converted_mesh.data.nor);
		getData(model, memory.tan, converted_mesh.data.tan);
		getData(model, memory.col, converted_mesh.data.col);
		getData(model, memory.tex, converted_mesh.data.tex);
		getData(model, memory.joi, converted_mesh.data.joi);
		getData(model, memory.wei, converted_mesh.data.wei);
		getDataIdx(model, memory.idx, converted_mesh.data.idx);
		removeAllTextures(base_path, file_name);
		getTexture(base_path, file_name, model, memory.tex_alb, converted_mesh.data.tex_alb);
		getTexture(base_path, file_name, model, memory.tex_nor, converted_mesh.data.tex_nrm);
		getTexture(base_path, file_name, model, memory.tex_dmra, converted_mesh.data.tex_dmra);
		getTexture(base_path, file_name, model, memory.tex_emi, converted_mesh.data.tex_emi);

		auto convert = [](const Vector<int>& v, UnorderedMap<int, int>& m) { m.insert(eastl::make_pair(-1, -1)); for (int i = 0; i < (int)v.size(); ++i) { m.insert(eastl::make_pair(v.at(i), i)); } };
		convert(memory.pos, converter.pos);
		convert(memory.nor, converter.nor);
		convert(memory.tan, converter.tan);
		convert(memory.col, converter.col);
		convert(memory.tex, converter.tex);
		convert(memory.joi, converter.joi);
		convert(memory.wei, converter.wei);
		convert(memory.idx, converter.idx);
		convert(memory.tex_alb, converter.tex_alb);
		convert(memory.tex_nor, converter.tex_nor);
		convert(memory.tex_dmra, converter.tex_dmra);
		convert(memory.tex_emi, converter.tex_emi);

		converted_mesh.meshes.push_back(VioletSubMesh());
		for (tinygltf::Scene scene : model.scenes)
		{
			for (int node : scene.nodes)
			{
				converted_mesh.meshes.push_back(VioletSubMesh());
				addNode(model, node, (int)converted_mesh.meshes.size() - 1, converted_mesh.meshes, converter);
			}
		}

		return converted_mesh;
	}

	void getData(const tinygltf::Model& model, const Vector<int>& accessors, VioletDataInfo& info)
	{
		for (const int& a : accessors)
		{
			const tinygltf::Accessor&   accessor = model.accessors.at(a);
			const tinygltf::BufferView& buffer_view = model.bufferViews.at(accessor.bufferView);
			const tinygltf::Buffer&     buffer = model.buffers.at(buffer_view.buffer);

			size_t offset = accessor.byteOffset + buffer_view.byteOffset;
			size_t count = accessor.count;
			size_t stride = getElementType(accessor.type, accessor.componentType);

			info.segments.push_back({
				/* offset */ info.data.size(),
				/* count  */ count,
				/* stride */ stride
			});

			Vector<unsigned char> data(count * stride);
			memcpy(data.data(), buffer.data.data() + offset, count * stride);
			info.data.insert(info.data.end(), data.begin(), data.end());
		}
	}
	void getDataIdx(const tinygltf::Model& model, const Vector<int>& accessors, VioletDataInfo& info)
	{
		for (const int& a : accessors)
		{
			const tinygltf::Accessor&   accessor = model.accessors.at(a);
			const tinygltf::BufferView& buffer_view = model.bufferViews.at(accessor.bufferView);
			const tinygltf::Buffer&     buffer = model.buffers.at(buffer_view.buffer);

			size_t offset = accessor.byteOffset + buffer_view.byteOffset;
			size_t count = accessor.count;
			size_t stride = getElementType(accessor.type, accessor.componentType);

			info.segments.push_back({
				/* offset */ info.data.size(),
				/* count  */ count,
				/* stride */ sizeof(uint32_t)
			});

			if (stride == sizeof(uint32_t))
			{
				Vector<unsigned char> data(count * stride);
				memcpy(data.data(), buffer.data.data() + offset, count * stride);
				info.data.insert(info.data.end(), data.begin(), data.end());
			}
			else
			{
				Vector<unsigned char> data(count * stride);
				memcpy(data.data(), buffer.data.data() + offset, count * stride);

				if (stride == sizeof(uint16_t))
				{
					Vector<uint32_t> converted(data.size() / sizeof(uint16_t));

					for (size_t i = 0; i < data.size(); i += sizeof(uint16_t))
					{
						converted.at(i / sizeof(uint16_t)) = (data.at(i + 1) << 8) + data.at(i);
					}

					data.resize(converted.size() * sizeof(uint32_t));
					memcpy((void*)data.data(), converted.data(), data.size());

					info.data.insert(info.data.end(), data.begin(), data.end());
				}
				else if (stride == sizeof(uint8_t))
				{
					Vector<uint32_t> converted(data.size());

					for (size_t i = 0; i < data.size(); ++i)
					{
						converted.at(i) = (uint32_t)data.at(i);
					}

					data.resize(converted.size() * sizeof(uint32_t));
					memcpy((void*)data.data(), converted.data(), data.size());

					info.data.insert(info.data.end(), data.begin(), data.end());
				}
			}
		}
	}

	void removeAllTextures(const String& base_path, const String& file_name)
	{
		int texture_index = 0;
		bool found;
		do
		{
			String uri = base_path + "__" + file_name + "_" + toString(texture_index) + "__.png";
			found = FileSystem::DoesFileExist(uri);
			if (found)
				FileSystem::RemoveFile(uri);
		} while (found);
	}

	void getTexture(const String& base_path, const String& file_name, const tinygltf::Model& model, const Vector<int>& textures, Vector<String>& handles)
	{
		for (const int& t : textures)
		{
			const tinygltf::Texture& texture = model.textures.at(t);
			const tinygltf::Image&   image = model.images.at(texture.source);

			size_t width = image.width;
			size_t height = image.height;
			size_t bpp = image.component;

			String uri = lmbString(image.uri);

			if (uri.empty())
			{
				uri = base_path + "__" + file_name + "_" + toString(t) + "__.png";
				stbi_write_png(uri.c_str(), (int)width, (int)height, (int)bpp, image.image.data(), (int)width * (int)bpp);
			}
			else
				uri = base_path + uri;

			uri = FileSystem::MakeRelative(uri);
			handles.push_back(uri);
		}
	}

	void addNode(const tinygltf::Model& model, const int& n, const int& current_mesh, Vector<VioletSubMesh>& meshes, const AccessorMemoryConverter& converter)
	{
		const tinygltf::Node& node = model.nodes.at(n);
		VioletSubMesh& mesh = meshes.at(current_mesh);

		// Get the matrix.
		if (node.matrix.size())
		{
			//glm::mat4 m = glm::make_mat4(node.matrix.data());
			lambda::utilities::decomposeMatrix(
				glm::make_mat4(node.matrix.data()),
				&mesh.scale,
				&mesh.rotation,
				&mesh.translation
			);
			glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), mesh.translation);
			model_mat = model_mat * glm::mat4_cast(mesh.rotation);
			model_mat = glm::scale(model_mat, mesh.scale);
			model_mat = model_mat;
		}
		else
		{
			mesh.translation = (node.translation.size() > 0 ? glm::vec3((float)node.translation.at(0), (float)node.translation.at(1), (float)node.translation.at(2)) : glm::vec3(0.0f));
			mesh.scale       = (node.scale.size()       > 0 ? glm::vec3((float)node.scale.at(0), (float)node.scale.at(1), (float)node.scale.at(2)) : glm::vec3(1.0f));
			mesh.rotation    = (node.rotation.size()    > 0 ? glm::quat((float)node.rotation.at(3), (float)node.rotation.at(0), (float)node.rotation.at(1), (float)node.rotation.at(2)) : glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
		}

		if (node.mesh >= 0)
		{
			const tinygltf::Mesh& gltf_mesh = model.meshes.at(node.mesh);

			// For each primitive, find each attribute and insert it.
			for (const tinygltf::Primitive& primitive : gltf_mesh.primitives)
			{
				meshes.push_back(VioletSubMesh());
				VioletSubMesh& sub_mesh = meshes.back();
				sub_mesh.parent = current_mesh;

				sub_mesh.pos = converter.pos.at(contains(primitive.attributes, "POSITION"));
				sub_mesh.nor = converter.nor.at(contains(primitive.attributes, "NORMAL"));
				sub_mesh.tan = converter.tan.at(contains(primitive.attributes, "TANGENT"));
				sub_mesh.col = converter.col.at(contains(primitive.attributes, "COLOR"));
				sub_mesh.tex = converter.tex.at(contains(primitive.attributes, "TEXCOORD_0"));
				sub_mesh.joi = converter.joi.at(contains(primitive.attributes, "JOINTS_0"));
				sub_mesh.wei = converter.wei.at(contains(primitive.attributes, "WEIGHTS_0"));
				sub_mesh.idx = converter.idx.at(primitive.indices);
				sub_mesh.topology = primitive.mode;

				const tinygltf::Accessor& position_accessor = model.accessors.at(primitive.attributes.at("POSITION"));
				sub_mesh.aabb_min = glm::vec3(
					(float)position_accessor.minValues.at(0u),
					(float)position_accessor.minValues.at(1u),
					(float)position_accessor.minValues.at(2u)
				);
				sub_mesh.aabb_max = glm::vec3(
					(float)position_accessor.maxValues.at(0u),
					(float)position_accessor.maxValues.at(1u),
					(float)position_accessor.maxValues.at(2u)
				);

				if (primitive.material < (const int)model.materials.size())
				{
					tinygltf::Parameter param;
					const tinygltf::Material& material = model.materials.at(primitive.material);
					if (true == containsRaw(material.values, "baseColorTexture", param)) {
						sub_mesh.tex_alb = converter.tex_alb.at((int)param.json_double_value.at("index"));
					}
					if (true == containsRaw(material.additionalValues, "normalTexture", param)) {
						sub_mesh.tex_nor = converter.tex_nor.at((int)param.json_double_value.at("index"));
					}
					if (true == containsRaw(material.values, "metallicRoughnessTexture", param)) {
						sub_mesh.tex_dmra = converter.tex_dmra.at((int)param.json_double_value.at("index"));
					}
					if (true == containsRaw(material.values, "emissiveTexture", param)) {
						sub_mesh.tex_emi = converter.tex_emi.at((int)param.json_double_value.at("index"));
					}
					if (true == containsRaw(material.values, "doubleSided", param)) {
						sub_mesh.double_sided = param.bool_value;
					}
					if (true == containsRaw(material.values, "metallicFactor", param)) {
						sub_mesh.metallic = param.number_array.empty() ? 0.0f : (float)param.number_array.at(0);
					}
					if (true == containsRaw(material.values, "roughnessFactor", param)) {
						sub_mesh.roughness = param.number_array.empty() ? 0.0f : (float)param.number_array.at(0);
					}
					if (true == containsRaw(material.values, "emissiveFactor", param)) {
						sub_mesh.emissiveness = param.number_array.empty() ? glm::vec3() : glm::vec3(
							param.number_array.at(0), param.number_array.at(1), param.number_array.at(2)
						);
					}
					if (true == containsRaw(material.values, "baseColorFactor", param)) {
						sub_mesh.colour = param.number_array.empty() ? glm::vec4(0.0f) : glm::vec4(
							(float)param.number_array.at(0), (float)param.number_array.at(1),
							(float)param.number_array.at(2), (float)param.number_array.at(3)
						);
					}
				}
			}
		}

		// Add all children.
		for (const int& child : node.children)
		{
			meshes.push_back(VioletSubMesh());
			meshes.back().parent = current_mesh;
			addNode(model, child, (int)meshes.size() - 1, meshes, converter);
		}
	}

	void addNode(const tinygltf::Model& model, AccessorMemory& memory, const int& n)
	{
		// Get the current node.
		const tinygltf::Node& node = model.nodes.at(n);

		// Check if this node has a mesh.
		if (node.mesh >= 0)
		{
			const tinygltf::Mesh& mesh = model.meshes.at(node.mesh);

			// For each primitive, find each attribute and insert it.
			for (const tinygltf::Primitive& primitive : mesh.primitives)
			{
				insert(memory.pos, contains(primitive.attributes, "POSITION"));
				insert(memory.nor, contains(primitive.attributes, "NORMAL"));
				insert(memory.tan, contains(primitive.attributes, "TANGENT"));
				insert(memory.col, contains(primitive.attributes, "COLOR"));
				insert(memory.tex, contains(primitive.attributes, "TEXCOORD_0"));
				insert(memory.joi, contains(primitive.attributes, "JOINTS_0"));
				insert(memory.wei, contains(primitive.attributes, "WEIGHTS_0"));
				insert(memory.idx, primitive.indices);

				if (primitive.material < (const int)model.materials.size())
				{
					tinygltf::Parameter param;
					if (true == containsRaw(model.materials[primitive.material].values, "baseColorTexture", param)) {
						insert(memory.tex_alb, (int)param.json_double_value.at("index"));
					}
					if (true == containsRaw(model.materials[primitive.material].additionalValues, "normalTexture", param)) {
						insert(memory.tex_nor, (int)param.json_double_value.at("index"));
					}
					if (true == containsRaw(model.materials[primitive.material].values, "metallicRoughnessTexture", param)) {
						insert(memory.tex_dmra, (int)param.json_double_value.at("index"));
					}
					if (true == containsRaw(model.materials[primitive.material].values, "emissiveTexture", param)) {
						insert(memory.tex_emi, (int)param.json_double_value.at("index"));
					}
				}
			}
		}

		// Add all children.
		for (const int& child : node.children)
		{
			addNode(model, memory, child);
		}
	}

	tinygltf::Model loadWorld(const String& path)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string error_message;
		std::string warning_message;
		String extension = FileSystem::GetExtension(path);
		bool return_value = false;

		if (extension.compare("glb") == 0)
		{
			// assume binary glTF.
			return_value = loader.LoadBinaryFromFile(&model, &error_message, &warning_message, path.c_str());
		}
		else
		{
			// assume ascii glTF.
			return_value = loader.LoadASCIIFromFile(&model, &error_message, &warning_message, path.c_str());
		}

		if (false == warning_message.empty())
		{
			foundation::Error("MeshIO: Warning " + lmbString(warning_message) + "\n");
		}
		if (false == error_message.empty())
		{
			foundation::Error("MeshIO: Error " + lmbString(error_message) + "\n");
		}
		if (false == return_value)
		{
			foundation::Error("MeshIO: Failed to load ." + extension + " : " + String(path) + "\n");
			exit(-1);
		}

		return model;
	}










	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VioletMeshCompiler::VioletMeshCompiler() :
		VioletMeshManager()
	{
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool VioletMeshCompiler::Compile(MeshCompileInfo mesh_info)
	{
		VioletMesh mesh = loadMeshGLTF(mesh_info.file);
		mesh.hash = GetHash(mesh_info.file);
		mesh.file = mesh_info.file;
		AddMesh(mesh);

		return true;
	}
}