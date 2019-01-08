#pragma once
#include "message_manager.h"
#include <containers/containers.h>

struct _ENetPeer;
typedef _ENetPeer ENetPeer;
struct _ENetHost;
typedef _ENetHost ENetHost;

namespace lambda
{
  namespace networking
  {
    class Client
    {
    public:
      // General functions.
      void initialize(String name, uint16_t port = 1234u, uint32_t host = 0);
      void deinitialize();
      void disconnect();
      void update(double delta_time);
      bool isConnected();
      void sendMessage(Message message);
      void setUpdateRate(uint8_t hertz);
      uint8_t getUpdateRate();
      bool pollMessages(Message& received_message);

    private:
      void setId(String message);

    private:
      String name_ = "";
      MessageManager message_manager_;
      ENetHost* host_ = nullptr;
      ENetPeer* peer_ = nullptr;
      bool connected_ = false;
      uint8_t id_ = 0;

      Queue<Message> received_messages_;
    };
  }
}