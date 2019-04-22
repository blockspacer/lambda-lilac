#pragma once
#include "systems/entity.h"
#include <glm/glm.hpp>
#include "assets/mesh.h"
#include "assets/texture.h"

namespace lambda
{
  namespace utilities
  {
    struct Renderable
    {
      entity::Entity entity;
      glm::mat4x4 model_matrix;
      asset::VioletMeshHandle mesh;
      uint32_t sub_mesh = 0u;
      asset::VioletTextureHandle albedo_texture;
      asset::VioletTextureHandle normal_texture;
			asset::VioletTextureHandle dmra_texture;
			asset::VioletTextureHandle emissive_texture;
      float metallicness = 0.0f;
			float roughness    = 1.0f;
			glm::vec3 emissiveness;
      glm::vec3 min;
      glm::vec3 max;
      glm::vec3 center;
      float     radius;
    };

  }
}