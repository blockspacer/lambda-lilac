#include "message.h"
#include <enet/enet.h>
#include "messages.h"

namespace lambda
{
  namespace networking
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MessagePacket::MessagePacket()
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MessagePacket::MessagePacket(ENetPacket* packet)
    {
      read(packet);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    uint8_t MessagePacket::messageCount() const
    {
      return (uint8_t)messages_.size();
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Message MessagePacket::getMessage(uint8_t index) const
    {
      return messages_.at(index);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ENetPeer * MessagePacket::getClient() const
    {
      return client_;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MessagePacket::addMessage(Message message)
    {
      messages_.push_back(message);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MessagePacket::read(ENetPacket* packet)
    {
      clear();

      // Check if the packet was empty.
      if (packet->dataLength <= 0)
      {
        return;
      }
      
      // Get the packets data.
      String data(packet->dataLength, ' ');
      memcpy((void*)data.data(), packet->data, packet->dataLength);
      size_t data_length = packet->dataLength;
      
      // Release the packet.
      enet_packet_destroy(packet);

      // Initial offset since 0 is client_id;
      size_t offset = 0;

      // Read the messages.
      while (offset < data_length - 1)
      {
        Message message;
        // Get the client id.
        message.client_id = (uint8_t)(data.at(offset));
        offset++;

        // Get the header.
        message.header_size = (uint8_t)(data.at(offset));
        offset++;
        message.header.resize(message.header_size);
        memcpy((void*)message.header.data(), &data.at(offset), message.header_size);
        // Increase the offset.
        offset += message.header_size;

        // Get the message.
        message.message_size = (uint8_t)(data.at(offset));
        offset++;
        message.message.resize(message.message_size);
        memcpy((void*)message.message.data(), &data.at(offset), message.message_size);
        // Increase the offset.
        offset += message.message_size;

        // Add the message to the package.
        addMessage(message);
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ENetPacket* MessagePacket::asPacket()
    {
      String data;

      for (const auto& message : messages_)
      {
        data.append(message.asString());
      }

      clear();

      return enet_packet_create((void*)data.c_str(), data.size() + 1, ENET_PACKET_FLAG_RELIABLE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MessagePacket::clear()
    {
      // Clear the package.
      messages_.clear();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MessagePacket MessagePacket::fromConnectedClient(ENetPeer * client)
    {
      MessagePacket packet;
      packet.client_ = client;
      packet.addMessage(Message(MESSAGE_CONNECTED, ""));
      return packet;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MessagePacket MessagePacket::fromDisonnectedClient(ENetPeer * client)
    {
      MessagePacket packet;
      packet.client_ = client;
      packet.addMessage(Message(MESSAGE_DISCONNECTED, ""));
      return packet;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Message::Message() :
      header_size(0),
      header(""),
      message_size(0),
      message("")
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Message::Message(String header, String message) :
      header_size((uint8_t)strlen(header.c_str())),
      header(header),
      message_size((uint8_t)strlen(message.c_str())),
      message(message)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    String Message::asString() const
    {
      String ret;
      ret += (char)(client_id);
      ret += (char)(header_size) + header + (char)(message_size)+message;
      return ret;
    }
  }
}