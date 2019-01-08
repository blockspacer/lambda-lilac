SET(BuilderSources
  "main.cc"
)

SOURCE_GROUP("builder" FILES ${BuilderSources})

SET(Sources
  ${BuilderSources}
)

IF(NOT ${VIOLET_CONFIG_FOUNDATION})
  FATAL_ERROR("Tools requires Foundation")
ENDIF()

ADD_EXECUTABLE(lambda-builder ${Sources})
TARGET_LINK_LIBRARIES(lambda-builder PUBLIC lambda-foundation lambda-packager)

TARGET_INCLUDE_DIRECTORIES(lambda-builder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
