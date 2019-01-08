#pragma once
#include <containers/containers.h>
#include <glm/vec2.hpp>

namespace lambda
{
  namespace platform
  {
    class DebugRenderer;
  }
  namespace utilities
  {
    class Frustum;
    class Token
    {
    public:
      Token();
      Token(size_t id, void* user_data);

      size_t getId() const;
      void* getUserData() const;

      bool operator==(const size_t& rhs) const;
      bool operator==(const Token&  rhs) const;
      bool operator<(const size_t& rhs) const;
      bool operator<(const Token&  rhs) const;

    private:
      size_t id_;
      void* user_data_;
    };

    class Zone
    {
    public:
      Zone(glm::vec2 center) : center_(center) {}
      void addToken(Token token);
      void removeToken(Token token);
      const Vector<Token>& getTokens() const;
      glm::vec2 getCenter() const { return center_; }

    private:
      glm::vec2 center_;
      Vector<Token> tokens_;
    };

    class ZoneManager
    {
    public:
      ZoneManager();
      void addToken(glm::vec2 min, glm::vec2 max, Token token);
      void removeToken(Token token);
      Vector<Token> getTokens(glm::vec2 min, glm::vec2 max) const;
      Vector<Token> getTokens(const utilities::Frustum& frustum) const;

    private:
      Zone& getZone(int16_t x, int16_t y);
      const Zone& getZone(int16_t x, int16_t y) const;
      uint32_t hash(int16_t x, int16_t y) const;

    private:
      glm::vec2 zone_size_;
      glm::vec2 half_zone_size_;
      Map<uint32_t, uint32_t> hash_to_zone_;
      Vector<Zone> zones_;
      Zone dummy_zone_;
    };
  }
}