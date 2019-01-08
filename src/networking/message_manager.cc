#include "message_manager.h"
#include <enet/enet.h>

namespace lambda
{
  namespace networking
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MessageManager::connect(ENetHost* server)
    {
      is_server_ = true;
      host_      = server;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MessageManager::connect(ENetHost* host, ENetPeer* peer)
    {
      is_server_ = false;
      host_      = host;
      client_    = peer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MessageManager::update(double delta_time)
    {
      elapsed_time_ += delta_time;
      
      if (elapsed_time_ >= frequency_)
      {
        elapsed_time_ -= frequency_;
        sendMessages();
        pollMessages();
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MessageManager::sendMessage(Message message)
    {
      packet_to_send_.addMessage(message);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool MessageManager::receiveMessages(Vector<MessagePacket>& packets)
    {
      if (true == packets_to_receive_.empty())
      {
        return false;
      }
      else
      {
        for (const auto& packet : packets_to_receive_)
        {
          packets.push_back(packet);
        }

        packets_to_receive_.clear();
        return true;
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MessageManager::setUpdateRate(uint8_t hertz)
    {
      update_rate_ = hertz;
      frequency_ = 1.0 / (double)update_rate_;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    uint8_t MessageManager::getUpdateRate() const
    {
      return update_rate_;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    double MessageManager::getFrequency() const
    {
      return frequency_;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MessageManager::pollMessages()
    {
      ENetEvent event;
      while (enet_host_service(host_, &event, 0) > 0)
      {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
          packets_to_receive_.push_back(MessagePacket::fromConnectedClient(event.peer));
          break;
        case ENET_EVENT_TYPE_RECEIVE:
          packets_to_receive_.push_back(MessagePacket(event.packet));
          break;
        case ENET_EVENT_TYPE_DISCONNECT:
          packets_to_receive_.push_back(MessagePacket::fromDisonnectedClient(event.peer));
          break;
        case ENET_EVENT_TYPE_NONE:
            break;
        }
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MessageManager::sendMessages()
    {
      if (packet_to_send_.messageCount() > 0)
      {
        ENetPacket* packet = packet_to_send_.asPacket();

        if (true == is_server_)
        {
          enet_host_broadcast(host_, 0, packet);
        }
        else
        {
          enet_peer_send(client_, 1, packet);
        }
      }
    }
  }
}
