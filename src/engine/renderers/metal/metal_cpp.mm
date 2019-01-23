#include "renderers/metal/metal_cpp.h"
#include "utils/console.h"

// #include "argument.hpp"
#include <Metal/MTLArgument.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <AppKit/NSWindow.h>

#include <simd/simd.h>


const char* c_str = R"(
/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Metal shaders used for this sample
*/

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Include header shared between this Metal shader code and C code executing Metal API commands
/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Header containing types and enum constants shared between Metal shaders and C/ObjC source
*/

#ifndef AAPLShaderTypes_h
#define AAPLShaderTypes_h

#include <simd/simd.h>

// Buffer index values shared between shader and C code to ensure Metal shader buffer inputs match
//   Metal API buffer set calls
typedef enum AAPLVertexInputIndex
{
  AAPLVertexInputIndexPositions    = 0,
  AAPLVertexInputIndexColors       = 1,
  AAPLVertexInputIndexViewportSize = 2,
} AAPLVertexInputIndex;

#endif /* AAPLShaderTypes_h */


// Vertex shader outputs and fragment shader inputs
typedef struct
{
  // The [[position]] attribute of this member indicates that this value is the clip space
  // position of the vertex when this structure is returned from the vertex function
  float4 clipSpacePosition [[position]];

  // Since this member does not have a special attribute, the rasterizer interpolates
  // its value with the values of the other triangle vertices and then passes
  // the interpolated value to the fragment shader for each fragment in the triangle
  float4 color;

} RasterizerData;

// Vertex function
vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
constant vector_float2 *positions [[buffer(AAPLVertexInputIndexPositions)]],
constant vector_float4 *colors [[buffer(AAPLVertexInputIndexColors)]],
constant vector_uint2 *viewportSizePointer [[buffer(AAPLVertexInputIndexViewportSize)]])
{
RasterizerData out;

// Initialize our output clip space position
out.clipSpacePosition = vector_float4(0.0, 0.0, 0.0, 1.0);

// Index into our array of positions to get the current vertex
//   Our positions are specified in pixel dimensions (i.e. a value of 100 is 100 pixels from
//   the origin)
float2 pixelSpacePosition = positions[vertexID].xy;

// Dereference viewportSizePointer and cast to float so we can do floating-point division
vector_float2 viewportSize = vector_float2(*viewportSizePointer);

// The output position of every vertex shader is in clip-space (also known as normalized device
//   coordinate space, or NDC).   A value of (-1.0, -1.0) in clip-space represents the
//   lower-left corner of the viewport whereas (1.0, 1.0) represents the upper-right corner of
//   the viewport.

// Calculate and write x and y values to our clip-space position.  In order to convert from
//   positions in pixel space to positions in clip-space, we divide the pixel coordinates by
//   half the size of the viewport.
out.clipSpacePosition.xy = pixelSpacePosition / (viewportSize / 2.0);

// Pass our input color straight to our output color.  This value will be interpolated
//   with the other color values of the vertices that make up the triangle to produce
//   the color value for each fragment in our fragment shader
out.color = colors[vertexID];

return out;
}

// Fragment function
fragment float4 fragmentShader(RasterizerData in [[stage_in]])
{
// We return the color we just set which will be written to our color attachment.
return in.color;
})";




namespace lambda
{
  namespace osx
  {
    namespace metal
    {
      //////////////////////////////////////////////////////////////////////////
      bool createDeviceAndSwapchain(
        void* _window,
        void*& _device,
        void*& _swapchain)

      {
        // Create the device (GPU).
        const id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (device == nullptr)
        {
          LMB_ASSERT(false, "[METAL] Could not create device.");
          return false;
        }

        // Create the swapchain.
        CAMetalLayer* swapchain = [CAMetalLayer layer];
        if (swapchain == nullptr)
        {
          LMB_ASSERT(false, "[METAL] Could not create swapchain.");
          return false;
        }

        swapchain.device = device;
        swapchain.opaque = YES;

        // Bind the swapchain to the window.
        NSWindow* window = (NSWindow*)_window;
        if (window == nullptr)
        {
          LMB_ASSERT(false, "[METAL] Window was invalid.");
          return false;
        }

        window.contentView.layer      = swapchain;
        window.contentView.wantsLayer = YES;

        // Set the return values.
        _device    = (__bridge void*)device;
        _swapchain = (__bridge void*)swapchain;

        return true;
      }

