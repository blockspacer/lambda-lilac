#pragma once
#include <stdint.h>
#include "message.h"

#if VIOLET_WIN32
#define LAMBDA_DLL_EXPORT __declspec(dllexport)
#else
#define LAMBDA_DLL_EXPORT __attribute__((visibility("default")))
#endif

#if defined (LAMBDA_NETWORKING_EXPORT)
#define LAMBDA_NETWORKING_API extern "C" LAMBDA_DLL_EXPORT
#else
#define LAMBDA_NETWORKING_API extern "C" LAMBDA_DLL_EXPORT
#endif
#define LAMBDA_MAX_HEADER_LENGTH 255u
#define LAMBDA_MAX_MESSAGE_LENGTH 255u


namespace lambda
{
  namespace networking
  {
    LAMBDA_NETWORKING_API void networkingInitialize();
    LAMBDA_NETWORKING_API void networkingDeinitialize();

    LAMBDA_NETWORKING_API unsigned int clientInitialize(const char* name, uint16_t port = 1234u, uint32_t host = 0);
    LAMBDA_NETWORKING_API bool clientPollMessage(unsigned int client, const char* header, const char* message);
    LAMBDA_NETWORKING_API void clientSendMessage(unsigned int client, const char* header, const char* message);
    LAMBDA_NETWORKING_API void clientUpdate(unsigned int client, double delta_time);
    LAMBDA_NETWORKING_API void clientDeinitialize(unsigned int client);
    LAMBDA_NETWORKING_API bool clientIsConnected(unsigned int client);
    LAMBDA_NETWORKING_API void clientSetUpdateRate(unsigned int client, unsigned char hertz);
    LAMBDA_NETWORKING_API unsigned char clientGetUpdateRate(unsigned int client);

    LAMBDA_NETWORKING_API void serverInitialize(uint16_t port = 1234u, uint32_t host = 0);
    LAMBDA_NETWORKING_API void serverUpdate(double delta_time);
    LAMBDA_NETWORKING_API void serverDeinitialize();
    LAMBDA_NETWORKING_API void serverSetUpdateRate(unsigned int client, unsigned char hertz);
    LAMBDA_NETWORKING_API unsigned char serverGetUpdateRate(unsigned int client);
  }
}
