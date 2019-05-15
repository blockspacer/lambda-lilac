#include "nav_mesh.h"
#include <memory/memory.h>
#include <glm/gtx/norm.hpp>
#include <utils/console.h>
#include <algorithm>

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

		static inline bool equal(const glm::vec2& a, const glm::vec2& b)
		{
			static constexpr float kEpsilon = 0.001f;
			const glm::bvec2 res = glm::epsilonEqual(a, b, kEpsilon);
			return res.x && res.y;
		}

		void TriNavMap::addTri(glm::vec2 a, glm::vec2 b, glm::vec2 c)
		{
			addShape({ a, b, c });
		}

		void TriNavMap::addTriHole(glm::vec2 a, glm::vec2 b, glm::vec2 c)
		{
			addTri(c, b, a);
		}

		void TriNavMap::addQuad(glm::vec2 bl, glm::vec2 tr)
		{
			glm::vec2 a, b, c, d;
			a.x = bl.x; a.y = tr.y;
			b.x = tr.x; b.y = tr.y;
			c.x = tr.x; c.y = bl.y;
			d.x = bl.x; d.y = bl.y;

			addShape({ a, b, c, d });
		}

		void TriNavMap::addShape(Vector<glm::vec2> points)
		{
			NavMapShape* shape = foundation::Memory::construct<NavMapShape>();
			shape->p = points;
			shape->share_polys.resize(points.size());

			glm::vec2 bl(FLT_MAX);
			glm::vec2 tr(FLT_MIN);

			for (const glm::vec2& p : points)
			{
				bl.x = std::min(bl.x, p.x);
				bl.y = std::min(bl.y, p.y);
				tr.x = std::max(tr.x, p.x);
				tr.y = std::max(tr.y, p.y);
			}

			Vector<void*> user_data = bvh_.getAllUserDataInAABB(utilities::BVHAABB(glm::vec3(bl.x - 1.0f, 0.0f, bl.y - 1.0f), glm::vec3(tr.x + 1.0f, 1.0f, tr.y + 1.0f)));

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

			bvh_.add((entity::Entity)shapes_.size(), shape, utilities::BVHAABB(glm::vec3(bl.x, 0.0f, bl.y), glm::vec3(tr.x, 1.0f, tr.y)));
			shapes_.push_back(shape);
		}

		void TriNavMap::addQuadHole(glm::vec2 bl, glm::vec2 tr)
		{
			addQuad(tr, bl);
		}

		Vector<glm::vec2> TriNavMap::getTris()
		{
			LMB_ASSERT(!shapes_.empty(), "TRI NAV MAP: Tried to get the tris from an empty navigation map");
			
			Vector<glm::vec2> tris;

			struct Line
			{
				glm::vec2 a;
				glm::vec2 b;
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
		Vector<glm::vec3> TriNavMap::findPath(glm::vec3 from, glm::vec3 to)
		{
			return findPath(glm::vec2(from.x, from.z), glm::vec2(to.x, to.z));
		}

		bool IsInConvexPolygon(const Vector<glm::vec2>& polygon, glm::vec2 testPoint)
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

				//And the i+1'th, or if i is the last, with the first point
				int i2 = i < polygon.size() - 1 ? i + 1 : 0;

				float x2 = polygon[i2].x;
				float y2 = polygon[i2].y;

				float x = testPoint.x;
				float y = testPoint.y;

				//Compute the cross product
				float d = (x - x1)*(y2 - y1) - (y - y1)*(x2 - x1);

				if (d > 0) pos++;
				if (d < 0) neg++;

				//If the sign changes, then point is outside
				if (pos > 0 && neg > 0)
					return false;
			}

			//If no change in direction, then on same side of all segments, and thus inside
			return true;
		}

		NavMapShape* pointOnNavMesh(const utilities::BVH& bvh, glm::vec2 point)
		{
			glm::vec2 bl = point - 0.01f;
			glm::vec2 tr = point + 0.01f;

			auto user_datas = bvh.getAllUserDataInAABB(utilities::BVHAABB(glm::vec3(bl.x, 0.0f, bl.y), glm::vec3(tr.x, 1.0f, tr.y)));
			auto size = user_datas.size();
			for (void* ud : user_datas)
			{
				NavMapShape* shape = (NavMapShape*)ud;
				if (IsInConvexPolygon(shape->p, point))
					return shape;
			}

			return nullptr;
		}

		Vector<glm::vec3> TriNavMap::findPath(glm::vec2 from, glm::vec2 to)
		{
			Vector<glm::vec3> path;
			NavMapPoint* tri_from = findClosest(from);
			NavMapPoint* tri_to   = findClosest(to);

			if (tri_from == nullptr || tri_to == nullptr)
				return{};
			if (tri_from->shape == tri_to->shape)
				return{ glm::vec3(from.x, 0.0f, from.y), glm::vec3(to.x, 0.0f, to.y) };

			Vector<NavMapPoint*> set_closed;
			Vector<NavMapPoint*> set_open = { tri_from };
			UnorderedMap<NavMapPoint*, NavMapPoint*> came_from;
			UnorderedMap<NavMapPoint, NavMapPoint*> nav_map_points;
			UnorderedMap<NavMapPoint*, float> score_g;
			UnorderedMap<NavMapPoint*, float> score_f;
			score_f[tri_from] = heuristicCostEstimate(tri_from, tri_to);
			score_g[tri_from] = 0.0f;
			nav_map_points[*tri_from] = tri_from;
			nav_map_points[*tri_to] = tri_to;

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

				if (current->shape == tri_to->shape)
				{
					came_from[tri_to] = current;
					current = tri_to;

					struct Node
					{
						NavMapShape* tri;
						glm::vec3 p;
					};

					Vector<Node> p = { { current->shape, glm::vec3(current->point.x, 0.0f, current->point.y) } };

					while (current)
					{
						current = came_from[current];
						if (current)
							p.push_back({ current->shape, glm::vec3(current->point.x, 0.0f, current->point.y) });
						if (current == tri_from)
							current = nullptr;
					}

					p.push_back({ tri_from->shape, glm::vec3(tri_from->point.x, 0.0f, tri_from->point.y) });

					auto to3 = [](glm::vec2 v) { return glm::vec3(v.x, 0.0f, v.y); };

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
								if (pointOnNavMesh(bvh_, glm::vec2(point.x, point.z)) == nullptr)
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
					break;
				}

				set_open.erase(eastl::find(set_open.begin(), set_open.end(), current));
				set_closed.push_back(current);

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
								score_f[neighbor] = tenative_score_g + heuristicCostEstimate(neighbor, tri_to);
							}
						}
					}
				}
			}

			for (const auto& it : nav_map_points)
				foundation::Memory::destruct(it.second);

			return path;
		}

		static inline bool sameSide(glm::vec3 p1, glm::vec3 p2, glm::vec3 a, glm::vec3 b)
		{
			glm::vec3 cp1 = glm::cross(b - a, p1 - a);
			glm::vec3 cp2 = glm::cross(b - a, p2 - a);
			return (glm::dot(cp1, cp2) >= 0);
		}

		static inline bool pointInTriangle(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c)
		{
			return (sameSide(p, a, b, c) && sameSide(p, b, a, c) && sameSide(p, c, a, b));
		}

		NavMapPoint* TriNavMap::findClosest(glm::vec2 position)
		{
			glm::vec2 bl = position - 0.01f;
			glm::vec2 tr = position + 0.01f;

			NavMapShape* shape = pointOnNavMesh(bvh_, position);
			if (shape)
			{
				NavMapPoint* point = foundation::Memory::construct<NavMapPoint>();
				point->point = position;
				point->shape = shape;
				return point;
			}

			return nullptr;
		}
		float TriNavMap::heuristicCostEstimate(NavMapPoint* a, NavMapPoint* b)
		{
			return glm::length2(a->point - b->point);
		}
		bool NavMapPoint::operator==(const NavMapPoint& other) const
		{
			return shape == other.shape && equal(point, other.point);
		}
	}
}