#include "mesh_decimator.h"
#include <mdMeshDecimator.h>
#include <utils/console.h>

namespace lambda
{
  namespace platform
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class LambdaMeshDecimator : public MeshDecimation::MeshDecimator
    {
    public:
      inline Vector<bool> GetVertexTags() const
      {
        Vector<bool> tags(m_nPoints);

        for (size_t v = 0; v < m_nPoints; ++v)
        {
          tags[v] = m_vertices[v].m_tag;
        }

        return eastl::move(tags);
      }
      inline Vector<bool> GetTriangleTags() const
      {
        Vector<bool> tags(m_nInitialTriangles);

        for (size_t v = 0; v < m_nInitialTriangles; ++v)
        {
          tags[v] = m_trianglesTags[v];
        }

        return eastl::move(tags);
      }
    protected:
      size_t                                m_nPoints;
      std::vector<MeshDecimation::MDVertex> m_vertices;
      size_t                                m_nInitialTriangles;
      bool*                                 m_trianglesTags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename TI, typename TR>
    void convert(const Vector<TI>& input, Vector<TR>& output)
    {
      TR* it_output = output.data();
      TI* it_input  = (TI*)input.data();

      for (size_t i = 0u; i < input.size(); ++i)
      {
        *it_output = (TR)(*it_input);
        ++it_output;
        ++it_input;
      }
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename TI, typename TR>
    Vector<TR> convert(const Vector<TI>& input)
    {
      Vector<TR> output(input.size());
      convert(input, output);
      return eastl::move(output);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MeshDecimator::decimate(asset::Mesh* input, asset::Mesh* output, float reduction, float target_error)
    {
      // Output data.
      Vector<glm::vec3> new_pos;
      Vector<glm::vec3> new_nor;
      Vector<glm::vec2> new_tex;
      Vector<glm::vec4> new_col;
      Vector<glm::vec3> new_tan;
      Vector<glm::vec4> new_joi;
      Vector<glm::vec4> new_wei;
      Vector<uint16_t>  new_idx16;
      Vector<uint32_t>  new_idx32;
      Vector<asset::SubMesh> sub_meshes = input->getSubMeshes();

      for(size_t sid = 0u; sid < sub_meshes.size(); ++sid)
      {
        asset::SubMesh& sub_mesh = sub_meshes.at(sid);
        // Get the correct counts and vertices.
        size_t vertex_count = sub_mesh.offset.at(asset::MeshElements::kPositions).count;
        size_t index_count  = sub_mesh.offset.at(asset::MeshElements::kIndices).count / 3u;
        if(vertex_count != 0u&& index_count != 0u)
        {
          Vector<glm::vec3> vertices = input->get<glm::vec3>(asset::MeshElements::kPositions, sid);
          char* i_data = (char*)input->get(asset::MeshElements::kIndices).data + sub_mesh.offset.at(asset::MeshElements::kIndices).offset;
        
          // Convert the indices.
          Vector<int> indices;
          if (sizeof(uint16_t) == input->get(asset::MeshElements::kIndices).size)
          {
            Vector<uint16_t> ti(index_count * 3u);
            memcpy(ti.data(), i_data, index_count * 3u * sizeof(uint16_t));
            indices = convert<uint16_t, int>(ti);
          }
          else
          {
            Vector<uint32_t> ti(index_count * 3u);
            memcpy(ti.data(), i_data, index_count * 3u * sizeof(uint32_t));
            indices = convert<uint32_t, int>(ti);
          }

          // Init decimator.
          Vector<bool> vertex_tags;
          Vector<int> new_indices;
          size_t new_index_count;
          size_t new_vertex_count;
          if (vertex_count > 50u&& index_count > 50u)
          {
            LambdaMeshDecimator decimator;
            decimator.Initialize(vertex_count, index_count, (MeshDecimation::Vec3<MeshDecimation::Float>*)vertices.data(), (MeshDecimation::Vec3<int>*)indices.data());

            // Decimate.
            size_t target_vertex_count = vertex_count > 50u ? (size_t)((float)vertex_count * reduction) : vertex_count;
            size_t target_index_count  = index_count  > 50u ? (size_t)((float)index_count  * reduction) : index_count;
            decimator.Decimate(target_vertex_count, target_index_count, (double)target_error);

            // Get the new indices and vertices.
            new_index_count  = decimator.GetNTriangles() * 3u;
            new_vertex_count = decimator.GetNVertices();
            new_indices.resize(new_index_count);
            Vector<glm::vec3> new_vertices(new_vertex_count);
            decimator.GetMeshData((MeshDecimation::Vec3<float>*)new_vertices.data(), (MeshDecimation::Vec3<int>*)new_indices.data());
            vertex_tags = decimator.GetVertexTags();
          }
          else
          {
            new_index_count  = index_count;
            new_vertex_count = vertex_count;
            new_indices  = indices;
            vertex_tags.resize(vertex_count, true);
          }
        
          // Get all required things. So many different things.
          // Input data.
          Vector<glm::vec3> old_pos = input->get<glm::vec3>(asset::MeshElements::kPositions, sid);
          Vector<glm::vec3> old_nor = input->get<glm::vec3>(asset::MeshElements::kNormals,   sid);
          Vector<glm::vec2> old_tex = input->get<glm::vec2>(asset::MeshElements::kTexCoords, sid);
          Vector<glm::vec4> old_col = input->get<glm::vec4>(asset::MeshElements::kColours,   sid);
          Vector<glm::vec3> old_tan = input->get<glm::vec3>(asset::MeshElements::kTangents,  sid);
          Vector<glm::vec4> old_joi = input->get<glm::vec4>(asset::MeshElements::kJoints,    sid);
          Vector<glm::vec4> old_wei = input->get<glm::vec4>(asset::MeshElements::kWeights,   sid);

          size_t size_pos = new_pos.size();
          size_t size_nor = new_nor.size();
          size_t size_tex = new_tex.size();
          size_t size_col = new_col.size();
          size_t size_tan = new_tan.size();
          size_t size_joi = new_joi.size();
          size_t size_wei = new_wei.size();
          new_pos.resize(new_pos.size() + std::min(old_pos.size(), new_vertex_count));
          new_nor.resize(new_nor.size() + std::min(old_nor.size(), new_vertex_count));
          new_tex.resize(new_tex.size() + std::min(old_tex.size(), new_vertex_count));
          new_col.resize(new_col.size() + std::min(old_col.size(), new_vertex_count));
          new_tan.resize(new_tan.size() + std::min(old_tan.size(), new_vertex_count));
          new_joi.resize(new_joi.size() + std::min(old_joi.size(), new_vertex_count));
          new_wei.resize(new_wei.size() + std::min(old_wei.size(), new_vertex_count));

          // Iterators.
          glm::vec3* pos_it = old_pos.size() > 0 ?&new_pos.at(size_pos) : nullptr;
          glm::vec3* nor_it = old_nor.size() > 0 ?&new_nor.at(size_nor) : nullptr;
          glm::vec2* tex_it = old_tex.size() > 0 ?&new_tex.at(size_tex) : nullptr;
          glm::vec4* col_it = old_col.size() > 0 ?&new_col.at(size_col) : nullptr;
          glm::vec3* tan_it = old_tan.size() > 0 ?&new_tan.at(size_tan) : nullptr;
          glm::vec4* joi_it = old_joi.size() > 0 ?&new_joi.at(size_joi) : nullptr;
          glm::vec4* wei_it = old_wei.size() > 0 ?&new_wei.at(size_wei) : nullptr;

          // Set vertices.
          for (size_t i = 0u; i < vertex_tags.size(); ++i)
          {
            if (vertex_tags.at(i) == true)
            {
              // Copy correct data.
              if (!old_pos.empty()) { *pos_it = old_pos.at(i); ++pos_it; }
              if (!old_nor.empty()) { *nor_it = old_nor.at(i); ++nor_it; }
              if (!old_tex.empty()) { *tex_it = old_tex.at(i); ++tex_it; }
              if (!old_col.empty()) { *col_it = old_col.at(i); ++col_it; }
              if (!old_tan.empty()) { *tan_it = old_tan.at(i); ++tan_it; }
              if (!old_joi.empty()) { *joi_it = old_joi.at(i); ++joi_it; }
              if (!old_wei.empty()) { *wei_it = old_wei.at(i); ++wei_it; }
            }
          }

          // Set output.
          sub_mesh.offset.at(asset::MeshElements::kIndices).count = new_indices.size();

          if (sizeof(uint16_t) == input->get(asset::MeshElements::kIndices).size)
          {
            sub_mesh.offset.at(asset::MeshElements::kIndices).offset = new_idx16.size() * sizeof(uint16_t);
            Vector<uint16_t> i = convert<int, uint16_t>(new_indices);
            new_idx16.insert(new_idx16.end(), i.begin(), i.end());
          }
          else
          {
            sub_mesh.offset.at(asset::MeshElements::kIndices).offset = new_idx32.size() * sizeof(uint32_t);
            Vector<uint32_t> i = convert<int, uint32_t>(new_indices);
            new_idx32.insert(new_idx32.end(), i.begin(), i.end());
          }

          // Sub meshes.
          sub_mesh.offset.at(asset::MeshElements::kPositions).offset = size_pos * sub_mesh.offset.at(asset::MeshElements::kPositions).stride;
          sub_mesh.offset.at(asset::MeshElements::kNormals).offset   = size_nor * sub_mesh.offset.at(asset::MeshElements::kNormals).stride;
          sub_mesh.offset.at(asset::MeshElements::kTexCoords).offset = size_tex * sub_mesh.offset.at(asset::MeshElements::kTexCoords).stride;
          sub_mesh.offset.at(asset::MeshElements::kColours).offset   = size_col * sub_mesh.offset.at(asset::MeshElements::kColours).stride;
          sub_mesh.offset.at(asset::MeshElements::kTangents).offset  = size_tan * sub_mesh.offset.at(asset::MeshElements::kTangents).stride;
          sub_mesh.offset.at(asset::MeshElements::kJoints).offset    = size_joi * sub_mesh.offset.at(asset::MeshElements::kJoints).stride;
          sub_mesh.offset.at(asset::MeshElements::kWeights).offset   = size_wei * sub_mesh.offset.at(asset::MeshElements::kWeights).stride;

          sub_mesh.offset.at(asset::MeshElements::kPositions).count = new_pos.size() - size_pos;
          sub_mesh.offset.at(asset::MeshElements::kNormals).count   = new_nor.size() - size_nor;
          sub_mesh.offset.at(asset::MeshElements::kTexCoords).count = new_tex.size() - size_tex;
          sub_mesh.offset.at(asset::MeshElements::kColours).count   = new_col.size() - size_col;
          sub_mesh.offset.at(asset::MeshElements::kTangents).count  = new_tan.size() - size_tan;
          sub_mesh.offset.at(asset::MeshElements::kJoints).count    = new_joi.size() - size_joi;
          sub_mesh.offset.at(asset::MeshElements::kWeights).count   = new_wei.size() - size_wei;
        }
      }
      
      output->set(asset::MeshElements::kPositions, eastl::move(new_pos));
      output->set(asset::MeshElements::kNormals,   eastl::move(new_nor));
      output->set(asset::MeshElements::kTexCoords, eastl::move(new_tex));
      output->set(asset::MeshElements::kColours,   eastl::move(new_col));
      output->set(asset::MeshElements::kTangents,  eastl::move(new_tan));
      output->set(asset::MeshElements::kJoints,    eastl::move(new_joi));
      output->set(asset::MeshElements::kWeights,   eastl::move(new_wei));
      if (!new_idx16.empty()) output->set(asset::MeshElements::kIndices, eastl::move(new_idx16));
      if (!new_idx32.empty()) output->set(asset::MeshElements::kIndices, eastl::move(new_idx32));

      output->setSubMeshes(sub_meshes);
      output->setAttachedTextureCount(input->getAttachedTextureCount());
      output->setTopology(input->getTopology());
      output->setAttachedTextures(input->getAttachedTextures());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MeshDecimator::decimateOld(asset::Mesh* input, asset::Mesh* output, float reduction, float target_error)
    {
      // Get the correct counts and vertices.
      size_t vertex_count = input->get(asset::MeshElements::kPositions).count;
      size_t index_count  = input->get(asset::MeshElements::kIndices).count / 3u;
      MeshDecimation::Vec3<MeshDecimation::Float>* vertices = (MeshDecimation::Vec3<MeshDecimation::Float>*)input->get(asset::MeshElements::kPositions).data;
      
      // Convert the indices.
      Vector<int> indices;
      if (sizeof(uint16_t) == input->get(asset::MeshElements::kIndices).size)
      {
        indices = convert<uint16_t, int>(input->get<uint16_t>(asset::MeshElements::kIndices));
      }
      else
      {
        indices = convert<uint32_t, int>(input->get<uint32_t>(asset::MeshElements::kIndices));
      }

      // Init decimator.
      LambdaMeshDecimator decimator;
      decimator.Initialize(vertex_count, index_count, vertices, (MeshDecimation::Vec3<int>*)indices.data());

      // Decimate.
      size_t target_vertex_count = (size_t)((float)vertex_count * reduction);
      size_t target_index_count  = (size_t)((float)index_count  * reduction);
      decimator.Decimate(target_vertex_count, target_index_count, (double)target_error);

      // Get the new indices and vertices.
      size_t new_index_count = decimator.GetNTriangles() * 3u;
      size_t new_vertex_count = decimator.GetNVertices();
      Vector<int> new_indices(new_index_count);
      Vector<glm::vec3> new_vertices(new_vertex_count);
      decimator.GetMeshData((MeshDecimation::Vec3<float>*)new_vertices.data(), (MeshDecimation::Vec3<int>*)new_indices.data());
      Vector<bool> vertex_tags = decimator.GetVertexTags();
      Vector<bool> index_tags  = decimator.GetTriangleTags();

      // Get all required things. So many different things.
      // Input data.
      Vector<glm::vec3> old_pos = input->get<glm::vec3>(asset::MeshElements::kPositions);
      Vector<glm::vec3> old_nor = input->get<glm::vec3>(asset::MeshElements::kNormals);
      Vector<glm::vec2> old_tex = input->get<glm::vec2>(asset::MeshElements::kTexCoords);
      Vector<glm::vec4> old_col = input->get<glm::vec4>(asset::MeshElements::kColours);
      Vector<glm::vec4> old_tan = input->get<glm::vec4>(asset::MeshElements::kTangents);
      Vector<glm::vec4> old_joi = input->get<glm::vec4>(asset::MeshElements::kJoints);
      Vector<glm::vec4> old_wei = input->get<glm::vec4>(asset::MeshElements::kWeights);

      // Output data.
      Vector<glm::vec3> new_pos(std::min(old_pos.size(), new_vertex_count));
      Vector<glm::vec3> new_nor(std::min(old_nor.size(), new_vertex_count));
      Vector<glm::vec2> new_tex(std::min(old_tex.size(), new_vertex_count));
      Vector<glm::vec4> new_col(std::min(old_col.size(), new_vertex_count));
      Vector<glm::vec4> new_tan(std::min(old_tan.size(), new_vertex_count));
      Vector<glm::vec4> new_joi(std::min(old_joi.size(), new_vertex_count));
      Vector<glm::vec4> new_wei(std::min(old_wei.size(), new_vertex_count));

      // Iterators.
      glm::vec3* pos_it = new_pos.data();
      glm::vec3* nor_it = new_nor.data();
      glm::vec2* tex_it = new_tex.data();
      glm::vec4* col_it = new_col.data();
      glm::vec4* tan_it = new_tan.data();
      glm::vec4* joi_it = new_joi.data();
      glm::vec4* wei_it = new_wei.data();

      // Set vertices.
      for (size_t i = 0u; i < vertex_tags.size(); ++i)
      {
        if (vertex_tags.at(i) == true)
        {
          // Copy correct data.
          if (pos_it != nullptr) { *pos_it = old_pos.at(i); ++pos_it; }
          if (nor_it != nullptr) { *nor_it = old_nor.at(i); ++nor_it; }
          if (tex_it != nullptr) { *tex_it = old_tex.at(i); ++tex_it; }
          if (col_it != nullptr) { *col_it = old_col.at(i); ++col_it; }
          if (tan_it != nullptr) { *tan_it = old_tan.at(i); ++tan_it; }
          if (joi_it != nullptr) { *joi_it = old_joi.at(i); ++joi_it; }
          if (wei_it != nullptr) { *wei_it = old_wei.at(i); ++wei_it; }
        }
      }

      // Set output.
      if (sizeof(uint16_t) == input->get(asset::MeshElements::kIndices).size)
      {
        output->set(asset::MeshElements::kIndices, convert<int, uint16_t>(new_indices));
      }
      else
      {
        output->set(asset::MeshElements::kIndices, convert<int, uint32_t>(new_indices));
      }

      output->set(asset::MeshElements::kPositions, new_pos);
      output->set(asset::MeshElements::kNormals,   new_nor);
      output->set(asset::MeshElements::kTexCoords, new_tex);
      output->set(asset::MeshElements::kColours,   new_col);
      output->set(asset::MeshElements::kTangents,  new_tan);
      output->set(asset::MeshElements::kJoints,    new_joi);
      output->set(asset::MeshElements::kWeights,   new_wei);

      // Sub meshes.
      // TODO (Hilze): Optimize this.

      Vector<asset::SubMesh> sub_meshes = input->getSubMeshes();
      for (asset::SubMesh& sub_mesh : sub_meshes)
      {
        for (size_t j = 0u; j < asset::MeshElements::kCount; ++j)
        {
          if (sub_mesh.offset.at(j).count > 0u)
          {
            size_t new_count = 0u;
            size_t new_offset = 0u;
            size_t ods = sub_mesh.offset.at(j).offset / sub_mesh.offset.at(j).stride;

            if (j == asset::MeshElements::kIndices)
            {
              for (size_t i = 0u; i < ods; ++i)
              {
                size_t idx = i / 3u;
                LMB_ASSERT(idx < index_tags.size(), "Decimator: Tried to access index tag out of range!");
                if (index_tags.at(idx))
                {
                  new_offset++;
                }
              }
              for (size_t i = 0u; i < sub_mesh.offset.at(j).count; ++i)
              {
                size_t idx = (ods + i) / 3u;
                LMB_ASSERT(idx < index_tags.size(), "Decimator: Tried to access index tag out of range!");
                if (index_tags.at(idx))
                {
                  new_count++;
                }
              }
            }
            else
            {
              for (size_t i = 0u; i < ods; ++i)
              {
                LMB_ASSERT(i < vertex_tags.size(), "Decimator: Tried to access vertex tag out of range!");
                if (vertex_tags.at(i))
                {
                  new_offset++;
                }
              }
              for (size_t i = 0u; i < sub_mesh.offset.at(j).count; ++i)
              {
                LMB_ASSERT(i < vertex_tags.size(), "Decimator: Tried to access vertex tag out of range!");
                if (vertex_tags.at(ods + i))
                {
                  new_count++;
                }
              }
            }

            sub_mesh.offset.at(j).count  = new_count;
            sub_mesh.offset.at(j).offset = new_offset * sub_mesh.offset.at(j).stride;
          }
        }
      }

      output->setSubMeshes(sub_meshes);
      output->setAttachedTextureCount(input->getAttachedTextureCount());
      output->setTopology(input->getTopology());
      output->setAttachedTextures(input->getAttachedTextures());
    }
  }
}