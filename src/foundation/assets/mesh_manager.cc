#include "mesh_manager.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <utils/console.h>

namespace lambda
{
	static constexpr char kMagicHeader[] = { 'L', 'M', 'B' };
	static constexpr char kInvalidHeader[] = { 'I', 'N', 'V' };

	struct VioletDataHeader
	{
		size_t data_size;
		size_t segment_count;
	};
	struct VioletSegmentHeader
	{
		size_t offset;
		size_t count;
		size_t stride;
	};
	struct VioletModelHeader
	{
		size_t model_count;
	};
	struct VioletTextureHeader
	{
		size_t alb_count;
		size_t nrm_count;
		size_t dmra_count;
		size_t emi_count;
	};

	void write(Vector<char>& data, const char* t, size_t len)
	{
		data.resize(data.size() + len);
		memcpy((void*)(data.data() + (data.size() - len)), t, len);
	}
	template<typename T>
	void write(Vector<char>& data, const T& t)
	{
		write(data, (const char*)&t, sizeof(T));
	}
	void writeHeader(Vector<char>& data, const VioletDataInfo& info)
	{
		VioletDataHeader header;
		header.data_size = info.data.size();
		header.segment_count = info.segments.size();
		write(data, header);

		write(data, (const char*)info.data.data(), info.data.size());

		for (const VioletDataSegment& segment : info.segments)
		{
			VioletSegmentHeader segment_header;
			segment_header.count = segment.count;
			segment_header.offset = segment.offset;
			segment_header.stride = segment.stride;
			write(data, segment_header);
		}
	}
	void writeHeader(Vector<char>& data, const Vector<VioletSubMesh>& meshes)
	{
		VioletModelHeader header;
		header.model_count = meshes.size();
		write(data, header);

		for (const VioletSubMesh& mesh : meshes)
		{
			write(data, (const char*)&mesh, sizeof(VioletSubMesh));
		}
	}
	void writeHeader(Vector<char>& data, const Vector<String>& alb, const Vector<String>& nrm, const Vector<String>& dmra, const Vector<String>& emi)
	{
		VioletTextureHeader header;
		header.alb_count = alb.size();
		header.nrm_count = nrm.size();
		header.dmra_count = dmra.size();
		header.emi_count = emi.size();
		write(data, header);

		for (const String& texture : alb)
		{
			write(data, (size_t)texture.size());
			write(data, texture.c_str(), texture.size());
		}
		for (const String& texture : nrm)
		{
			write(data, (size_t)texture.size());
			write(data, texture.c_str(), texture.size());
		}
		for (const String& texture : dmra)
		{
			write(data, (size_t)texture.size());
			write(data, texture.c_str(), texture.size());
		}
		for (const String& texture : emi)
		{
			write(data, (size_t)texture.size());
			write(data, texture.c_str(), texture.size());
		}
	}
	void startWriting(Vector<char>& data)
	{
		write(data, kInvalidHeader, 3);
	}
	void finalizeWriting(Vector<char>& data)
	{
		memcpy(data.data(), kMagicHeader, 3);
	}

	Vector<char> write(const VioletMesh& mesh)
	{
		Vector<char> data;
		startWriting(data);

		writeHeader(data, mesh.data.pos);
		writeHeader(data, mesh.data.nor);
		writeHeader(data, mesh.data.tan);
		writeHeader(data, mesh.data.col);
		writeHeader(data, mesh.data.tex);
		writeHeader(data, mesh.data.joi);
		writeHeader(data, mesh.data.wei);
		writeHeader(data, mesh.data.idx);
		writeHeader(data, mesh.data.tex_alb, mesh.data.tex_nrm, mesh.data.tex_dmra, mesh.data.tex_emi);
		writeHeader(data, mesh.meshes);

		finalizeWriting(data);
		return eastl::move(data);
	}







	void read(const Vector<char>& data, size_t& offset, char* t, size_t len)
	{
		memcpy(t, data.data() + offset, len);
		offset += len;
	}

