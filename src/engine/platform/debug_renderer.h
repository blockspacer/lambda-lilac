#pragma once
#include "assets/mesh.h"
#include "shader_pass.h"

namespace lambda
{
  namespace world
  {
    class IWorld;
  }
  
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    struct DebugLine
    {
      DebugLine() : start(0.0f), end(0.0f) {}
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

    ///////////////////////////////////////////////////////////////////////////
    class DebugRenderer
    {
    public:
      void DrawLine(const DebugLine& line);
      void Render(world::IWorld* world);

    private:
      asset::MeshHandle mesh_;
      ShaderPass shader_pass_;
      Vector<glm::vec3> positions_;
      Vector<glm::vec4> colours_;
      //size_t idx_;
      //size_t last_size_;
    };
  }
}
