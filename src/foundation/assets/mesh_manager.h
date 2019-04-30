#pragma once
#include "base_asset_manager.h"
#include <containers/containers.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace lambda
{
	struct VioletDataSegment
	{
		size_t offset;
		size_t count;
		size_t stride;
	};
	struct VioletDataInfo
	{
		Vector<unsigned char> data;
		Vector<VioletDataSegment> segments;
	};
	struct VioletSubMesh
	{
		// Required information.
		int parent = 0;
		int topology = 0;
		glm::vec3 translation = glm::vec3(0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f);
		// Bounding volume.
		glm::vec3 aabb_min = glm::vec3(0.0f);
		glm::vec3 aabb_max = glm::vec3(0.0f);
		// Buffers.
		int pos = -1;
		int nor = -1;
		int tan = -1;
		int col = -1;
		int tex = -1;
		int idx = -1;
		int joi = -1;
		int wei = -1;
		// Textures.
		int tex_alb = -1;
		int tex_nor = -1;
		int tex_dmra = -1;
		int tex_emi = -1;
		// Additional information.
		bool double_sided = false;
		float metallic = 0.0f;
		float roughness = 1.0f;
		glm::vec3 emissiveness = glm::vec3(0.0f);
		glm::vec4 colour = glm::vec4(1.0f);
	};
	struct VioletMeshData
	{
		VioletDataInfo pos;
		VioletDataInfo nor;
		VioletDataInfo tan;
		VioletDataInfo col;
		VioletDataInfo tex;
		VioletDataInfo joi;
		VioletDataInfo wei;
		VioletDataInfo idx;
		Vector<String> tex_alb;
		Vector<String> tex_nrm;
		Vector<String> tex_dmra;
		Vector<String> tex_emi;
	};
	struct VioletMesh
	{
		uint64_t hash = 0u;
		String   file = "";
		VioletMeshData data;
		Vector<VioletSubMesh> meshes;
	};

	class VioletMeshManager : public VioletBaseAssetManager
	{
	public:
		VioletMeshManager();

		uint64_t GetHash(String mesh_name);

		void AddMesh(VioletMesh mesh);
		VioletMesh GetMesh(uint64_t hash, bool get_data = false);
		void RemoveMesh(uint64_t hash);

	private:
		VioletMesh JSonToMeshHeader(Vector<char> json);
		Vector<char> MeshHeaderToJSon(VioletMesh mesh);
	};
}