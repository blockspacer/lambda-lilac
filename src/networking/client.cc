#include "client.h"
#include <enet/enet.h>
#include <utils/console.h>
#include "messages.h"

namespace lambda
{
  namespace networking
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Client::initialize(String name, uint16_t port, uint32_t host)
    {

      name_ = name;
      host_ = enet_host_create(
        nullptr   /* create a client host */,
        1         /* only allow 1 outgoing connection */,
        2         /* allow up 2 channels to be used, 0 and 1 */,
        57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
        14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */
      );

      if (host_ == nullptr)
        LMB_LOG_ERR("[CLIENT] An error occurred while trying to create an ENet host.");

      ENetAddress address;
      enet_address_set_host(&address, "localhost");
      address.port = port;

      /* Connect to localhost:1234. */
      /* Initiate the connection, allocating the two channels 0 and 1. */
      peer_ = enet_host_connect(host_, &address, 2, 0);

      if (peer_ == nullptr)
      {
        LMB_LOG_ERR("[CLIENT] An error occurred while trying to create an ENet peer.");
      }
             
      LMB_LOG_DEBG("[CLIENT] ENet client was created successfully.");
     
      message_manager_.connect(host_, peer_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Client::deinitialize()
    {
      enet_peer_reset(peer_);
      enet_host_destroy(host_);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Client::disconnect()
    {
      enet_peer_disconnect(peer_, 0);
      message_manager_.update(message_manager_.getFrequency());
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Client::update(double delta_time)
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
            if (message.client_id == id_)
              continue;

            if (message.header == MESSAGE_CONNECTED)
            {
              connected_ = true;
              LMB_LOG_DEBG("[CLIENT] Connected to server.");
            }
            else if (message.header == MESSAGE_DISCONNECTED)
            {
              connected_ = false;
              LMB_LOG_DEBG("[CLIENT] Disconnected from server.");
            }
            else if (message.header == MESSAGE_SET_ID)
              setId(message.message);
            else
              received_messages_.push(message);
          }
        }
      }
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Client::isConnected()
    {
      return connected_ && (id_ != 0);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Client::sendMessage(Message message)
    {
      message.client_id = id_;
      message_manager_.sendMessage(message);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Client::setUpdateRate(uint8_t hertz)
    {
      message_manager_.setUpdateRate(hertz);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    uint8_t Client::getUpdateRate()
    {
      return message_manager_.getUpdateRate();
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Client::pollMessages(Message& received_message)
    {
      if (received_messages_.empty())
        return false;
      else
      {
        received_message = received_messages_.front();
        received_messages_.pop();
        return true;
      }
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Client::setId(String message)
    {
      // Get the connect ID of the peer that the new ID will be assigned to.
      uint32_t connect_id;
      memcpy(&connect_id, message.data() + 1, sizeof(uint32_t));

      // Check if this is that specific peer.
      if (peer_->connectID == connect_id)
      {
        // Set the new ID.
        id_ = (uint8_t)message.at(0);

        // Send a change name message.
        sendMessage(lambda::networking::Message(MESSAGE_SET_NAME, name_));

        LMB_LOG_DEBG("[CLIENT] Got ID %i assigned from server.", id_);
      }
    }
  }
}