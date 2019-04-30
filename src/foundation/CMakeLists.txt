SET(AssetsSources
  "assets/base_asset_manager.h"
  "assets/base_asset_manager.cc"
  "assets/enums.h"
  "assets/mesh_manager.h"
  "assets/mesh_manager.cc"
  "assets/shader_manager.h"
  "assets/shader_manager.cc"
  "assets/shader_pass_manager.h"
  "assets/shader_pass_manager.cc"
  "assets/texture_manager.h"
  "assets/texture_manager.cc"
  "assets/wave_manager.h"
  "assets/wave_manager.cc"
)
SET(ContainersSources
  "containers/containers.h"
)
SET(MemorySources
  "memory/eastl_allocator.h"
  "memory/eastl_allocator.cc"
  "memory/frame_heap.h"
  "memory/frame_heap.cc"
  "memory/iallocator.h"
  "memory/iallocator.cc"
  "memory/malloc_allocator.h"
  "memory/malloc_allocator.cc"
  "memory/memory.h"
  "memory/memory.cc"
  "memory/pointer_arithmetic.h"
)
SET(PackageSources
  "package/package.h"
  "package/package.cc"
)
SET(UtilsSources
  "utils/decompose_matrix.h"
  "utils/colour.h"
  "utils/colour.cc"
  "utils/console.h"
  "utils/console.cc"
  "utils/stack_trace.h"
  "utils/stack_trace.cc"
  "utils/file_system.h"
  "utils/file_system.cc"
  "utils/profiler.h"
  "utils/profiler.cc"
  "utils/timer.h"
  "utils/timer.cc"
  "utils/utilities.h"
)

SOURCE_GROUP("assets" FILES ${AssetsSources})
SOURCE_GROUP("containers" FILES ${ContainersSources})
SOURCE_GROUP("memory" FILES ${MemorySources})
SOURCE_GROUP("package" FILES ${PackageSources})
SOURCE_GROUP("utils" FILES ${UtilsSources})

SET(Sources
  ${AssetsSources}
  ${ContainersSources}
  ${MemorySources}
  ${PackageSources}
  ${UtilsSources}
)

ADD_LIBRARY(lambda-foundation ${Sources})
TARGET_LINK_LIBRARIES(lambda-foundation PUBLIC eastl glm lz4 rapidjson StackWalker)

TARGET_INCLUDE_DIRECTORIES(lambda-foundation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
