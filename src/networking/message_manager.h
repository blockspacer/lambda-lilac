#pragma once
#include "message.h"
#include <limits>

struct _ENetPeer;
typedef _ENetPeer ENetPeer;
struct _ENetHost;
typedef _ENetHost ENetHost;

namespace lambda
{
  namespace networking
  {
    class MessageManager
    {
    public:
      void connect(ENetHost* server);
      void connect(ENetHost* host, ENetPeer* peer);

      void update(double delta_time);

      void sendMessage(Message message);
      bool receiveMessages(Vector<MessagePacket>& packet);
      void setUpdateRate(uint8_t hertz);
      uint8_t getUpdateRate() const;
      double getFrequency() const;

    private:
      void pollMessages();
      void sendMessages();

    private:
      uint8_t update_rate_ = 60;
      double frequency_    = 0.01666667;
      double elapsed_time_ = std::numeric_limits<double>::max();
      bool is_server_      = true;
      ENetHost* host_      = nullptr;
      ENetPeer* client_    = nullptr;
      MessagePacket packet_to_send_;
      Vector<MessagePacket> packets_to_receive_;
    };
  }
}