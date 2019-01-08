#include "dll.h"
#include "networking.h"
#include <unordered_map>
#include <assert.h>
#include <containers/containers.h>
#include <utils/console.h>
#include <iostream>

namespace lambda
{
  namespace networking
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static Server g_server;
    static UnorderedMap<unsigned int, Client> g_clients;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Client& get(unsigned int client)
    {
      auto it = g_clients.find(client);
      if (it == g_clients.end())
      {
        LMB_LOG_ERR("Client with ID %i was never created.", client);
      }

      return it->second;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API void networkingInitialize()
    {
      initializeNetworking();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API void networkingDeinitialize()
    {
      deinitializeNetworking();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API unsigned int clientInitialize(const char* name, uint16_t port, uint32_t host)
    {
      static unsigned int id = 0;
      unsigned int assigned_id = id++;
      g_clients.insert(eastl::make_pair(assigned_id, Client()));
      get(assigned_id).initialize(name, port, host);
      return assigned_id;
    }
      
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API bool clientPollMessage(unsigned int client, const char* header, const char* message)
    {
      Message msg;
      if (get(client).pollMessages(msg))
      {
        assert(msg.header_size < LAMBDA_MAX_HEADER_LENGTH);
        assert(msg.message_size < LAMBDA_MAX_MESSAGE_LENGTH);

        unsigned char null_char = '\0';

#if VIOLET_OSX
        memcpy((void*)header, msg.header.data(), msg.header.size());
        memcpy((void*)(header + msg.header_size), &null_char, 1);
        memcpy((void*)message, msg.message.data(), msg.message.size());
        memcpy((void*)(message + msg.message_size), &null_char, 1);
#else
        memcpy_s((void*)header, LAMBDA_MAX_HEADER_LENGTH, msg.header.data(), msg.header.size());
        memcpy_s((void*)(header + msg.header_size), 1, &null_char, 1);
        memcpy_s((void*)message, LAMBDA_MAX_HEADER_LENGTH, msg.message.data(), msg.message.size());
        memcpy_s((void*)(message + msg.message_size), 1, &null_char, 1);
#endif
        
        return true;
      }
      return false;
    }
      
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API void clientSendMessage(unsigned int client, const char* header, const char* message)
    {
      get(client).sendMessage(Message(header, message));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API void clientUpdate(unsigned int client, double delta_time)
    {
      get(client).update(delta_time);
    }
      
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API void clientDeinitialize(unsigned int client)
    {
      get(client).deinitialize();
      g_clients.erase(client);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API bool clientIsConnected(unsigned int client)
    {
      return get(client).isConnected();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API void clientSetUpdateRate(unsigned int client, unsigned char hertz)
    {
      get(client).setUpdateRate(hertz);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API unsigned char clientGetUpdateRate(unsigned int client)
    {
      return get(client).getUpdateRate();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API void serverInitialize(uint16_t port, uint32_t host)
    {
      g_server.initialize(port, host);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API void serverUpdate(double delta_time)
    {
      g_server.update(delta_time);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API void serverDeinitialize()
    {
      g_server.deinitialize();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API void serverSetUpdateRate(unsigned int client, unsigned char hertz)
    {
      g_server.setUpdateRate(hertz);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LAMBDA_NETWORKING_API unsigned char serverGetUpdateRate(unsigned int client)
    {
      return g_server.getUpdateRate();
    }
  }
}
