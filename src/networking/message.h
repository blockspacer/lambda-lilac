#pragma once
#include <containers/containers.h>

struct _ENetPacket;
typedef _ENetPacket ENetPacket;

struct _ENetPeer;
typedef _ENetPeer ENetPeer;

namespace lambda
{
  namespace networking
  {
    class Message
    {
    public:
      Message();
      Message(String header, String message);
      String asString() const;

      // Header.
      uint8_t client_id;
      uint8_t header_size;
      String  header;
      // Message.
      uint8_t message_size;
      String  message;
    };

    class MessagePacket
    {
    public:
      // Constructors.
      MessagePacket();
      MessagePacket(ENetPacket* packet);
      // Getters.
      uint8_t messageCount() const;
      Message getMessage(uint8_t index) const;
      ENetPeer* getClient() const;

      // Setters.
      void addMessage(Message message);
      
      // Misc.
      void read(ENetPacket* packet);
      ENetPacket* asPacket();
      void clear();
      static MessagePacket fromConnectedClient(ENetPeer* client);
      static MessagePacket fromDisonnectedClient(ENetPeer* client);

    private:
      // Client ID 0 means server.
      Vector<Message> messages_;
      ENetPeer*       client_ = 0; // For connecting and disconnecting.
    };
  }
}