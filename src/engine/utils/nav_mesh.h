#pragma once
#include <containers/containers.h>
#include <glm/glm.hpp>
#include <utils/bvh.h>

namespace lambda
{
	namespace platform
	{
		struct NavNode
		{
			glm::vec3 position;
			size_t index;
			Vector<NavNode*> connections;

			void addConnection(NavNode* connection);
			void removeConnection(NavNode* connection);
		};

		class NavMap
		{
		public:
			NavNode* addNode(glm::vec3 position);
			void removeNode(glm::vec3 position);
			Vector<glm::vec3> findPath(glm::vec3 from, glm::vec3 to);

		private:
			void updateIndices(size_t start_index);
			NavNode* findClosest(glm::vec3 position);
			float heuristicCostEstimate(NavNode* a, NavNode* b);

		private:
			Vector<NavNode*> nodes_;
		};

		struct NavMapShape
		{
			Vector<glm::vec2> p;
			glm::vec2 c;
			Vector<Vector<NavMapShape*>> share_polys;
		};

		struct NavMapPoint
		{
			bool operator==(const NavMapPoint& other) const;
			glm::vec2 point;
			NavMapShape* shape;
		};

		class TriNavMap
		{
		public:
			void addTri(glm::vec2 a, glm::vec2 b, glm::vec2 c);
			void addTriHole(glm::vec2 a, glm::vec2 b, glm::vec2 c);
			void addQuad(glm::vec2 bl, glm::vec2 tr);
			void addShape(Vector<glm::vec2> points);
			void addQuadHole(glm::vec2 bl, glm::vec2 tr);
			Vector<glm::vec2> getTris();
			Vector<glm::vec3> findPath(glm::vec3 from, glm::vec3 to);
			Vector<glm::vec3> findPath(glm::vec2 from, glm::vec2 to);

		private:
			utilities::BVH bvh_;
			Vector<NavMapShape*> shapes_;
			NavMapPoint* findClosest(glm::vec2 position);
			float heuristicCostEstimate(NavMapPoint* a, NavMapPoint* b);
		};

		extern inline bool equal(const glm::vec2& a, const glm::vec2& b);
	}
}

namespace eastl
{
	/////////////////////////////////////////////////////////////////////////////
	template <>
	struct hash<lambda::platform::NavMapPoint>
	{
		std::size_t operator()(const lambda::platform::NavMapPoint& k) const
		{
			size_t hash = 0ull;
			lambda::hashCombine(hash, k.shape);
			for (uint32_t i = 0; i < k.shape->p.size(); ++i)
				if (lambda::platform::equal(k.point, k.shape->p[i]))
					lambda::hashCombine(hash, (i + 1) * 1000);
			return hash;
		}
	};
}