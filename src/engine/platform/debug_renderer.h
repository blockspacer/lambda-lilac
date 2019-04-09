#pragma once
#include "assets/mesh.h"
#include "shader_pass.h"

namespace lambda
{
	namespace scene
	{
		struct Scene;
	}

	namespace platform
	{
		///////////////////////////////////////////////////////////////////////////
		struct DebugLine
		{
			DebugLine()
				: start(0.0f)
				, end(0.0f)
				, colour_start(0.0f)
				, colour_end(0.0f)
			{}
			DebugLine(
				const glm::vec3& start,
				const glm::vec3& end,
				const glm::vec4 colour)
				: start(start)
				, end(end)
				, colour_start(colour)
				, colour_end(colour) {}
			DebugLine(
				const glm::vec3& start,
				const glm::vec3& end,
				const glm::vec4 colour_start,
				const glm::vec4 colour_end)
				: start(start)
				, end(end)
				, colour_start(colour_start)
				, colour_end(colour_end) {}

			glm::vec3 start;
			glm::vec3 end;
			glm::vec4 colour_start;
			glm::vec4 colour_end;
		};

		struct DebugTri {
			DebugTri()
				: p1(0.0f)
				, p2(0.0f)
				, p3(0.0f)
				, c1(0.0f)
				, c2(0.0f)
				, c3(0.0f)
			{}
			DebugTri(
				const glm::vec3& p1,
				const glm::vec3& p2,
				const glm::vec3& p3,
				const glm::vec4 colour)
				: p1(p1)
				, p2(p2)
				, p3(p3)
				, c1(colour)
				, c2(colour)
				, c3(colour)
			{}

			DebugTri(
				const glm::vec3& p1,
				const glm::vec3& p2,
				const glm::vec3& p3,
				const glm::vec4 c1,
				const glm::vec4 c2,
				const glm::vec4 c3)
				: p1(p1)
				, p2(p2)
				, p3(p3)
				, c1(c1)
				, c2(c2)
				, c3(c3)
			{}

			glm::vec3 p1;
			glm::vec3 p2;
			glm::vec3 p3;
			glm::vec4 c1;
			glm::vec4 c2;
			glm::vec4 c3;
		};

		///////////////////////////////////////////////////////////////////////////
		class DebugRenderer
		{
		public:
			void operator=(const DebugRenderer& other);
			void DrawLine(const DebugLine& line);
			void DrawTri(const DebugTri& tri);
			void Render(scene::Scene& scene);
			void Initialize(scene::Scene& scene);
			void Deinitialize();
			void Clear();

		private:
			struct Data
			{
				asset::VioletMeshHandle mesh;
				Vector<glm::vec3> positions;
				Vector<glm::vec4> colours;
			};

			Data lines_;
			Data tris_;
			ShaderPass shader_pass_;
		};
	}
}