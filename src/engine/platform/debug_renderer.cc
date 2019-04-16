#include "debug_renderer.h"
#include "assets/shader_io.h"
#include "post_process_manager.h"
#include "interfaces/irenderer.h"
#include "platform/scene.h"

namespace lambda
{
	namespace platform
	{
		///////////////////////////////////////////////////////////////////////////
		void DebugRenderer::operator=(const DebugRenderer& other)
		{
			lines_       = other.lines_;
			tris_        = other.tris_;
			shader_pass_ = other.shader_pass_;
		}

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
		void DebugRenderer::Render(scene::Scene& scene)
		{
			if (lines_.positions.empty() && tris_.positions.empty())
				return;

			// Lines.
			asset::SubMesh sub_mesh;
			sub_mesh.offsets[asset::MeshElements::kPositions] =
				asset::SubMesh::Offset(0u, lines_.positions.size(), sizeof(glm::vec3));
			sub_mesh.offsets[asset::MeshElements::kColours] =
				asset::SubMesh::Offset(0u, lines_.colours.size(), sizeof(glm::vec4));

			lines_.mesh->set(asset::MeshElements::kPositions, lines_.positions);
			lines_.mesh->set(asset::MeshElements::kColours, lines_.colours);
			lines_.mesh->setSubMeshes({ sub_mesh });

			lines_.positions.resize(0u);
			lines_.colours.resize(0u);

			// Tris.
			sub_mesh.offsets[asset::MeshElements::kPositions] =
				asset::SubMesh::Offset(0u, tris_.positions.size(), sizeof(glm::vec3));
			sub_mesh.offsets[asset::MeshElements::kColours] =
				asset::SubMesh::Offset(0u, tris_.colours.size(), sizeof(glm::vec4));

			tris_.mesh->set(asset::MeshElements::kPositions, tris_.positions);
			tris_.mesh->set(asset::MeshElements::kColours, tris_.colours);
			tris_.mesh->setSubMeshes({ sub_mesh });

			tris_.positions.resize(0u);
			tris_.colours.resize(0u);

			scene.renderer->bindShaderPass(shader_pass_);
			scene.renderer->setSubMesh(0u);

			// Lines.
			scene.renderer->setMesh(lines_.mesh);
			scene.renderer->draw();

			// Tris.
			scene.renderer->setMesh(tris_.mesh);
			scene.renderer->draw();
		}

		void DebugRenderer::Initialize(scene::Scene& scene)
		{
			lambda::asset::VioletShaderHandle shader = asset::ShaderManager::getInstance()->get(Name("resources/shaders/debug.fx"));
			// Lines.
			lines_.mesh = asset::MeshManager::getInstance()->create(Name("debug_mesh_lines"));
			lines_.mesh->setTopology(asset::Topology::kLines);

			// Tris.
			tris_.mesh = asset::MeshManager::getInstance()->create(Name("debug_mesh_tris"));
			tris_.mesh->setTopology(asset::Topology::kTriangles);

			shader_pass_ = ShaderPass(Name("debug"), shader, {}, {
				scene.post_process_manager->getTarget(
					scene.post_process_manager->getFinalTarget()
				)
			});
		}

		void DebugRenderer::Deinitialize()
		{
			tris_.colours.resize(0ull);
			tris_.positions.resize(0ull);
			tris_.mesh = asset::VioletMeshHandle();
			lines_.colours.resize(0ull);
			lines_.positions.resize(0ull);
			lines_.mesh = asset::VioletMeshHandle();
			shader_pass_ = platform::ShaderPass();
		}

		void DebugRenderer::Clear()
		{
			tris_.positions.clear();
			tris_.colours.clear();
			lines_.positions.clear();
			lines_.colours.clear();
		}
	}
}