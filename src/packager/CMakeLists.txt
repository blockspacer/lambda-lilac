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
TARGET_LINK_LIBRARIES(lambda-packager PUBLIC lambda-foundation soloud)

IF(${VIOLET_SHADER_CONDUCTOR})
  TARGET_LINK_LIBRARIES(lambda-packager PUBLIC ShaderConductor)
	TARGET_COMPILE_DEFINITIONS(lambda-packager PRIVATE VIOLET_SHADER_CONDUCTOR)
ENDIF()

IF(${VIOLET_DIRECTX_TEX})
  TARGET_LINK_LIBRARIES(lambda-packager PUBLIC directxtex)
	TARGET_COMPILE_DEFINITIONS(lambda-packager PRIVATE VIOLET_DIRECTX_TEX)
ELSE()
  TARGET_LINK_LIBRARIES(lambda-packager PUBLIC stb)
	TARGET_COMPILE_DEFINITIONS(lambda-packager PRIVATE VIOLET_STB)
ENDIF()

TARGET_INCLUDE_DIRECTORIES(lambda-packager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
