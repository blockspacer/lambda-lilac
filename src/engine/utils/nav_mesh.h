#pragma once
#include <containers/containers.h>
#include <glm/glm.hpp>
#include <utils/bvh.h>

namespace lambda
{
	namespace platform
	{
		template <typename T>
		struct Promise
		{
			static std::mutex g_mutex;
			T t;
			bool is_finished = false;
			bool finished() {
				g_mutex.lock();
				bool finished = is_finished;
				g_mutex.unlock();
				return finished;
			}
			bool get(T& t)
			{
				g_mutex.lock();
				if (is_finished)
					t = this->t;
				bool finished = is_finished;
				g_mutex.unlock();
				return finished;
			}
		};

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
			Vector<glm::vec3> p;
			glm::vec3 c;
			Vector<Vector<NavMapShape*>> share_polys;
		};

		struct NavMapPoint
		{
			bool operator==(const NavMapPoint& other) const;
			glm::vec3 point;
			NavMapShape* shape;
		};

		class TriNavMap
		{
		public:
			void addTri(glm::vec3 a, glm::vec3 b, glm::vec3 c);
			void addTriHole(glm::vec3 a, glm::vec3 b, glm::vec3 c);
			void addQuad(glm::vec3 bl, glm::vec3 tr);
			void addShape(Vector<glm::vec3> points);
			void addQuadHole(glm::vec3 bl, glm::vec3 tr);
			Vector<glm::vec3> getTris();
			Vector<glm::vec3> findPath(glm::vec3 from, glm::vec3 to) const;
			static Promise<Vector<glm::vec3>>* findPathPromise(TriNavMap* map, glm::vec3 from, glm::vec3 to);

		private:
			utilities::BVH bvh_;
			Vector<NavMapShape*> shapes_;
			NavMapPoint* findClosest(glm::vec3 position) const;
			float heuristicCostEstimate(NavMapPoint* a, NavMapPoint* b) const;
		};

		extern inline bool equal(const glm::vec3& a, const glm::vec3& b);
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