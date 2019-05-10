#pragma once
#include <containers/containers.h>
#include <glm/glm.hpp>

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
	}
}