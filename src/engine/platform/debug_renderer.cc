#include "debug_renderer.h"
#include "assets/asset_manager.h"
#include "assets/shader_io.h"
#include "post_process_manager.h"
#include "interfaces/irenderer.h"
#include "interfaces/iworld.h"

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    void DebugRenderer::DrawLine(const DebugLine& line)
    {
      positions_.push_back(line.start);
      positions_.push_back(line.end);
      colours_.push_back(line.colour_start);
      colours_.push_back(line.colour_end);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void DebugRenderer::Render(world::IWorld* world)
    {
      if (positions_.empty())
      {
        return;
      }

      static bool initialized = false;
      {
        if (!initialized)
        {
          initialized = true;
          lambda::asset::ShaderHandle shader = 
            asset::AssetManager::getInstance().createAsset(
              Name("debug_shader"),
              lambda::foundation::Memory::constructShared<lambda::asset::Shader>(
                io::ShaderIO::asAsset(io::ShaderIO::load("resources/shaders/debug.fx"))
              )
          );
          mesh_ = asset::AssetManager::getInstance().createAsset(
            Name("debug_mesh"),
            lambda::foundation::Memory::constructShared<lambda::asset::Mesh>()
          );
          mesh_->setTopology(asset::Topology::kLines);
          shader_pass_ = ShaderPass(Name("debug"), shader, {}, {
            world->getPostProcessManager().getTarget(
              world->getPostProcessManager().getFinalTarget()
            )
          });
        }
      }

      asset::SubMesh sub_mesh;
      sub_mesh.offset.at(asset::MeshElements::kPositions) = 
        asset::SubMesh::Offset(0u, positions_.size(), sizeof(glm::vec3));
      sub_mesh.offset.at(asset::MeshElements::kColours)   = 
        asset::SubMesh::Offset(0u, colours_.size(),   sizeof(glm::vec4));

      mesh_->set(asset::MeshElements::kPositions, positions_);
      mesh_->set(asset::MeshElements::kColours,   colours_);
      mesh_->setSubMeshes({ sub_mesh });

      positions_.resize(0u);
      colours_.resize(0u);

      auto renderer = world->getRenderer();
      
      renderer->bindShaderPass(shader_pass_);
      renderer->setSubMesh(0u);
      renderer->setMesh(mesh_);
      renderer->draw();
    }
  }
}
