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
      lines_.positions.push_back(line.start);
      lines_.positions.push_back(line.end);
			lines_.colours.push_back(line.colour_start);
			lines_.colours.push_back(line.colour_end);
    }
    
	///////////////////////////////////////////////////////////////////////////
	void DebugRenderer::DrawTri(const DebugTri& tri)
	{
      tris_.positions.push_back(tri.p1);
			tris_.positions.push_back(tri.p2);
			tris_.positions.push_back(tri.p3);
			tris_.colours.push_back(tri.c1);
			tris_.colours.push_back(tri.c2);
			tris_.colours.push_back(tri.c3);
	}

    ///////////////////////////////////////////////////////////////////////////
    void DebugRenderer::Render(world::IWorld* world)
    {
      if (lines_.positions.empty() && tris_.positions.empty())
        return;

      static bool initialized = false;
      {
        if (!initialized)
        {
          initialized = true;
					lambda::asset::VioletShaderHandle shader =
						asset::ShaderManager::getInstance()->get(Name("resources/shaders/debug.fx"));
          // Lines.
		  lines_.mesh = asset::AssetManager::getInstance().createAsset(
            Name("debug_mesh_lines"),
            lambda::foundation::Memory::constructShared<lambda::asset::Mesh>()
          );
		  lines_.mesh->setTopology(asset::Topology::kLines);

		  // Tris.
		  tris_.mesh = asset::AssetManager::getInstance().createAsset(
			  Name("debug_mesh_tris"),
			  lambda::foundation::Memory::constructShared<lambda::asset::Mesh>()
		  );
		  tris_.mesh->setTopology(asset::Topology::kTriangles);

          shader_pass_ = ShaderPass(Name("debug"), shader, {}, {
            world->getPostProcessManager().getTarget(
              world->getPostProcessManager().getFinalTarget()
            )
          });
        }
      }

			// Lines.
      asset::SubMesh sub_mesh;
      sub_mesh.offsets[asset::MeshElements::kPositions] =
        asset::SubMesh::Offset(0u, lines_.positions.size(), sizeof(glm::vec3));
      sub_mesh.offsets[asset::MeshElements::kColours] =
        asset::SubMesh::Offset(0u, lines_.colours.size(),   sizeof(glm::vec4));

			lines_.mesh->set(asset::MeshElements::kPositions, lines_.positions);
			lines_.mesh->set(asset::MeshElements::kColours,   lines_.colours);
			lines_.mesh->setSubMeshes({ sub_mesh });

			lines_.positions.resize(0u);
			lines_.colours.resize(0u);

			// Tris.
			sub_mesh.offsets[asset::MeshElements::kPositions] =
				asset::SubMesh::Offset(0u, tris_.positions.size(), sizeof(glm::vec3));
			sub_mesh.offsets[asset::MeshElements::kColours] =
				asset::SubMesh::Offset(0u, tris_.colours.size(), sizeof(glm::vec4));

			tris_.mesh->set(asset::MeshElements::kPositions, tris_.positions);
			tris_.mesh->set(asset::MeshElements::kColours,   tris_.colours);
			tris_.mesh->setSubMeshes({ sub_mesh });

			tris_.positions.resize(0u);
			tris_.colours.resize(0u);

			auto renderer = world->getRenderer();
      
			renderer->bindShaderPass(shader_pass_);
			renderer->setSubMesh(0u);

			// Lines.
			renderer->setMesh(lines_.mesh);
			renderer->draw();

			// Tris.
			renderer->setMesh(tris_.mesh);
			renderer->draw();
    }
  }
}
