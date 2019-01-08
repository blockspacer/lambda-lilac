#pragma once
#include <containers/containers.h>
#include "script_vector.h"

namespace lambda
{
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    class ScriptValue
    {
    public:
      /////////////////////////////////////////////////////////////////////////
      enum Type
      {
        kBoolean,
        kInt8,
        kUint8,
        kInt16,
        kUint16,
        kInt32,
        kUint32,
        kInt64,
        kUint64,
        kFloat,
        kDouble,
        kString,
        kVec2,
        kVec3,
        kVec4,
        kNull,
      };

      ScriptValue()           {                     type_ = kNull;    }
      ScriptValue(bool v)     { data_.boolean  = v; type_ = kBoolean; }
      ScriptValue(int8_t v)   { data_.int8     = v; type_ = kInt8;    }
      ScriptValue(uint8_t v)  { data_.uint8    = v; type_ = kUint8;   }
      ScriptValue(int16_t v)  { data_.int16    = v; type_ = kInt16;   }
      ScriptValue(uint16_t v) { data_.uint16   = v; type_ = kUint16;  }
      ScriptValue(int32_t v)  { data_.int32    = v; type_ = kInt32;   }
      ScriptValue(uint32_t v) { data_.uint32   = v; type_ = kUint32;  }
      ScriptValue(int64_t v)  { data_.int64    = v; type_ = kInt64;   }
      ScriptValue(uint64_t v) { data_.uint64   = v; type_ = kUint64;  }
      ScriptValue(float v)    { data_.s_float  = v; type_ = kFloat;   }
      ScriptValue(double v)   { data_.s_double = v; type_ = kDouble;  }
      ScriptValue(String v)   { string_        = v; type_ = kString;  }
      ScriptValue(ScriptVec2 v) { vec2_        = v; type_ = kVec2;    }
      ScriptValue(ScriptVec3 v) { vec3_        = v; type_ = kVec3;    }
      ScriptValue(ScriptVec4 v) { vec4_        = v; type_ = kVec4;    }

      Type     getType()   const { return type_;          }
      bool     getBool()   const { return data_.boolean;  }
      int8_t   getInt8()   const { return data_.int8;     }
      uint8_t  getUint8()  const { return data_.uint8;    }
      int16_t  getInt16()  const { return data_.int16;    }
      uint16_t getUint16() const { return data_.uint16;   }
      int32_t  getInt32()  const { return data_.int32;    }
      int64_t  getInt64()  const { return data_.int64;    }
      uint32_t getUint32() const { return data_.uint32;   }
      uint64_t getUint64() const { return data_.uint64;   }
      float    getFloat()  const { return data_.s_float;  }
      double   getDouble() const { return data_.s_double; }
      String   getString() const { return string_;        }
      ScriptVec2 getVec2() const { return vec2_;          }
      ScriptVec3 getVec3() const { return vec3_;          }
      ScriptVec4 getVec4() const { return vec4_;          }

    private:
      /////////////////////////////////////////////////////////////////////////
      union Data
      {
        bool boolean = 0;
        int8_t int8;
        int16_t int16;
        int32_t int32;
        int64_t int64;
        uint8_t uint8;
        uint16_t uint16;
        uint32_t uint32;
        uint64_t uint64;
        float s_float;
        double s_double;
      } data_;
      ScriptVec2 vec2_;
      ScriptVec3 vec3_;
      ScriptVec4 vec4_;
      String string_ = "";
      Type type_;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct ScriptArray
    {
      Vector<bool>       vec_bool;
      Vector<int8_t>     vec_int8;
      Vector<int16_t>    vec_int16;
      Vector<int32_t>    vec_int32;
      Vector<int64_t>    vec_int64;
      Vector<uint8_t>    vec_uint8;
      Vector<uint16_t>   vec_uint16;
      Vector<uint32_t>   vec_uint32;
      Vector<uint64_t>   vec_uint64;
      Vector<float>      vec_float;
      Vector<double>     vec_double;
      Vector<ScriptVec2> vec_vec2;
      Vector<ScriptVec3> vec_vec3;
      Vector<ScriptVec4> vec_vec4;
      Vector<String>     vec_string;
    };
  }
}