      //////////////////////////////////////////////////////////////////////////
      bool createCommandQueue(
        void* _device,
        void*& _queue)

      {
        id<MTLDevice> device = (__bridge id<MTLDevice>)_device;

        // Create the command queue.
        const id<MTLCommandQueue> queue = [device newCommandQueue];
        if (queue == nullptr)
        {
          LMB_ASSERT(false, "[METAL] Could not create command queue.");
          return false;
        }

        // Set the return values.
        _queue     = (__bridge void*)queue;

        return true;
      }

      //////////////////////////////////////////////////////////////////////////
      bool createPipelineState(
        void* _device,
        void*& _pipeline_state)
      {
        id<MTLDevice> device = (__bridge id<MTLDevice>)_device;
        id<MTLRenderPipelineState> pipeline_state;

        NSString* source = [NSString
          stringWithCString: c_str
          encoding: NSASCIIStringEncoding];
        NSError* library_error = nullptr;

        // Load all the shader files with a .metal file extension in the project
        id<MTLLibrary> default_library = [device
           newLibraryWithSource: source
           options: nullptr
           error: &library_error];
        if (default_library == nullptr)
        {
          LMB_ASSERT(false, "[METAL] default library was invalid.");
          return false;
        }

        // Load the vertex function from the library
        id<MTLFunction> vertex_function =
          [default_library newFunctionWithName: @"vertexShader"];
        if (vertex_function == nullptr)
        {
          LMB_ASSERT(false, "[METAL] vertex function was invalid.");
          return false;
        }

        // Load the fragment function from the library
        id<MTLFunction> fragment_function =
          [default_library newFunctionWithName: @"fragmentShader"];
        if (fragment_function == nullptr)
        {
          LMB_ASSERT(false, "[METAL] fragment function was invalid.");
          return false;
        }

        // Configure a pipeline descriptor that is used to create a pipeline
        MTLRenderPipelineDescriptor* pipeline_state_descriptor =
          [[MTLRenderPipelineDescriptor alloc] init];
        if (pipeline_state_descriptor == nullptr)
        {
          LMB_ASSERT(false, "[METAL] pipeline state descriptor was invalid.");
          return false;
        }

        pipeline_state_descriptor.label            = @"Simple Pipeline";
        pipeline_state_descriptor.vertexFunction   = vertex_function;
        pipeline_state_descriptor.fragmentFunction = fragment_function;
        pipeline_state_descriptor.colorAttachments[0].pixelFormat =
          MTLPixelFormatRGBA8Unorm;

        NSError* error = nullptr;
        pipeline_state = [device
           newRenderPipelineStateWithDescriptor: pipeline_state_descriptor
           error: &error];

        if (pipeline_state == nullptr)
        {
          // Pipeline State creation could fail if we haven't properly set up
          // our pipeline descriptor.
          // If the Metal API validation is enabled, we can find out more
          // information about what went wrong.
          // (Metal API validation is enabled by default when a debug build
          // is run from Xcode)
          NSLog(@"Failed to created pipeline state, error %@", error);
          return false;
        }

        _pipeline_state = (__bridge void*)pipeline_state;

        return true;
      }

      //////////////////////////////////////////////////////////////////////////
      void resizeSwapchain(
        void* _swapchain,
        glm::uvec2 size
      )
      {
        CAMetalLayer* swapchain = (__bridge CAMetalLayer*)_swapchain;
        [swapchain setDrawableSize: { (double)size.x, (double)size.y } ];
      }

