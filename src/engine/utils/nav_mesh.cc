#include "nav_mesh.h"
#include <memory/memory.h>
#include <glm/gtx/norm.hpp>
#include <utils/console.h>
#include <algorithm>
#include "mt_manager.h"

#if VIOLET_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef near
#undef far
#undef min
#undef max
#endif

namespace lambda
{
	namespace platform
	{
		template<typename T>
		std::mutex Promise<T>::g_mutex;

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

		static inline bool equal(const glm::vec3& a, const glm::vec3& b)
		{
			static constexpr float kEpsilon = 0.001f;
			const glm::bvec3 res = glm::epsilonEqual(a, b, kEpsilon);
			return res.x && res.y && res.z;
		}

		void TriNavMap::addTri(glm::vec3 a, glm::vec3 b, glm::vec3 c)
		{
			addShape({ a, b, c });
		}

		void TriNavMap::addTriHole(glm::vec3 a, glm::vec3 b, glm::vec3 c)
		{
			addTri(c, b, a);
		}

		void TriNavMap::addQuad(glm::vec3 bl, glm::vec3 tr)
		{
			glm::vec3 a, b, c, d;
			a.x = bl.x; a.y = tr.y; a.z = tr.z;
			b.x = tr.x; b.y = tr.y; b.z = tr.z;
			c.x = tr.x; c.y = bl.y; c.z = bl.z;
			d.x = bl.x; d.y = bl.y; d.z = bl.z;

			addShape({ a, b, c, d });
		}

		void TriNavMap::addShape(Vector<glm::vec3> points)
		{
			NavMapShape* shape = foundation::Memory::construct<NavMapShape>();
			shape->p = points;
			shape->share_polys.resize(points.size());

			glm::vec3 bl(FLT_MAX);
			glm::vec3 tr(FLT_MIN);

			for (const glm::vec3& p : points)
			{
				bl.x = std::min(bl.x, p.x);
				bl.y = std::min(bl.y, p.y);
				bl.z = std::min(bl.z, p.z);
				tr.x = std::max(tr.x, p.x);
				tr.y = std::max(tr.y, p.y);
				tr.z = std::max(tr.z, p.z);
			}

			tr.y =  1000.0f;
			bl.y = -1000.0f;

			Vector<void*> user_data = bvh_.getAllUserDataInAABB(utilities::BVHAABB(bl - 1.0f, tr + 1.0f));

			for (void* ud : user_data)
			{
				NavMapShape* s = (NavMapShape*)ud;

				for (uint32_t i = 0; i < shape->p.size(); ++i)
				{
					for (uint32_t j = 0; j < s->p.size(); ++j)
					{
						if (equal(shape->p[i], s->p[j]))
						{
							shape->share_polys[i].push_back(s);
							s->share_polys[j].push_back(shape);
						}
					}
				}
			}

			bvh_.add((entity::Entity)shapes_.size(), shape, utilities::BVHAABB(bl, tr));
			shapes_.push_back(shape);
		}

		void TriNavMap::addQuadHole(glm::vec3 bl, glm::vec3 tr)
		{
			addQuad(tr, bl);
		}

		Vector<glm::vec3> TriNavMap::getTris()
		{
			LMB_ASSERT(!shapes_.empty(), "TRI NAV MAP: Tried to get the tris from an empty navigation map");
			
			Vector<glm::vec3> tris;

			struct Line
			{
				glm::vec3 a;
				glm::vec3 b;
				bool operator==(const Line& other) const
				{
					return equal(a, other.a) && equal(b, other.b);
				}
				bool operator<(const Line& other) const
				{
					return glm::length2(a + b) < glm::length2(other.a + other.b);
				}
			};
			
			Vector<Line> lines;

			for (const auto& shape : shapes_)
				for (uint32_t i = 0; i < shape->p.size(); ++i)
					lines.push_back({ shape->p[i],shape->p[(i + 1) % shape->p.size()] });

			std::sort(lines.begin(), lines.end());
			lines.erase(eastl::unique(lines.begin(), lines.end()), lines.end());

			for (const Line& line : lines)
			{
				tris.push_back(line.a);
				tris.push_back(line.b);
			}

			return tris;
		}

		bool IsInConvexPolygon(const Vector<glm::vec3>& polygon, glm::vec3 testPoint)
		{
			//n>2 Keep track of cross product sign changes
			float pos = 0;
			float neg = 0;

			for (int i = 0; i < polygon.size(); i++)
			{
				//If point is in the polygon
				if (equal(polygon[i], testPoint))
					return true;

				//Form a segment between the i'th point
				float x1 = polygon[i].x;
				float y1 = polygon[i].y;
				float z1 = polygon[i].z;

				//And the i+1'th, or if i is the last, with the first point
				int i2 = i < polygon.size() - 1 ? i + 1 : 0;

				float x2 = polygon[i2].x;
				float y2 = polygon[i2].y;
				float z2 = polygon[i2].z;

				float x = testPoint.x;
				float y = testPoint.y;
				float z = testPoint.z;

				//Compute the cross product
				float d = (x - x1)*(z2 - z1) - (z - z1)*(x2 - x1);

				if (d > 0) pos++;
				if (d < 0) neg++;

				//If the sign changes, then point is outside
				if (pos > 0 && neg > 0)
					return false;
			}

			//If no change in direction, then on same side of all segments, and thus inside
			return true;
		}

