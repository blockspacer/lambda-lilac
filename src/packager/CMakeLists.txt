SET(CompilersSources
  "compilers/shader_compiler.h"
  "compilers/shader_compiler.cc"
  "compilers/shader_includer.h"
  "compilers/shader_includer.cc"
  "compilers/shader_pass_compiler.h"
  "compilers/shader_pass_compiler.cc"
  "compilers/texture_compiler.h"
  "compilers/texture_compiler.cc"
  "compilers/wave_compiler.h"
  "compilers/wave_compiler.cc"
)

SOURCE_GROUP("compilers" FILES ${CompilersSources})

SET(Sources
  ${CompilersSources}
)

IF(NOT ${VIOLET_CONFIG_FOUNDATION})
  FATAL_ERROR("Tools requires Foundation")
ENDIF()

ADD_LIBRARY(lambda-packager ${Sources})
TARGET_LINK_LIBRARIES(lambda-packager PUBLIC lambda-foundation soloud hlslcc ShaderConductor)
#TARGET_LINK_LIBRARIES(lambda-packager PUBLIC ${CMAKE_SOURCE_DIR}/deps/DXC/lib/dxcompiler.lib)

IF(${VIOLET_USE_DIRECTX_TEX})
  TARGET_LINK_LIBRARIES(lambda-packager PUBLIC directxtex)
ELSE()
  TARGET_LINK_LIBRARIES(lambda-packager PUBLIC stb)
ENDIF()

IF(${VIOLET_USE_HLSLCC})
  TARGET_INCLUDE_DIRECTORIES(lambda-packager PUBLIC "../../deps/HLSLcc/include" "../../deps/HLSLcc/src/internal_includes" "../../deps/HLSLcc/src/cbstring" "../../deps/HLSLcc/src")
ENDIF()

TARGET_INCLUDE_DIRECTORIES(lambda-packager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