      //////////////////////////////////////////////////////////////////////////
      void draw(
        void* _device,
        void* _queue,
        void* _swapchain,
        void* _pipeline_state,
        glm::uvec2 window_size)
      {
        id<MTLDevice>       device    = (__bridge id<MTLDevice>)_device;
        id<MTLCommandQueue> queue     = (__bridge id<MTLCommandQueue>)_queue;
        CAMetalLayer*       swapchain = (__bridge CAMetalLayer*)_swapchain;
        id<MTLRenderPipelineState> pipeline_state =
          (__bridge id<MTLRenderPipelineState>)_pipeline_state;

        id<CAMetalDrawable> surface = [swapchain nextDrawable];

        MTLRenderPassDescriptor* render_pass_descriptor =
          [MTLRenderPassDescriptor renderPassDescriptor];
        render_pass_descriptor.colorAttachments[0].clearColor =
          MTLClearColorMake(0, 0, 0, 1);
        render_pass_descriptor.colorAttachments[0].loadAction =
          MTLLoadActionClear;
        render_pass_descriptor.colorAttachments[0].storeAction =
          MTLStoreActionStore;
        render_pass_descriptor.colorAttachments[0].texture =
          surface.texture;

        struct AAPLVertex
        {
          // Positions in pixel space
          // (e.g. a value of 100 indicates 100 pixels from the center)
          vector_float2 position;

          // Floating-point RGBA colors
          vector_float4 color;
        };

        glm::vec2 trianglePositions[4] = {
          {  250, -250},
          { -250, -250},
          {  250,  250},
          { -250,  250}
        };
        glm::vec4 triangleColors[4] = {
          {1, 0, 1, 1},
          {0, 1, 0, 1},
          {0, 0, 1, 1},
          {1, 1, 1, 1}
        };

        uint16_t indices[6] = {
          0, 1, 2,
          1, 3, 2
        };

        // Create a new command buffer for each render pass to the current
        // drawable
        id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
        command_buffer.label = @"MyCommand";

        if(render_pass_descriptor != nullptr)
        {
          // Create a render command encoder so we can render into something
          id<MTLRenderCommandEncoder> render_encoder =
            [command_buffer
             renderCommandEncoderWithDescriptor: render_pass_descriptor];
          render_encoder.label = @"MyRenderEncoder";

          // Set the region of the drawable to which we'll draw.
          [render_encoder setViewport:
            (MTLViewport){0.0, 0.0, (double)window_size.x, (double)window_size.y, -1.0, 1.0 }];

          [render_encoder setRenderPipelineState: pipeline_state];

          // We call -[MTLRenderCommandEncoder setVertexBytes:length:atIndex:]
          // to send data from our
          //   Application ObjC code here to our Metal 'vertexShader' function
          // This call has 3 arguments
          //   1) A pointer to the memory we want to pass to our shader
          //   2) The memory size of the data we want passed down
          //   3) An integer index which corresponds to the index of the buffer
          //   attribute qualifier
          //      of the argument in our 'vertexShader' function

          // You send a pointer to the `triangleVertices` array also and
          // indicate its size
          // The `AAPLVertexInputIndexVertices` enum value corresponds to the
          // `vertexArray`
          // argument in the `vertexShader` function because its buffer
          // attribute also uses
          // the `AAPLVertexInputIndexVertices` enum value for its index
          [render_encoder setVertexBytes: trianglePositions
                                  length: sizeof(trianglePositions)
                                 atIndex: 0];
          [render_encoder setVertexBytes: triangleColors
                                  length: sizeof(triangleColors)
                                 atIndex: 1 ];

          // You send a pointer to `_viewportSize` and also indicate its size
          // The `AAPLVertexInputIndexViewportSize` enum value corresponds to
          // the
          // `viewportSizePointer` argument in the `vertexShader` function
          // because its
          //  buffer attribute also uses the `AAPLVertexInputIndexViewportSize`
          //  enum value
          //  for its index

          [render_encoder setVertexBytes: &window_size
                                  length: sizeof(window_size)
                                 atIndex: 2];

          // Index buffer.
          id<MTLBuffer> index_buffer =
            [device newBufferWithBytes: indices
                                length: sizeof(indices)
                               options: MTLResourceOptionCPUCacheModeDefault ];

          // Draw.
          [render_encoder drawIndexedPrimitives: MTLPrimitiveTypeTriangle
                                     indexCount: 6
                                      indexType: MTLIndexTypeUInt16
                                    indexBuffer: index_buffer
                              indexBufferOffset: 0
                                  instanceCount: 1 ];

          [render_encoder endEncoding];

          // Schedule a present once the framebuffer is complete using the
          // current drawable
          [command_buffer presentDrawable: surface];
        }

        // Finalize rendering here & push the command buffer to the GPU
        [command_buffer commit];
      }
    }
  }
}