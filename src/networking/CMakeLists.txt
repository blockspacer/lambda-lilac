SET(NetworkingSources
  "client.h"
  "client.cc"
  "dll.h"
  "dll.cc"
  "message.h"
  "message.cc"
  "message_manager.h"
  "message_manager.cc"
  "messages.h"
  "networking.h"
  "networking.cc"
  "server.h"
  "server.cc"
)

SOURCE_GROUP("networking" FILES ${NetworkingSources})

SET(Sources
  ${NetworkingSources}
)

IF(NOT ${VIOLET_CONFIG_FOUNDATION})
  FATAL_ERROR("Networking requires Foundation")
ENDIF()

ADD_LIBRARY(lambda-networking ${Sources})
TARGET_LINK_LIBRARIES(lambda-networking PUBLIC lambda-foundation enet)

TARGET_LINK_LIBRARIES(lambda-networking PUBLIC ws2_32.lib winmm.lib)

TARGET_INCLUDE_DIRECTORIES(lambda-networking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(lambda-networking PRIVATE LAMBDA_NETWORKING_EXPORT)
