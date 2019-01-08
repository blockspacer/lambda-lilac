#include "zone_manager.h"
#include "platform/frustum.h"
#include <algorithm>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include "../platform/debug_renderer.h"

namespace lambda
{
  namespace utilities
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Token::Token() :
      id_(0u), user_data_(nullptr)
    {
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Token::Token(size_t id, void* user_data) :
      id_(id), user_data_(user_data)
    {
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    size_t Token::getId() const
    {
      return id_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void* Token::getUserData() const
    {
      return user_data_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Token::operator==(const size_t& rhs) const
    {
      return id_ == rhs;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Token::operator==(const Token& rhs) const
    {
      return id_ == rhs.id_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Token::operator<(const size_t& rhs) const
    {
      return id_ < rhs;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Token::operator<(const Token&  rhs) const
    {
      return id_ < rhs.id_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Zone::addToken(Token token)
    {
      tokens_.push_back(token);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Zone::removeToken(Token token)
    {
      auto it = std::find(tokens_.begin(), tokens_.end(), token);
      if(it != tokens_.end())
      {
        tokens_.erase(it);
      }
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const Vector<Token>& Zone::getTokens() const
    {
      return tokens_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ZoneManager::ZoneManager() :
      zone_size_(25.0f),
      dummy_zone_(glm::vec2(0.0f, 0.0f))
    {
      half_zone_size_ = zone_size_ * 0.5f;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void ZoneManager::addToken(glm::vec2 min, glm::vec2 max, Token token)
    {
      min = glm::floor(min / zone_size_);
      max = glm::ceil(max / zone_size_);
      
      for (int16_t y = (int16_t)min.y; y < (int16_t)max.y; ++y)
      {
        for (int16_t x = (int16_t)min.x; x < (int16_t)max.x; ++x)
        {
          getZone(x, y).addToken(token);
        }
      }
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void ZoneManager::removeToken(Token token)
    {
      for (auto& zone : zones_)
      {
        zone.removeToken(token);
      }
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Vector<Token> ZoneManager::getTokens(glm::vec2 min, glm::vec2 max) const
    {
      Vector<Token> tokens;

      min = glm::floor(min / zone_size_);
      max = glm::ceil(max / zone_size_);

      for (int16_t y = (int16_t)min.y; y < (int16_t)max.y; ++y)
      {
        for (int16_t x = (int16_t)min.x; x < (int16_t)max.x; ++x)
        {
          const Vector<Token>& t = getZone(x, y).getTokens();
          tokens.insert(tokens.end(), t.begin(), t.end());
        }
      }

      std::sort(tokens.begin(), tokens.end());
      tokens.erase(std::unique(tokens.begin(), tokens.end()), tokens.end());

      return tokens;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Vector<Token> ZoneManager::getTokens(const utilities::Frustum& frustum) const
    {
      const glm::vec2 min = glm::floor(glm::vec2(frustum.getMin().x, frustum.getMin().z) / zone_size_);
      const glm::vec2 max = glm::ceil(glm::vec2(frustum.getMax().x, frustum.getMax().z) / zone_size_);

      Vector<Token> tokens;
      for (int16_t y = (int16_t)min.y; y < (int16_t)max.y; ++y)
      {
        for (int16_t x = (int16_t)min.x; x < (int16_t)max.x; ++x)
        {
          const Zone& zone = getZone(x, y);
          const glm::vec2 center = zone.getCenter();

          const glm::vec3 min = glm::vec3(center.x - half_zone_size_.x, -1000000.0f, center.y - half_zone_size_.y);
          const glm::vec3 max = glm::vec3(center.x + half_zone_size_.x, +1000000.0f, center.y + half_zone_size_.y);
          if (frustum.ContainsAABB(min, max))
          {
            /*glm::vec3 c = glm::vec3(zone.getCenter().x, 0.0f, zone.getCenter().y);
            glm::vec3 tl = c + glm::vec3(-0.5f * zone_size_.x, 0.0f, +0.5f * zone_size_.y);
            glm::vec3 tr = c + glm::vec3(+0.5f * zone_size_.x, 0.0f, +0.5f * zone_size_.y);
            glm::vec3 bl = c + glm::vec3(-0.5f * zone_size_.x, 0.0f, -0.5f * zone_size_.y);
            glm::vec3 br = c + glm::vec3(+0.5f * zone_size_.x, 0.0f, -0.5f * zone_size_.y);
            debug_renderer.DrawLine(platform::DebugLine(tl, br, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)));
            debug_renderer.DrawLine(platform::DebugLine(bl, tr, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)));
            debug_renderer.DrawLine(platform::DebugLine(bl, br, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)));
            debug_renderer.DrawLine(platform::DebugLine(br, tr, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)));
            debug_renderer.DrawLine(platform::DebugLine(tr, tl, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)));
            debug_renderer.DrawLine(platform::DebugLine(tl, bl, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)));*/

            const Vector<Token>& t = zone.getTokens();
            tokens.insert(tokens.end(), t.begin(), t.end());
          }
        }
      }

      std::sort(tokens.begin(), tokens.end());
      tokens.erase(std::unique(tokens.begin(), tokens.end()), tokens.end());

      return tokens;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Zone& ZoneManager::getZone(int16_t x, int16_t y)
    {
      uint32_t hash = this->hash(x, y);
      auto it = hash_to_zone_.find(hash);
      if (it == hash_to_zone_.end())
      {
        glm::vec2 center(((float)x + 0.5f) * zone_size_.x, ((float)y + 0.5f) * zone_size_.y);

        hash_to_zone_.insert(eastl::make_pair(hash, (uint32_t)zones_.size()));
        zones_.push_back(Zone(center));
        it = hash_to_zone_.find(hash);
      }

      return zones_.at(it->second);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const Zone& ZoneManager::getZone(int16_t x, int16_t y) const
    {
      uint32_t hash = this->hash(x, y);
      auto it = hash_to_zone_.find(hash);
      if (it != hash_to_zone_.end())
      {
        return zones_.at(it->second);
      }

      return dummy_zone_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    uint32_t ZoneManager::hash(int16_t x, int16_t y) const
    {
      uint32_t hash = 0u;
      memcpy(&hash, &x, sizeof(int16_t));
      memcpy((char*)&hash + sizeof(int16_t), &y, sizeof(int16_t));
      return hash;
    }
  }
}