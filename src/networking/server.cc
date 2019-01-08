#include "server.h"
#include <enet/enet.h>
#include <utils/console.h>
#include "message.h"
#include "messages.h"
#include <iostream>

namespace lambda
{
  namespace networking
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Server::initialize(uint16_t port, uint32_t host)
    {
      ENetAddress address;
      /* Bind the server to the default localhost.     */
      /* A specific host address can be specified by   */
      /* enet_address_set_host (& address, "x.x.x.x"); */
      address.port = port;
      enet_address_set_host(&address, "localhost");

      server_ = enet_host_create(
        &address /* the address to bind the server host to */,
        32       /* allow up to 32 clients and/or outgoing connections */,
        2        /* allow up to 2 channels to be used, 0 and 1 */,
        0        /* assume any amount of incoming bandwidth */,
        0        /* assume any amount of outgoing bandwidth */
      );

      if (server_ == nullptr)
      {
        LMB_LOG_ERR("[SERVER] Failed to create an ENet server.");
      }
      else
      {
        LMB_LOG_DEBG("[SERVER] ENet server was created successfully.");
      }
      message_manager_.connect(server_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Server::deinitialize()
    {
      enet_host_destroy(server_);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Server::update(double delta_time)
    {
      message_manager_.update(delta_time);

      Vector<MessagePacket> packets;
      if (true == message_manager_.receiveMessages(packets))
      {
        for (const auto& packet : packets)
        {
          for (int i = 0; i < packet.messageCount(); ++i)
          {
            const Message& message = packet.getMessage(i);

            if (message.header == MESSAGE_CONNECTED)
            {
              connect(packet.getClient());
            }
            else if (message.header == MESSAGE_DISCONNECTED)
            {
              disconnect(packet.getClient());
            }
            else if (message.header == MESSAGE_SET_NAME)
            {
              setName(message.client_id, message.message);
            }
            else
            {
              handleMessage(message);
            }
          }
        }
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    uint8_t Server::getUpdateRate() const
    {
      return message_manager_.getUpdateRate();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Server::setUpdateRate(uint8_t hertz)
    {
      message_manager_.setUpdateRate(hertz);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Server::connect(ENetPeer* peer)
    {
      peer->data = malloc(1);
      memcpy(peer->data, &latest_open_client_id_, 1);

      clients_.insert(eastl::make_pair(peer, ClientData()));

      Message message(MESSAGE_SET_ID, String(sizeof(uint8_t) + sizeof(uint32_t), ' '));
      message.message.at(0) = (char)latest_open_client_id_;
      memcpy((void*)(message.message.data() + 1), &peer->connectID, sizeof(uint32_t));
      message_manager_.sendMessage(message);

      latest_open_client_id_++;
      connected_clients_++;
      LMB_LOG_DEBG("[SERVER] Successfully connected to a client");
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Server::disconnect(ENetPeer* peer)
    {
      uint8_t id;
      memcpy(&id, peer->data, 1);
      free(peer->data);
      clients_.erase(peer);
      connected_clients_--;

      LMB_LOG_DEBG("[SERVER] Successfully disconnected client %i", id);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Server::setName(uint8_t id, String name)
    {
      getClient(id).name = name;
      LMB_LOG_DEBG("[SERVER] Client %i: Name is set to %s.", id, name.c_str());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Server::handleMessage(const Message& message)
    {
      message_manager_.sendMessage(message);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ClientData& Server::getClient(uint8_t id)
    {
      for (auto& it : clients_)
      {
        uint8_t client_id = *(uint8_t*)it.first->data;
        if (client_id == id)
        {
          return it.second;
        }
      }
      LMB_LOG_ERR("[SERVER] Could not find client with an ID of %i!", id);
      
      // To please the compiler. It will never get here.
      static ClientData backup_data;
      return backup_data;
    }
  }
}