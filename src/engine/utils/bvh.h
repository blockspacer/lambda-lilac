#pragma once
#include <containers/containers.h>
#include <glm/vec2.hpp>
#include <mutex>
#include <systems/entity.h>

namespace lambda
{
  namespace platform
  {
    class DebugRenderer;
  }
  namespace utilities
  {
	  struct BVHAABB
	  {
		  BVHAABB();
		  BVHAABB(const glm::vec2& bl, const glm::vec2& tr);
		  BVHAABB(const BVHAABB& other);

		  glm::vec2 bl; // Bottom left;
		  glm::vec2 tr; // Top Right.

		  glm::vec2 center;
		  glm::vec2 size;
		  BVHAABB combine(const BVHAABB& other) const;
		  bool intersects(const BVHAABB& other) const;
	  };

	  struct BVHNode
	  {
		  entity::Entity entity;
		  void* user_data;
		  BVHNode* parent;
		  BVHNode* child_left;
		  BVHNode* child_right;
		  BVHAABB aabb;
		  bool is_leaf;
	  };

	  class BaseBVH
	  {
	  public:
		  void add(const entity::Entity& entity, void* user_data, const BVHAABB& aabb);
		  void remove(const entity::Entity& entity);
		  void clear();

		  void draw(platform::DebugRenderer* renderer) const;

		  Vector<void*> getAllUserDataInAABB(const BVHAABB& aabb) const;
		  Vector<entity::Entity> getAllEntityInAABB(const BVHAABB& aabb) const;

	  protected:
		  void findAndRemove(BVHNode* node, const entity::Entity& entity);
		  void remove(BVHNode* node);
		  void clear(BVHNode* node);
		  virtual void privateRemove(BVHNode* node) = 0;
		  void refit(BVHNode* node);

		  void getUserDataInAABB(Vector<void*>& user_datas, const BVHAABB& aabb, BVHNode* node) const;
		  void getEntityInAABB(Vector<entity::Entity>& entities, const BVHAABB& aabb, BVHNode* node) const;

		  virtual BVHNode* privateCreate() = 0;
		  void insert(BVHNode* node, BVHNode* parent);
		  void drawNode(BVHNode* node, size_t depth, platform::DebugRenderer* renderer) const;
		  void drawNode(BVHNode* node, glm::vec2 prev, float child, platform::DebugRenderer* renderer) const;

	  protected:
		  BVHNode* base_node_ = nullptr;
		  size_t   num_nodes_ = 0ull;
	  };

	  class BVH : public BaseBVH
	  {
	  private:
		  virtual void privateRemove(BVHNode* node) override;
		  virtual BVHNode* privateCreate() override;
	  };

	  class TransientBVH : public BaseBVH
	  {
	  private:
		  virtual void privateRemove(BVHNode* node) override;
		  virtual BVHNode* privateCreate() override;
	  };
  }
}