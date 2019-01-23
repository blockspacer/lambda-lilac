#pragma once
#include <glm/glm.hpp>

namespace lambda
{
  namespace osx
  {
    namespace metal
    {
      //////////////////////////////////////////////////////////////////////////
      bool createDeviceAndSwapchain(
        void*  window,
        void*& device,
        void*& swapchain);

      bool createCommandQueue(
        void*  device,
        void*& queue);

      bool createPipelineState(
        void*  device,
        void*& pipeline_state);

      void resizeSwapchain(
        void* _swapchain,
        glm::uvec2 size
      );

      void draw(
        void* device,
        void* queue,
        void* swapchain,
        void* pipeline_state,
        glm::uvec2 window_size);
    }
  }
}