		NavMapShape* pointOnNavMesh(const Vector<NavMapShape*>& shapes, glm::vec3 point)
		{
			for (NavMapShape* shape : shapes)
				if (IsInConvexPolygon(shape->p, point))
					return shape;

			return nullptr;
		}

		NavMapShape* pointOnNavMesh(const utilities::BVH& bvh, glm::vec3 point)
		{
			glm::vec3 bl = point - 0.0001f;
			glm::vec3 tr = point + 0.0001f;

			auto user_datas = bvh.getAllUserDataInAABB(utilities::BVHAABB(bl, tr));
			Vector<NavMapShape*> shapes(user_datas.size());
			for (uint32_t i = 0; i < user_datas.size(); ++i)
				shapes[i] = (NavMapShape*)user_datas[i];

			return pointOnNavMesh(shapes, point);
		}

		float shortestDistance(glm::vec3 a, glm::vec3 b, glm::vec3 p)
		{
			glm::vec3 delta = b - a;
			glm::vec3 cross = glm::cross(delta, glm::vec3(0.0f, 1.0f, 0.0f));
			float length = glm::length(delta);
			delta /= length;

			auto ap = p - a;
			auto bp = p - b;

			float ret = 0.0f;
			float dot = glm::dot(delta, ap);
			if (dot <= 0.0f)
				ret = glm::length(ap);
			else if (dot >= length)
				ret = glm::length(bp);
			else
				ret = glm::dot(cross, ap);
			return ret >- 0.0f ? ret : -ret;
		}

		float distanceToShape(NavMapShape* s, glm::vec3 p)
		{
			if (IsInConvexPolygon(s->p, p))
				return 0.0f;

			float closestDistance = FLT_MAX;
			for (uint32_t i = 0; i < s->p.size(); ++i)
			{
				float distance = shortestDistance(s->p[i], s->p[(i + 1) % s->p.size()], p);
				if (distance < closestDistance)
					closestDistance = distance;
			}
			return closestDistance;
		}

		NavMapShape* closestNavMesh(const Vector<NavMapShape*>& shapes, glm::vec3 point, float maxDistance = FLT_MAX)
		{
			NavMapShape* closest = nullptr;
			float closestDistance = FLT_MAX;

			for (NavMapShape* shape : shapes)
			{
				float distance = distanceToShape(shape, point);
				if (distance < closestDistance)
				{
					closestDistance = distance;
					closest = shape;
				}

				if (distance == 0.0f)
					return shape;
			}

			if (closestDistance > maxDistance)
				return nullptr;

			return closest;
		}

		NavMapShape* closestNavMeshInArea(const utilities::BVH& bvh, glm::vec3 point, float radius)
		{
			Vector<void*> user_datas = bvh.getAllUserDataInAABB(utilities::BVHAABB(point - radius, point + radius));
			Vector<NavMapShape*> shapes(user_datas.size());
			for (uint32_t i = 0; i < user_datas.size(); ++i)
				shapes[i] = (NavMapShape*)user_datas[i];
			return closestNavMesh(shapes, point, radius);
		}