	template<typename T>
	void read(const Vector<char>& data, size_t& offset, T& t)
	{
		read(data, offset, (char*)&t, sizeof(T));
	}
	void readHeader(const Vector<char>& data, size_t& offset, VioletDataInfo& info)
	{
		VioletDataHeader header;
		read(data, offset, header);

		info.data.resize(header.data_size);
		read(data, offset, (char*)info.data.data(), header.data_size);

		for (size_t i = 0; i < header.segment_count; ++i)
		{
			VioletSegmentHeader segment_header;
			read(data, offset, segment_header);

			VioletDataSegment segment;
			segment.count = segment_header.count;
			segment.offset = segment_header.offset;
			segment.stride = segment_header.stride;

			info.segments.push_back(segment);
		}
	}
	void readHeader(const Vector<char>& data, size_t& offset, Vector<VioletSubMesh>& meshes)
	{
		VioletModelHeader header;
		read(data, offset, header);

		for (size_t i = 0; i < header.model_count; ++i)
		{
			VioletSubMesh mesh;
			read(data, offset, mesh);
			meshes.push_back(mesh);
		}
	}
	void readHeader(const Vector<char>& data, size_t& offset, Vector<String>& alb, Vector<String>& nrm, Vector<String>& dmra, Vector<String>& emi)
	{
		VioletTextureHeader header;
		read(data, offset, header);

		for (size_t i = 0; i < header.alb_count; ++i)
		{
			size_t size;
			read(data, offset, size);
			String name(size, '\0');
			read(data, offset, (char*)name.c_str(), name.size());
			alb.push_back(name);
		}

		for (size_t i = 0; i < header.nrm_count; ++i)
		{
			size_t size;
			read(data, offset, size);
			String name(size, '\0');
			read(data, offset, (char*)name.c_str(), name.size());
			nrm.push_back(name);
		}

		for (size_t i = 0; i < header.dmra_count; ++i)
		{
			size_t size;
			read(data, offset, size);
			String name(size, '\0');
			read(data, offset, (char*)name.c_str(), name.size());
			dmra.push_back(name);
		}

		for (size_t i = 0; i < header.emi_count; ++i)
		{
			size_t size;
			read(data, offset, size);
			String name(size, '\0');
			read(data, offset, (char*)name.c_str(), name.size());
			emi.push_back(name);
		}
	}

	bool validateFile(const Vector<char>& data, size_t& offset)
	{
		char magic_header[3];
		read(data, offset, magic_header, 3);

		return magic_header[0] == kMagicHeader[0] &&
			   magic_header[1] == kMagicHeader[1] &&
			   magic_header[2] == kMagicHeader[2];
	}

	void read(VioletMesh& mesh, const Vector<char>& data)
	{
		size_t offset = 0;
		if (!validateFile(data, offset))
		{
			foundation::Error("[MESH] Failed to load mesh\n");
			return;
		}

		readHeader(data, offset, mesh.data.pos);
		readHeader(data, offset, mesh.data.nor);
		readHeader(data, offset, mesh.data.tan);
		readHeader(data, offset, mesh.data.col);
		readHeader(data, offset, mesh.data.tex);
		readHeader(data, offset, mesh.data.joi);
		readHeader(data, offset, mesh.data.wei);
		readHeader(data, offset, mesh.data.idx);
		readHeader(data, offset, mesh.data.tex_alb, mesh.data.tex_nrm, mesh.data.tex_dmra, mesh.data.tex_emi);
		readHeader(data, offset, mesh.meshes);
	}

















  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletMeshManager::VioletMeshManager()
  {
    SetMagicNumber("msh");
    SetGeneratedFilePath("generated/");
    Load();
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t VioletMeshManager::GetHash(String mesh_name)
  {
    return hash(mesh_name);
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletMeshManager::AddMesh(VioletMesh mesh)
  {
    SaveHeader(MeshHeaderToJSon(mesh), mesh.hash);
	Vector<char> data = write(mesh);
	SaveData(data, mesh.hash);
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletMesh VioletMeshManager::GetMesh(uint64_t hash, bool get_data)
  {
    VioletMesh mesh = JSonToMeshHeader(GetHeader(hash));
    if (get_data)
      read(mesh, GetData(hash));
    return mesh;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletMeshManager::RemoveMesh(uint64_t hash)
  {
		RemoveData(hash);
		RemoveHeader(hash);
  }

  extern String rapidjsonErrortoString(rapidjson::ParseErrorCode error);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletMesh VioletMeshManager::JSonToMeshHeader(Vector<char> data)
  {
    rapidjson::Document doc;
    const auto& parse_error = doc.Parse(data.data(), data.size());
    LMB_ASSERT(!parse_error.HasParseError(), rapidjsonErrortoString(parse_error.GetParseError()).c_str());

    VioletMesh mesh;
    mesh.hash = doc["hash"].GetUint64();
    mesh.file = lmbString(doc["file"].GetString());

    return mesh;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Vector<char> VioletMeshManager::MeshHeaderToJSon(VioletMesh Mesh)
  {
    rapidjson::Document doc;
    doc.SetObject();

    doc.AddMember("hash",      Mesh.hash, doc.GetAllocator());
    doc.AddMember("file",      rapidjson::StringRef(Mesh.file.c_str()), doc.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::string string = buffer.GetString();

    Vector<char> data(string.size());
    memcpy(data.data(), string.data(), string.size());
    return data;
  }
}
