CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules ${CMAKE_CURRENT_SOURCE_DIR}/cmake")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DVIOLET_DEBUG")

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

SET(CMAKE_CXX_FLAGS_RELEASE "/MT")
SET(CMAKE_CXX_FLAGS_DEBUG "/MTd")
SET(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
FOREACH(CompilerFlag ${CompilerFlags})
  STRING(REPLACE "/MT" "/MD" ${CompilerFlag} "${${CompilerFlag}}")
ENDFOREACH()
FOREACH(CompilerFlag ${CompilerFlags})
  STRING(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
ENDFOREACH()

# Add all platform defines.
IF (WIN32)
  SET(VIOLET_WIN32 ON)
	ADD_DEFINITIONS (-DVIOLET_WIN32)
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(VIOLET_LINUX ON)
	ADD_DEFINITIONS (-DVIOLET_LINUX)
ELSEIF (APPLE)
  SET(VIOLET_OSX ON)
	ADD_DEFINITIONS (-DVIOLET_OSX)
ENDIF (WIN32)

# Disable RTTI
#if (VIOLET_WIN32)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
#elseif (VIOLET_LINUX || VIOLET_OSX)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
#endif()


SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DVIOLET_DEBUG=1")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DVIOLET_DEBUG=1")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DVIOLET_RELEASE=1")
SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DVIOLET_RELEASE=1")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DVIOLET_DEBUG=1")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -DVIOLET_DEBUG=1")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DVIOLET_RELEASE=1")
SET(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -DVIOLET_RELEASE=1")


# Do required things.
IF (VIOLET_WIN32)
#  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")

  SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
  SET_PROPERTY(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake-targets")
ELSEIF (VIOLET_LINUX)
#  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ELSEIF (VIOLET_OSX)
#  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ELSE ()
  MESSAGE(FATAL_ERROR "Unknown copmiler platform")
ENDIF (VIOLET_WIN32)

set(CompilerFlags
  CMAKE_C_FLAGS
  CMAKE_C_FLAGS_DEBUG
  CMAKE_C_FLAGS_MINSIZEREL
  CMAKE_C_FLAGS_RELEASE
  CMAKE_C_FLAGS_RELWITHDEBINFO
  CMAKE_CXX_FLAGS
  CMAKE_CXX_FLAGS_DEBUG
  CMAKE_CXX_FLAGS_MINSIZEREL
  CMAKE_CXX_FLAGS_RELEASE
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
)
#foreach(CompilerFlag ${CompilerFlags})
#  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
#endforeach()

# ///////////////////////////////////////////////////////////////
# /// BOOLEANS //////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
# /// CONFIG .../////////////////////////////////////////////////
SET(VIOLET_CONFIG_FOUNDATION TRUE CACHE BOOL "[CORE] Should Foundation be build?")
SET(VIOLET_CONFIG_ENGINE TRUE CACHE BOOL "[CORE] Should Engine be build?")

IF(${VIOLET_WIN32})
  SET(VIOLET_CONFIG_TOOLS FALSE CACHE BOOL "[CORE] Should Tools be build?")
ELSE()
  SET(VIOLET_CONFIG_TOOLS FALSE CACHE INTERNAL "[CORE] Tools not available on this platform.")
ENDIF()

IF(${VIOLET_WIN32})
  SET(VIOLET_CONFIG_NETWORKING FALSE CACHE BOOL "[CORE] Should Networking be build?")
ELSE()
  SET(VIOLET_CONFIG_NETWORKING FALSE CACHE INTERNAL "[CORE] Networking not available on this platform.")
ENDIF()

# /// FOUNDATION ////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
IF(${VIOLET_CONFIG_FOUNDATION})
ENDIF()

# /// NETWORKING ////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
IF(${VIOLET_CONFIG_NETWORKING})
ENDIF()

# /// ENGINE ////////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
IF(${VIOLET_CONFIG_ENGINE})
# /// RENDERERS /////////////////////////////////////////////////
  SET(VIOLET_RENDERER_AVAILABLE "${VIOLET_RENDERER_AVAILABLE};No")
  SET(VIOLET_RENDERER_DEFAULT "No")

  IF(${VIOLET_WIN32})
    SET(VIOLET_RENDERER_AVAILABLE "${VIOLET_RENDERER_AVAILABLE};D3D11;Vulkan")
    SET(VIOLET_RENDERER_DEFAULT "D3D11")
  ENDIF()

  IF(${VIOLET_OSX})
    SET(VIOLET_RENDERER_AVAILABLE "${VIOLET_RENDERER_AVAILABLE};Metal")
    SET(VIOLET_RENDERER_DEFAULT "Metal")
  ENDIF()

  IF(${VIOLET_LINUX})
    SET(VIOLET_RENDERER_AVAILABLE "${VIOLET_RENDERER_AVAILABLE};Metal;Vulkan")
    SET(VIOLET_RENDERER_DEFAULT "Vulkan")
  ENDIF()

  
  SET(VIOLET_RENDERER ${VIOLET_RENDERER_DEFAULT} CACHE STRING "Which renderer should be used?")
  SET_PROPERTY(CACHE VIOLET_RENDERER PROPERTY STRINGS ${VIOLET_RENDERER_AVAILABLE})

# /// WINDOWS ///////////////////////////////////////////////////
  SET(VIOLET_WINDOW_AVAILABLE "${VIOLET_WINDOW_AVAILABLE};GLFW;SDL2")
  SET(VIOLET_WINDOW_DEFAULT "GLFW")

  IF(${VIOLET_WIN32})
    SET(VIOLET_WINDOW_AVAILABLE "${VIOLET_WINDOW_AVAILABLE};Win32")
    SET(VIOLET_WINDOW_DEFAULT "Win32")
  ENDIF()

  SET(VIOLET_WINDOW ${VIOLET_WINDOW_DEFAULT} CACHE STRING "Which window should be used?")
  SET_PROPERTY(CACHE VIOLET_WINDOW PROPERTY STRINGS ${VIOLET_WINDOW_AVAILABLE})

# /// SCRIPTING /////////////////////////////////////////////////
  SET(VIOLET_SCRIPTING_AVAILABLE "${VIOLET_SCRIPTING_AVAILABLE};Wren;AngelScript")
  SET(VIOLET_SCRIPTING_DEFAULT "Wren")

  SET(VIOLET_SCRIPTING ${VIOLET_SCRIPTING_DEFAULT} CACHE STRING "Which scripting engine should be used?")
  SET_PROPERTY(CACHE VIOLET_SCRIPTING PROPERTY STRINGS ${VIOLET_SCRIPTING_AVAILABLE})

# /// IMGUI /////////////////////////////////////////////////////
  SET(VIOLET_IMGUI_AVAILABLE "${VIOLET_IMGUI_AVAILABLE};No;Dear ImGUI;Nuklear")
  SET(VIOLET_IMGUI_DEFAULT "No")

  SET(VIOLET_IMGUI ${VIOLET_IMGUI_DEFAULT} CACHE STRING "Which ImGUI should be used?")
  SET_PROPERTY(CACHE VIOLET_IMGUI PROPERTY STRINGS ${VIOLET_IMGUI_AVAILABLE})
  
# /// GUI ///////////////////////////////////////////////////////
  SET(VIOLET_GUI_AVAILABLE "${VIOLET_GUI_AVAILABLE};No;Ultralight")
  SET(VIOLET_GUI_DEFAULT "Ultralight")

  SET(VIOLET_GUI ${VIOLET_GUI_DEFAULT} CACHE STRING "Which GUI should be used?")
  SET_PROPERTY(CACHE VIOLET_GUI PROPERTY STRINGS ${VIOLET_GUI_AVAILABLE})

# /// PHYSICS ///////////////////////////////////////////////////
  SET(VIOLET_PHYSICS_AVAILABLE "${VIOLET_PHYSICS_AVAILABLE};Bullet3;React")
  SET(VIOLET_PHYSICS_DEFAULT "Bullet3")

  SET(VIOLET_PHYSICS ${VIOLET_PHYSICS_DEFAULT} CACHE STRING "Which physics engine should be used?")
  SET_PROPERTY(CACHE VIOLET_PHYSICS PROPERTY STRINGS ${VIOLET_PHYSICS_AVAILABLE})  
ENDIF()


# /// TOOLS /////////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
IF(${VIOLET_CONFIG_TOOLS})
	SET(VIOLET_DIRECTX_TEX FALSE CACHE BOOL "[TOOLS] Should DirectXTex be used?")
	SET(VIOLET_SHADER_CONDUCTOR FALSE CACHE BOOL "[TOOLS] Should ShaderConductor be used?")
ENDIF()
# ///////////////////////////////////////////////////////////////
# /// BOOLEANS //////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////


# Create the project.
PROJECT(lambda-violet)

# Add all dependencies folders.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(LinkDependencies)
LinkDependencies()

ADD_SUBDIRECTORY("src")
