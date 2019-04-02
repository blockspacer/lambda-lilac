#pragma once
#include <glm/glm.hpp>
#include <containers/containers.h>
#include "utils/name.h"
#include "utils/console.h"

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    struct ShaderVariable
    {
      enum class Type : unsigned char
      {
        kFloat1,
        kFloat2,
        kFloat3,
        kFloat4,
        kFloat2x2,
        kFloat3x3,
        kFloat4x4,
      };

      ShaderVariable();
      ShaderVariable(const Name& name, const float& v);
      ShaderVariable(const Name& name, const glm::vec2& v);
      ShaderVariable(const Name& name, const glm::vec3& v);
      ShaderVariable(const Name& name, const glm::vec4& v);
      ShaderVariable(const Name& name, const glm::mat2x2& v);
      ShaderVariable(const Name& name, const glm::mat3x3& v);
      ShaderVariable(const Name& name, const glm::mat4x4& v);
      ShaderVariable(
        const Name& name, 
        const Vector<float>& v, 
        const Type& type
      );
	  void operator=(const ShaderVariable& other);
	  bool operator==(const ShaderVariable& other) const;

      size_t getSize() const;
      void update(const ShaderVariable& other);

      Name name;
      Type type;
      Vector<float> data;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline ShaderVariable::ShaderVariable()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    inline ShaderVariable::ShaderVariable(const Name& name, const float& v) 
      : name(name)
      , type(Type::kFloat1)
      , data({ v })
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    inline ShaderVariable::ShaderVariable(const Name& name, const glm::vec2& v)
      : name(name)
      , type(Type::kFloat2)
      , data({ v.x, v.y })
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    inline ShaderVariable::ShaderVariable(const Name& name, const glm::vec3& v)
      : name(name)
      , type(Type::kFloat3)
      , data({ v.x, v.y, v.z })
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    inline ShaderVariable::ShaderVariable(const Name& name, const glm::vec4& v) 
      : name(name)
      , type(Type::kFloat4)
      , data({ v.x, v.y, v.z, v.w })
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    inline ShaderVariable::ShaderVariable(
      const Name& name,
      const glm::mat2x2& v) 
      : name(name)
      , type(Type::kFloat2x2)
      , data({ v[0].x, v[0].y, v[1].x, v[1].y })
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    inline ShaderVariable::ShaderVariable(
      const Name& name, 
      const glm::mat3x3& v)
      : name(name)
      , type(Type::kFloat3x3)
      , data({ v[0].x, v[0].y, v[0].z, v[1].x, v[1].y,
               v[1].z, v[2].x, v[2].y, v[2].z })
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////
    inline ShaderVariable::ShaderVariable(
      const Name& name, 
      const glm::mat4x4& v)
      : name(name)
      , type(Type::kFloat4x4)
      , data({ v[0].x, v[0].y, v[0].z, v[0].w, v[1].x,
               v[1].y, v[1].z, v[1].w, v[2].x, v[2].y, 
               v[2].z, v[2].w, v[3].x, v[3].y, v[3].z, v[3].w })
    {
    }

		///////////////////////////////////////////////////////////////////////////
		inline ShaderVariable::ShaderVariable(
			const Name& name,
			const Vector<float>& v,
			const Type& type)
			: name(name)
			, type(type)
			, data(v)
		{
		}

		///////////////////////////////////////////////////////////////////////////
		inline void ShaderVariable::operator=(const ShaderVariable& other)
		{
			name = other.name;
			type = other.type;
			data = other.data;
		}

		///////////////////////////////////////////////////////////////////////////
		inline bool ShaderVariable::operator==(const ShaderVariable& other) const
		{
			return name == other.name && type == other.type;
		}
		
    ///////////////////////////////////////////////////////////////////////////
    inline size_t ShaderVariable::getSize() const
    {
      switch (type)
      {
      case Type::kFloat1:   return sizeof(float) * 1u;
      case Type::kFloat2:   return sizeof(float) * 2u;
      case Type::kFloat3:   return sizeof(float) * 3u;
      case Type::kFloat4:   return sizeof(float) * 4u;
      case Type::kFloat2x2: return sizeof(float) * 4u;
      case Type::kFloat3x3: return sizeof(float) * 9u;
      case Type::kFloat4x4: return sizeof(float) * 16u;
      default:              return 0u;
      }
    }
   
    ///////////////////////////////////////////////////////////////////////////
    inline void ShaderVariable::update(const ShaderVariable& other)
    {
      data = other.data;
    }
  }
}
