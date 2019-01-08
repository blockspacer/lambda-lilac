#pragma once
#include "message_manager.h"

struct _ENetPeer;
typedef _ENetPeer ENetPeer;
struct _ENetHost;
typedef _ENetHost ENetHost;

namespace lambda
{
  namespace networking
  {
    class MessagePacket;

    struct ClientData
    {
      String name = "";
    };

    class Server
    {
    public:
      // General functions.
      void initialize(uint16_t port = 1234u, uint32_t host = 0);
      void deinitialize();
      void update(double delta_time);
      uint8_t getUpdateRate() const;
      void setUpdateRate(uint8_t hertz);

      // Connection and disconnection of clients.
      void connect(ENetPeer* peer);
      void disconnect(ENetPeer* peer);

      // Messages.
      void setName(uint8_t id, String name);

    private:
      void handleMessage(const Message& message);
      ClientData& getClient(uint8_t id);

    private:
      MessageManager message_manager_;
      UnorderedMap<ENetPeer*, ClientData> clients_;
      uint8_t latest_open_client_id_ = 1;
      uint8_t connected_clients_     = 0;

      ENetHost* server_ = nullptr;
    };
  }
}