#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>

namespace lambda
{
  namespace utilities
  {
    inline void decomposeMatrix(const glm::mat4& matrix, glm::vec3& scale, glm::quat& orientation, glm::vec3& translation)
    {
      glm::mat4 local_matrix(matrix);

      // Next take care of translation (easy).
      translation = glm::vec3(local_matrix[3]);
      local_matrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, local_matrix[3][3]);

      glm::vec3 row[3];

      // Now get scale and shear.
      for (glm::length_t i = 0; i < 3; ++i)
      {
        for (glm::length_t j = 0; j < 3; ++j)
        {
          row[i][j] = local_matrix[i][j];
        }
      }

      // Compute X scale factor and normalize first row.
      scale.x = glm::length(row[0]);
      row[0] /= scale.x;

      // Now, compute Y scale and normalize 2nd row.
      scale.y = glm::length(row[1]);
      row[1] /= scale.y;

      // Next, get Z scale and normalize 3rd row.
      scale.z = glm::length(row[2]);
      row[2] /= scale.z;

      // At this point, the matrix (in rows[]) is orthonormal.
      // Check for a coordinate system flip.  If the determinant
      // is -1, then negate the matrix and the scaling factors.
      glm::vec3 p_dum3 = glm::cross(row[1], row[2]);
      if (glm::dot(row[0], p_dum3) < 0)
      {
        for (glm::length_t i = 0; i < 3; i++)
        {
          scale[i] *= -1.0f;
          row[i] *= -1.0f;
        }
      }

      float root, trace = row[0].x + row[1].y + row[2].z;
      if (trace > 0.0f)
      {
        root = glm::sqrt(trace + 1.0f);
        orientation.w = 0.5f * root;
        root = 0.5f / root;
        orientation.x = root * (row[1].z - row[2].y);
        orientation.y = root * (row[2].x - row[0].z);
        orientation.z = root * (row[0].y - row[1].x);
      } // End if > 0
      else
      {
        constexpr int next[3] = { 1, 2, 0 };

        int i = 0;

        if (row[1].y > row[0].x)
        {
          i = 1;
        }

        if (row[2].z > row[i][i])
        {
          i = 2;
        }

        int j = next[i];
        int k = next[j];

        root = glm::sqrt(row[i][i] - row[j][j] - row[k][k] + 1.0f);

        orientation[i] = 0.5f * root;
        root = 0.5f / root;
        orientation[j] = root * (row[i][j] + row[j][i]);
        orientation[k] = root * (row[i][k] + row[k][i]);
        orientation.w = root * (row[j][k] - row[k][j]);
      } // End if <= 0
    }
  }
}