		Vector<glm::vec3> TriNavMap::findPath(glm::vec3 from, glm::vec3 to) const
		{
			Vector<glm::vec3> path;
			NavMapPoint* point_from = findClosest(from);
			NavMapPoint* point_to   = findClosest(to);

			if (point_from == nullptr || point_to == nullptr)
				return{};
			if (point_from->shape == point_to->shape)
				return{ from, to };

			Vector<NavMapPoint*> set_closed;
			Vector<NavMapPoint*> set_open = { point_from };
			UnorderedMap<NavMapPoint*, NavMapPoint*> came_from;
			UnorderedMap<NavMapPoint, NavMapPoint*> nav_map_points;
			UnorderedMap<NavMapPoint*, float> score_g;
			UnorderedMap<NavMapPoint*, float> score_f;
			score_f[point_from] = heuristicCostEstimate(point_from, point_to);
			score_g[point_from] = 0.0f;
			nav_map_points[*point_from] = point_from;
			nav_map_points[*point_to] = point_to;

			while (!set_open.empty())
			{
				NavMapPoint* current = nullptr;
				float lowest_score_f = FLT_MAX;

				for (NavMapPoint* point : set_open)
				{
					const float node_score_f = score_f[point];
					if (node_score_f < lowest_score_f)
					{
						lowest_score_f = node_score_f;
						current = point;
					}
				}

				set_open.erase(eastl::find(set_open.begin(), set_open.end(), current));
				set_closed.push_back(current);

				if (current->shape == point_to->shape)
				{
					if (point_to != current)
						came_from[point_to] = current;
					break;
				}

				bool found = false;
				Vector<NavMapShape*> neighbors;
				for (uint32_t i = 0; i < current->shape->p.size(); ++i)
				{
					if (equal(current->point, current->shape->p[i]))
					{
						neighbors = current->shape->share_polys[i];
						found = true;
					}
				}

				if (!found)
					neighbors.push_back(current->shape);

				for (NavMapShape* shape : neighbors)
				{
					for (const auto& p : shape->p)
					{
						if (equal(current->point, p))
							continue;

						NavMapPoint nav_map_point;
						nav_map_point.shape = shape;
						nav_map_point.point = p;
						NavMapPoint* neighbor = nav_map_points.find(nav_map_point) != nav_map_points.end() ? nav_map_points[nav_map_point] : nullptr;
						if (!neighbor)
							neighbor = nav_map_points[nav_map_point] = foundation::Memory::construct<NavMapPoint>(nav_map_point);

						if (eastl::find(set_closed.begin(), set_closed.end(), neighbor) == set_closed.end())
						{
							const float tenative_score_g = score_g[current] + heuristicCostEstimate(current, neighbor);
							bool valid = true;

							if (eastl::find(set_open.begin(), set_open.end(), neighbor) == set_open.end())
								set_open.push_back(neighbor);
							else if (tenative_score_g >= score_g[neighbor])
								valid = false;

							if (valid)
							{
								came_from[neighbor] = current;
								score_g[neighbor] = tenative_score_g;
								score_f[neighbor] = tenative_score_g + heuristicCostEstimate(neighbor, point_to);
							}
						}
					}
				}
			}
			
			struct Node
			{
				NavMapShape* tri;
				glm::vec3 p;
			};

			auto current = point_to;
			Vector<Node> p = { { current->shape, current->point } };

			while (current)
			{
				NavMapPoint* next = came_from[current];
				current = next;

				if (current)
					p.push_back({ current->shape, current->point });
				if (current == point_from)
					current = nullptr;
			}

			p.push_back({ point_from->shape, point_from->point });

			for (uint32_t i = 0; i < p.size();)
			{
				NavMapShape* prev = i > 0 ? p[i - 1].tri : nullptr;
				NavMapShape* curr = p[i].tri;
				NavMapShape* next = i < p.size() - 1 ? p[i + 1].tri : nullptr;

				if (prev && curr && next)
				{
					glm::vec3 delta = (p[i + 1].p - p[i - 1].p);
					float length = glm::length(delta);
					delta /= length;

					bool valid = true;
					for (float j = 0.0f; j < length; j += 1.0f)
					{
						glm::vec3 point = p[i - 1].p + delta * j;
						if (pointOnNavMesh(shapes_, point) == nullptr)
						{
							valid = false;
							break;
						}
					}

					if (valid)
					{
						p.erase(p.begin() + i);
					}
					else
					{
						path.push_back(p[i].p);
						i++;
					}
				}
				else
				{
					path.push_back(p[i].p);
					i++;
				}
			}

			std::reverse(path.begin(), path.end());

			for (const auto& it : nav_map_points)
				foundation::Memory::destruct(it.second);

			return path;
		}

		struct FindPathQueueInfo
		{
			TriNavMap* map;
			glm::vec3 from;
			glm::vec3 to;
			Promise<Vector<glm::vec3>>* promise;
		};

		void findPathQueued(void* user_data)
		{
			FindPathQueueInfo& fpqi = *(FindPathQueueInfo*)user_data;

			Vector<glm::vec3> path = fpqi.map->findPath(fpqi.from, fpqi.to);
			Promise<Vector<glm::vec3>>* promise = fpqi.promise;

			foundation::Memory::destruct(&fpqi);

			promise->g_mutex.lock();
			promise->t = path;
			promise->is_finished = true;
			promise->g_mutex.unlock();
		}

		Promise<Vector<glm::vec3>>* TriNavMap::findPathPromise(TriNavMap* map, glm::vec3 from, glm::vec3 to)
		{
			Promise<Vector<glm::vec3>>* promise = foundation::Memory::construct<Promise<Vector<glm::vec3>>>();
			promise->is_finished = false;

			FindPathQueueInfo* fpqi = foundation::Memory::construct<FindPathQueueInfo>();
			fpqi->map     = map;
			fpqi->from    = from;
			fpqi->to      = to;
			fpqi->promise = promise;

			TaskScheduler::queue(findPathQueued, fpqi, platform::TaskScheduler::kMedium);

			return promise;
		}

		NavMapPoint* TriNavMap::findClosest(glm::vec3 position) const
		{
			//NavMapShape* shape = closestNavMeshInArea(bvh_, position, 5.0f);
			NavMapShape* shape = closestNavMesh(shapes_, position);

			if (!shape)
				return nullptr;

			NavMapPoint* point = foundation::Memory::construct<NavMapPoint>();
			point->point = position;
			point->shape = shape;
			return point;
		}
		float TriNavMap::heuristicCostEstimate(NavMapPoint* a, NavMapPoint* b) const
		{
			return glm::length2(a->point - b->point);
		}
		bool NavMapPoint::operator==(const NavMapPoint& other) const
		{
			return shape == other.shape && equal(point, other.point);
		}
	}
}