#include "nav_mesh.h"
#include <memory/memory.h>
#include <glm/gtx/norm.hpp>

namespace lambda
{
	namespace platform
	{
		///////////////////////////////////////////////////////////////////////////
		void NavNode::addConnection(NavNode* connection)
		{
			if (connection == this)
				return;

			if (eastl::find(connections.begin(), connections.end(), connection) == connections.end())
			{
				connections.push_back(connection);
				connection->addConnection(this);
			}
		}

		///////////////////////////////////////////////////////////////////////////
		void NavNode::removeConnection(NavNode* connection)
		{
			if (connection == this)
				return;

			auto it = eastl::find(connections.begin(), connections.end(), connection);
			if (it != connections.end())
			{
				connections.erase(it);
				connection->removeConnection(this);
			}
		}

		///////////////////////////////////////////////////////////////////////////
		NavNode* NavMap::addNode(glm::vec3 position)
		{
			for (size_t i = 0; i < nodes_.size(); ++i)
				if (nodes_[i]->position == position)
					return nodes_[i];
			
			NavNode* node = foundation::Memory::construct<NavNode>();
			node->position = position;
			node->index = nodes_.size();
			nodes_.push_back(node);
			return node;
		}

		///////////////////////////////////////////////////////////////////////////
		void NavMap::removeNode(glm::vec3 position)
		{
			for (size_t i = 0; i < nodes_.size(); ++i)
			{
				if (nodes_[i]->position == position)
				{
					for (NavNode* connection : nodes_[i]->connections)
						connection->removeConnection(nodes_[i]);

					foundation::Memory::destruct(nodes_[i]);
					nodes_.erase(nodes_.begin() + i);
					updateIndices(i);
					return;
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////
		NavNode* NavMap::findClosest(glm::vec3 position)
		{
			NavNode* closest = nullptr;
			float length_closest = FLT_MAX;

			for (NavNode* node : nodes_)
			{
				float length = glm::length2(position - node->position);
				if (length < length_closest)
				{
					closest = node;
					length_closest = length;
				}
			}

			return closest;
		}

		///////////////////////////////////////////////////////////////////////////
		float NavMap::heuristicCostEstimate(NavNode* a, NavNode* b)
		{
			return glm::length2(a->position - b->position);
		}

		///////////////////////////////////////////////////////////////////////////
		Vector<glm::vec3> NavMap::findPath(glm::vec3 from, glm::vec3 to)
		{
			Vector<glm::vec3> path;
			NavNode* node_from = findClosest(from);
			NavNode* node_to = findClosest(to);

			if (node_from == node_to)
				return path;

			Vector<NavNode*> set_closed;
			Vector<NavNode*> set_open = { node_from };
			Vector<NavNode*> came_from(nodes_.size());
			Vector<float> score_g(nodes_.size(), FLT_MAX);
			Vector<float> score_f(nodes_.size(), FLT_MAX);
			score_g[node_from->index] = 0.0f;
			score_f[node_from->index] = heuristicCostEstimate(node_from, node_to);


			while (!set_open.empty())
			{
				NavNode* current = nullptr;
				float lowest_score_f = FLT_MAX;

				for (NavNode* node : set_open)
				{
					const float node_score_f = score_f[node->index];
					if (node_score_f < lowest_score_f)
					{
						lowest_score_f = node_score_f;
						current = node;
					}
				}

				if (current == node_to)
				{
					path = { current->position };

					while (current)
					{
						current = came_from[current->index];
						if (current)
							path.push_back(current->position);
						if (current == node_to)
							current = nullptr;
					}

					std::reverse(path.begin(), path.end());
					return path;
				}

				set_open.erase(eastl::find(set_open.begin(), set_open.end(), current));
				set_closed.push_back(current);

				for (NavNode* neighbor : current->connections)
				{
					if (eastl::find(set_closed.begin(), set_closed.end(), neighbor) == set_closed.end())
					{
						const float tenative_score_g = score_g[current->index] + heuristicCostEstimate(current, neighbor);
						bool valid = true;

						if (eastl::find(set_open.begin(), set_open.end(), neighbor) == set_open.end())
							set_open.push_back(neighbor);
						else if (tenative_score_g >= score_g[neighbor->index])
							valid = false;

						if (valid)
						{
							came_from[neighbor->index] = current;
							score_g[neighbor->index] = tenative_score_g;
							score_f[neighbor->index] = tenative_score_g + heuristicCostEstimate(neighbor, node_to);
						}
					}
				}
			}

			return path;
		}

		///////////////////////////////////////////////////////////////////////////
		void NavMap::updateIndices(size_t start_index)
		{
			for (size_t i = start_index; i < nodes_.size(); ++i)
				nodes_[i]->index = i;
		}
	}
}