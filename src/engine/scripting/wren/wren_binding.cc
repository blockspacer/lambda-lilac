#include "scripting/wren/wren_binding.h"
#include <assets/texture.h>
#include <assets/mesh.h>
#include <assets/mesh_io.h>
#include <assets/shader.h>
#include <assets/shader_io.h>
#include <systems/transform_system.h>
#include <systems/entity_system.h>
#include <systems/transform_system.h>
#include <systems/camera_system.h>
#include <systems/light_system.h>
#include <systems/mesh_render_system.h>
#include <systems/lod_system.h>
#include <systems/rigid_body_system.h>
#include <systems/wave_source_system.h>
#include <systems/collider_system.h>
#include <systems/mono_behaviour_system.h>
#include <platform/post_process_manager.h>
#include <interfaces/iworld.h>

#include <memory/memory.h>
#include <containers/containers.h>
#include <utils/file_system.h>
#include <utils/console.h>
#include <utils/utilities.h>

#include <wren.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <FastNoise.h>

#include <algorithm>

namespace lambda
{
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    world::IWorld* g_world;
    entity::EntitySystem* g_entitySystem;
    components::TransformSystem* g_transformSystem;
    components::CameraSystem* g_cameraSystem;
    components::LightSystem* g_lightSystem;
    components::MeshRenderSystem* g_meshRenderSystem;
    components::LODSystem* g_lodSystem;
    components::RigidBodySystem* g_rigidBodySystem;
    components::WaveSourceSystem* g_waveSourceSystem;
    components::ColliderSystem* g_colliderSystem;
    components::MonoBehaviourSystem* g_monoBehaviourSystem;

    ///////////////////////////////////////////////////////////////////////////
    template<typename T>
    T* GetForeign(WrenVM* vm, int slot = 0)
    {
      return (T*)wrenGetSlotForeign(vm, slot);
    }

    ///////////////////////////////////////////////////////////////////////////
    template<typename T>
    T* MakeForeign(WrenVM* vm, int slot = 0, int class_slot = 0)
    {
      return (T*)wrenSetSlotNewForeign(vm, slot, class_slot, sizeof(T));
    }

    namespace Console
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// console.wren ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Console {
    foreign static debug(string)
    foreign static error(string)
    foreign static info(string)
    foreign static warning(string)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "debug(_)") == 0) return [](WrenVM* vm) {
          LMB_LOG_DEBG("%s\n", wrenGetSlotString(vm, 1));
        };
        if (strcmp(signature, "error(_)") == 0) return [](WrenVM* vm) {
          LMB_LOG_ERR("%s\n", wrenGetSlotString(vm, 1));
        };
        if (strcmp(signature, "info(_)") == 0) return [](WrenVM* vm) {
          LMB_LOG_INFO("%s\n", wrenGetSlotString(vm, 1));
        };
        if (strcmp(signature, "warning(_)") == 0) return [](WrenVM* vm) {
          LMB_LOG_WARN("%s\n", wrenGetSlotString(vm, 1));
        };
        return nullptr;
      }
    }
    namespace Vec2
    {

      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// vec2.wren ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Vec2 {
    construct new() {
    }
    construct new(v) {
    }
    construct new(x, y) {
    }
    foreign normalize()
    foreign dot(v)

    +(v) {
        if (v is Num)  return Vec2.new(x + v, y + v)
        if (v is Vec2) return Vec2.new(x + v.x, y + v.y)
    }
    -(v) {
        if (v is Num)  return Vec2.new(x - v, y - v)
        if (v is Vec2) return Vec2.new(x - v.x, y - v.y)
    }
    *(v) {
        if (v is Num)  return Vec2.new(x * v, y * v)
        if (v is Vec2) return Vec2.new(x * v.x, y * v.y)
    }
    /(v) {
        if (v is Num)  return Vec2.new(x / v, y / v)
        if (v is Vec2) return Vec2.new(x / v.x, y / v.y)
    }

    foreign normalized
    foreign toString

    foreign x
    foreign y
    foreign magnitude
    foreign magnitudeSqr
    foreign length
    foreign lengthSqr
    foreign x=(x)
    foreign y=(y)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenHandle* handle = nullptr;

      /////////////////////////////////////////////////////////////////////////
      glm::vec2* make(WrenVM* vm, const glm::vec2 val = glm::vec2())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Vec2", "Vec2", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        glm::vec2* data = MakeForeign<glm::vec2>(vm, 0, 1);
        memcpy(data, &val, sizeof(glm::vec2));
        return data;
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          int arg_count = wrenGetSlotCount(vm);
          if (arg_count == 2)
            make(vm, glm::vec2((float)wrenGetSlotDouble(vm, 1)));
          else if (arg_count == 3)
            make(vm, glm::vec2((float)wrenGetSlotDouble(vm, 1), (float)wrenGetSlotDouble(vm, 2)));
        },
          [](void* data) {}
        };
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "x") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::vec2>(vm)).x);
        };
        if (strcmp(signature, "y") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::vec2>(vm)).y);
        };
        if (strcmp(signature, "x=(_)") == 0) return [](WrenVM* vm) {
          (*GetForeign<glm::vec2>(vm)).x = (float)wrenGetSlotDouble(vm, 1);
        };
        if (strcmp(signature, "y=(_)") == 0) return [](WrenVM* vm) {
          (*GetForeign<glm::vec2>(vm)).y = (float)wrenGetSlotDouble(vm, 1);
        };
        if (strcmp(signature, "magnitude") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)glm::length(*GetForeign<glm::vec2>(vm)));
        };
        if (strcmp(signature, "length") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)glm::length(*GetForeign<glm::vec2>(vm)));
        };
        if (strcmp(signature, "magnitudeSqr") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)glm::length2(*GetForeign<glm::vec2>(vm)));
        };
        if (strcmp(signature, "lengthSqr") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)glm::length2(*GetForeign<glm::vec2>(vm)));
        };
        if (strcmp(signature, "normalize()") == 0) return [](WrenVM* vm) {
          glm::vec2& vec = *GetForeign<glm::vec2>(vm);
          vec = glm::normalize(vec);
        };
        if (strcmp(signature, "normalized") == 0) return [](WrenVM* vm) {
          make(vm, glm::normalize(*GetForeign<glm::vec2>(vm)));
        };
        if (strcmp(signature, "dot(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)glm::dot(*GetForeign<glm::vec2>(vm, 0), *GetForeign<glm::vec2>(vm, 1)));
        };
        if (strcmp(signature, "toString") == 0) return [](WrenVM* vm) {
          glm::vec2& vec = *GetForeign<glm::vec2>(vm);
          String str = "[" + lambda::toString(vec.x) + ", " + lambda::toString(vec.y) + "]";
          const char* c_str = (const char*)foundation::Memory::allocate(str.size() + 1u);
          memcpy((void*)c_str, str.data(), str.size() + 1u);
          wrenSetSlotString(vm, 0, c_str);
        };
        return nullptr;
      }
    }
    namespace Vec3
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// vec3.wren ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Vec3 {
    construct new() {
    }
    construct new(v) {
    }
    construct new(x, y, z) {
    }
    foreign normalize()
    foreign dot(v)
    foreign cross(v)

    +(v) {
        if (v is Num)  return Vec3.new(x + v, y + v, z + v)
        if (v is Vec3) return Vec3.new(x + v.x, y + v.y, z + v.z)
    }
    -(v) {
        if (v is Num)  return Vec3.new(x - v, y - v, z - v)
        if (v is Vec3) return Vec3.new(x - v.x, y - v.y, z - v.z)
    }
    *(v) {
        if (v is Num)  return Vec3.new(x * v, y * v, z * v)
        if (v is Vec3) return Vec3.new(x * v.x, y * v.y, z * v.z)
    }
    /(v) {
        if (v is Num)  return Vec3.new(x / v, y / v, z / v)
        if (v is Vec3) return Vec3.new(x / v.x, y / v.y, z / v.z)
    }

    foreign normalized
    foreign toString

    foreign x
    foreign y
    foreign z
    foreign magnitude
    foreign magnitudeSqr
    foreign length
    foreign lengthSqr
    foreign x=(x)
    foreign y=(y)
    foreign z=(z)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenHandle* handle = nullptr;
      
      /////////////////////////////////////////////////////////////////////////
      glm::vec3* make(WrenVM* vm, const glm::vec3 val = glm::vec3())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Vec3", "Vec3", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        glm::vec3* data = MakeForeign<glm::vec3>(vm, 0, 1);
        memcpy(data, &val, sizeof(glm::vec3));
        return data;
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          int arg_count = wrenGetSlotCount(vm);
          if (arg_count == 2)
            make(vm, glm::vec3((float)wrenGetSlotDouble(vm, 1)));
          else if (arg_count == 4)
            make(vm, glm::vec3(
              (float)wrenGetSlotDouble(vm, 1), 
              (float)wrenGetSlotDouble(vm, 2), 
              (float)wrenGetSlotDouble(vm, 3))
            );
        },
          [](void* data) {}
        };
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "x") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::vec3>(vm)).x);
        };
        if (strcmp(signature, "y") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::vec3>(vm)).y);
        };
        if (strcmp(signature, "z") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::vec3>(vm)).z);
        };
        if (strcmp(signature, "x=(_)") == 0) return [](WrenVM* vm) {
          (*GetForeign<glm::vec3>(vm)).x = (float)wrenGetSlotDouble(vm, 1);
        };
        if (strcmp(signature, "y=(_)") == 0) return [](WrenVM* vm) {
          (*GetForeign<glm::vec3>(vm)).y = (float)wrenGetSlotDouble(vm, 1);
        };
        if (strcmp(signature, "z=(_)") == 0) return [](WrenVM* vm) {
          (*GetForeign<glm::vec3>(vm)).z = (float)wrenGetSlotDouble(vm, 1);
        };
        if (strcmp(signature, "magnitude") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length(*GetForeign<glm::vec3>(vm))
          );
        };
        if (strcmp(signature, "length") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length(*GetForeign<glm::vec3>(vm))
          );
        };
        if (strcmp(signature, "magnitudeSqr") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length2(*GetForeign<glm::vec3>(vm))
          );
        };
        if (strcmp(signature, "lengthSqr") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length2(*GetForeign<glm::vec3>(vm))
          );
        };
        if (strcmp(signature, "normalize()") == 0) return [](WrenVM* vm) {
          glm::vec3& vec = *GetForeign<glm::vec3>(vm);
          vec = glm::normalize(vec);
        };
        if (strcmp(signature, "normalized") == 0) return [](WrenVM* vm) {
          make(vm, glm::normalize(*GetForeign<glm::vec3>(vm)));
        };
        if (strcmp(signature, "cross(_)") == 0) return [](WrenVM* vm) {
          glm::vec3& lhs = *GetForeign<glm::vec3>(vm, 0);
          glm::vec3& rhs = *GetForeign<glm::vec3>(vm, 1);
          float x = lhs[1u] * rhs[2u] - lhs[2u] * rhs[1u];
          float y = lhs[2u] * rhs[0u] - lhs[0u] * rhs[2u];
          float z = lhs[0u] * rhs[1u] - lhs[1u] * rhs[0u];
          lhs[0u] = x;
          lhs[1u] = y;
          lhs[2u] = z;
        };
        if (strcmp(signature, "dot(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::dot(
              *GetForeign<glm::vec3>(vm, 0), 
              *GetForeign<glm::vec3>(vm, 1)
            )
          );
        };
        if (strcmp(signature, "toString") == 0) return [](WrenVM* vm) {
          glm::vec3& vec = *GetForeign<glm::vec3>(vm);
          String str = "[" + lambda::toString(vec.x) + ", " + 
            lambda::toString(vec.y) + ", " + lambda::toString(vec.z) + "]";
          const char* c_str = 
            (const char*)foundation::Memory::allocate(str.size() + 1u);
          memcpy((void*)c_str, str.data(), str.size() + 1u);
          wrenSetSlotString(vm, 0, c_str);
        };
        return nullptr;
      }
    }
    namespace Vec4
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// vec4.wren ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Vec4 {
    construct new() {
    }
    construct new(v) {
    }
    construct new(x, y, z, w) {
    }
    foreign normalize()
    foreign dot(v)

    +(v) {
        if (v is Num)  return Vec4.new(x + v, y + v, z + v, w + v)
        if (v is Vec4) return Vec4.new(x + v.x, y + v.y, z + v.z, w + v.w)
    }
    -(v) {
        if (v is Num)  return Vec4.new(x - v, y - v, z - v, w - v)
        if (v is Vec4) return Vec4.new(x - v.x, y - v.y, z - v.z, w - v.w)
    }
    *(v) {
        if (v is Num)  return Vec4.new(x * v, y * v, z * v, w * v)
        if (v is Vec4) return Vec4.new(x * v.x, y * v.y, z * v.z, w * v.w)
    }
    /(v) {
        if (v is Num)  return Vec4.new(x / v, y / v, z / v, w / v)
        if (v is Vec4) return Vec4.new(x / v.x, y / v.y, z / v.z, w / v.w)
    }

    foreign normalized
    foreign toString
    
    foreign x
    foreign y
    foreign z
    foreign w
    foreign magnitude
    foreign magnitudeSqr
    foreign length
    foreign lengthSqr
    foreign x=(x)
    foreign y=(y)
    foreign z=(z)
    foreign w=(w)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenHandle* handle = nullptr;
      glm::vec4* make(WrenVM* vm, const glm::vec4 val = glm::vec4())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Vec4", "Vec4", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        glm::vec4* data = MakeForeign<glm::vec4>(vm, 0, 1);
        memcpy(data, &val, sizeof(glm::vec4));
        return data;
      }
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          int arg_count = wrenGetSlotCount(vm);
          if (arg_count == 2)
            make(vm, glm::vec4((float)wrenGetSlotDouble(vm, 1)));
          else if (arg_count == 5)
            make(vm, glm::vec4(
              (float)wrenGetSlotDouble(vm, 1), 
              (float)wrenGetSlotDouble(vm, 2), 
              (float)wrenGetSlotDouble(vm, 3), 
              (float)wrenGetSlotDouble(vm, 4)
            ));
        },
          [](void* data) {}
        };
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "x") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::vec4>(vm)).x);
        };
        if (strcmp(signature, "y") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::vec4>(vm)).y);
        };
        if (strcmp(signature, "z") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::vec4>(vm)).z);
        };
        if (strcmp(signature, "w") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::vec4>(vm)).w);
        };
        if (strcmp(signature, "x=(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(wrenGetSlotType(vm, 1) == WREN_TYPE_NUM && 
            wrenGetSlotType(vm, 2) == WREN_TYPE_NUM, "Invalid call");
          (*GetForeign<glm::vec4>(vm)).x = (float)wrenGetSlotDouble(vm, 2);
        };
        if (strcmp(signature, "y=(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(wrenGetSlotType(vm, 1) == WREN_TYPE_NUM && 
            wrenGetSlotType(vm, 2) == WREN_TYPE_NUM, "Invalid call");
          (*GetForeign<glm::vec4>(vm)).y = (float)wrenGetSlotDouble(vm, 2);
        };
        if (strcmp(signature, "z=(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(wrenGetSlotType(vm, 1) == WREN_TYPE_NUM && 
            wrenGetSlotType(vm, 2) == WREN_TYPE_NUM, "Invalid call");
          (*GetForeign<glm::vec4>(vm)).z = (float)wrenGetSlotDouble(vm, 2);
        };
        if (strcmp(signature, "w=(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(wrenGetSlotType(vm, 1) == WREN_TYPE_NUM && 
            wrenGetSlotType(vm, 2) == WREN_TYPE_NUM, "Invalid call");
          (*GetForeign<glm::vec4>(vm)).w = (float)wrenGetSlotDouble(vm, 2);
        };
        if (strcmp(signature, "magnitude") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length(*GetForeign<glm::vec4>(vm))
          );
        };
        if (strcmp(signature, "length") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length(*GetForeign<glm::vec4>(vm))
          );
        };
        if (strcmp(signature, "magnitudeSqr") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length2(*GetForeign<glm::vec4>(vm))
          );
        };
        if (strcmp(signature, "lengthSqr") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length2(*GetForeign<glm::vec4>(vm))
          );
        };
        if (strcmp(signature, "normalize()") == 0) return [](WrenVM* vm) {
          glm::vec4& vec = *GetForeign<glm::vec4>(vm);
          vec = glm::normalize(vec);
        };
        if (strcmp(signature, "normalized") == 0) return [](WrenVM* vm) {
          make(vm, glm::normalize(*GetForeign<glm::vec4>(vm)));
        };
        if (strcmp(signature, "dot(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::dot(
              *GetForeign<glm::vec4>(vm, 0), 
              *GetForeign<glm::vec4>(vm, 1)
            )
          );
        };
        if (strcmp(signature, "toString") == 0) return [](WrenVM* vm) {
          glm::vec4& vec = *GetForeign<glm::vec4>(vm);
          String str = "[" + lambda::toString(vec.x) + ", " + 
            lambda::toString(vec.y) + ", " + lambda::toString(vec.z) + ", " + 
            lambda::toString(vec.w) + "]";
          const char* c_str = 
            (const char*)foundation::Memory::allocate(str.size() + 1u);
          memcpy((void*)c_str, str.data(), str.size() + 1u);
          wrenSetSlotString(vm, 0, c_str);
        };
        return nullptr;
      }
    }
    namespace Quat
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// quat.wren ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Quat {
    construct new() {
    }
    construct new(x, y, z) { // Euler
    }
    construct new(x, y, z, w) { // Quat
    }
    foreign normalize()

    foreign normalized
    foreign toString
    
    foreign x
    foreign y
    foreign z
    foreign w
    foreign magnitude
    foreign magnitudeSqr
    foreign length
    foreign lengthSqr
    foreign x=(x)
    foreign y=(y)
    foreign z=(z)
    foreign w=(w)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenHandle* handle = nullptr;

      /////////////////////////////////////////////////////////////////////////
      glm::quat* make(WrenVM* vm, const glm::quat val = glm::quat())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Quat", "Quat", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        glm::quat* data = MakeForeign<glm::quat>(vm, 0, 1);
        memcpy(data, &val, sizeof(glm::quat));
        return data;
      }
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          int arg_count = wrenGetSlotCount(vm);
          if (arg_count == 4)
            make(vm, glm::quat(glm::vec3(
              (float)wrenGetSlotDouble(vm, 1), 
              (float)wrenGetSlotDouble(vm, 2), 
              (float)wrenGetSlotDouble(vm, 3)
            )));
          else if (arg_count == 5)
            make(vm, glm::quat(
              (float)wrenGetSlotDouble(vm, 4), 
              (float)wrenGetSlotDouble(vm, 1),
              (float)wrenGetSlotDouble(vm, 2), 
              (float)wrenGetSlotDouble(vm, 3)
            ));
        },
          [](void* data) {}
        };
      }
  
      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "x") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::quat>(vm)).x);
        };
        if (strcmp(signature, "y") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::quat>(vm)).y);
        };
        if (strcmp(signature, "z") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::quat>(vm)).z);
        };
        if (strcmp(signature, "w") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)(*GetForeign<glm::quat>(vm)).w);
        };
        if (strcmp(signature, "x=(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(wrenGetSlotType(vm, 1) == WREN_TYPE_NUM &&
            wrenGetSlotType(vm, 2) == WREN_TYPE_NUM, "Invalid call");
          (*GetForeign<glm::quat>(vm)).x = (float)wrenGetSlotDouble(vm, 2);
        };
        if (strcmp(signature, "y=(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(wrenGetSlotType(vm, 1) == WREN_TYPE_NUM && 
            wrenGetSlotType(vm, 2) == WREN_TYPE_NUM, "Invalid call");
          (*GetForeign<glm::quat>(vm)).y = (float)wrenGetSlotDouble(vm, 2);
        };
        if (strcmp(signature, "z=(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(wrenGetSlotType(vm, 1) == WREN_TYPE_NUM && 
            wrenGetSlotType(vm, 2) == WREN_TYPE_NUM, "Invalid call");
          (*GetForeign<glm::quat>(vm)).z = (float)wrenGetSlotDouble(vm, 2);
        };
        if (strcmp(signature, "w=(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(wrenGetSlotType(vm, 1) == WREN_TYPE_NUM &&
            wrenGetSlotType(vm, 2) == WREN_TYPE_NUM, "Invalid call");
          (*GetForeign<glm::quat>(vm)).w = (float)wrenGetSlotDouble(vm, 2);
        };
        if (strcmp(signature, "magnitude") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length(*GetForeign<glm::quat>(vm))
          );
        };
        if (strcmp(signature, "length") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length(*GetForeign<glm::quat>(vm))
          );
        };
        if (strcmp(signature, "magnitudeSqr") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length2(*GetForeign<glm::quat>(vm))
          );
        };
        if (strcmp(signature, "lengthSqr") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)glm::length2(*GetForeign<glm::quat>(vm))
          );
        };
        if (strcmp(signature, "normalize()") == 0) return [](WrenVM* vm) {
          glm::quat& vec = *GetForeign<glm::quat>(vm);
          vec = glm::normalize(vec);
        };
        if (strcmp(signature, "normalized") == 0) return [](WrenVM* vm) {
          make(vm, glm::normalize(*GetForeign<glm::quat>(vm)));
        };
        if (strcmp(signature, "toString") == 0) return [](WrenVM* vm) {
          glm::quat& vec = *GetForeign<glm::quat>(vm);
          String str = "[" + lambda::toString(vec.x) + ", " +
            lambda::toString(vec.y) + ", " + lambda::toString(vec.z) + ", " + 
            lambda::toString(vec.w) + "]";
          const char* c_str =
            (const char*)foundation::Memory::allocate(str.size() + 1u);
          memcpy((void*)c_str, str.data(), str.size() + 1u);
          wrenSetSlotString(vm, 0, c_str);
        };
        return nullptr;
      }
    }
    namespace Texture
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// texture.wren ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Texture {
    foreign static load(name)
    foreign static loadCubeMap(front, back, top, bottom, left, right)
    foreign static create(size, format)
    // Bytes will be handled as an array of uint32's
    foreign static create(size, bytes, format) 

    foreign size
}

class TextureFormat {
    static unknown      { 0  }
    static R8G8B8A8     { 1  }
    static R16G16B16A16 { 2  }
    static R32G32B32A32 { 3  }
    static R16G16       { 4  }
    static R32G32       { 5  }
    static R32          { 6  }
    static R16          { 7  }
    static R24G8        { 8  }
    static BC1          { 9  }
    static BC2          { 10 }
    static BC3          { 11 }
    static BC4          { 12 }
    static BC5          { 13 }
    static BC6          { 14 }
    static BC7          { 15 }
}


)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenHandle* handle = nullptr;
  
      /////////////////////////////////////////////////////////////////////////
      asset::VioletTextureHandle* make(
        WrenVM* vm, 
        asset::VioletTextureHandle val = asset::VioletTextureHandle())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Texture", "Texture", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        asset::VioletTextureHandle* data = 
          MakeForeign<asset::VioletTextureHandle>(vm, 0, 1);
        memcpy(data, &val, sizeof(asset::VioletTextureHandle));
        return data;
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          make(vm);
        },
          [](void* data) {
          asset::VioletTextureHandle& handle = 
            *((asset::VioletTextureHandle*)data);
          handle = nullptr;
        }
        };
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "load(_)") == 0) return [](WrenVM* vm) {
          Name name(wrenGetSlotString(vm, 1));
          asset::VioletTextureHandle& handle = *make(vm);
          handle = asset::TextureManager::getInstance()->get(name);
        };
        if (strcmp(signature, "loadCubeMap(_,_,_,_,_,_)") == 0) 
          return [](WrenVM* vm) {
          Name name_01(wrenGetSlotString(vm, 1));
          Name name_02(wrenGetSlotString(vm, 2));
          Name name_03(wrenGetSlotString(vm, 3));
          Name name_04(wrenGetSlotString(vm, 4));
          Name name_05(wrenGetSlotString(vm, 5));
          Name name_06(wrenGetSlotString(vm, 6));
          asset::VioletTextureHandle& handle = *make(vm);
          handle = asset::TextureManager::getInstance()->create(
            Name(toString(rand()))
          ); // TODO (Hilze): Fix ASAP!
          handle->addLayer(asset::TextureManager::getInstance()->get(name_01));
          handle->addLayer(asset::TextureManager::getInstance()->get(name_02));
          handle->addLayer(asset::TextureManager::getInstance()->get(name_03));
          handle->addLayer(asset::TextureManager::getInstance()->get(name_04));
          handle->addLayer(asset::TextureManager::getInstance()->get(name_05));
          handle->addLayer(asset::TextureManager::getInstance()->get(name_06));
        };
        if (strcmp(signature, "create(_,_)") == 0) return [](WrenVM* vm) {
          const glm::vec2& size = *GetForeign<glm::vec2>(vm, 1);
          asset::VioletTextureHandle& handle = *make(vm);
          TextureFormat format =
            (TextureFormat)(uint32_t)wrenGetSlotDouble(vm, 2);
          handle = asset::TextureManager::getInstance()->create(
            Name(toString(rand())), 
            (uint32_t)size.x, 
            (uint32_t)size.y, 
            1u, 
            format
          ); // TODO (Hilze): Fix ASAP!
        };
        if (strcmp(signature, "create(_,_,_)") == 0) return [](WrenVM* vm) {
          const glm::vec2& size = *GetForeign<glm::vec2>(vm, 1);
          asset::VioletTextureHandle& handle = *make(vm);
          // TODO (Hilze): Do something with the bytes.
          TextureFormat format = 
            (TextureFormat)(uint32_t)wrenGetSlotDouble(vm, 3);
          handle = asset::TextureManager::getInstance()->create(
            Name(toString(rand())), 
            (uint32_t)size.x,
            (uint32_t)size.y,
            1u, 
            format
          ); // TODO (Hilze): Fix ASAP!
        };
        if (strcmp(signature, "size") == 0) return [](WrenVM* vm) {
          asset::VioletTextureHandle& handle = 
            *GetForeign<asset::VioletTextureHandle>(vm);
          Vec2::make(
            vm, 
            glm::vec2(
              (float)handle->getLayer(0u).getWidth(),
              (float)handle->getLayer(0u).getHeight()
            )
          );
        };
        return nullptr;
      }
    }
    namespace Shader
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// shader.wren /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Shader {
    foreign static load(name)

    foreign setVariableFloat1(name, value)
    foreign setVariableFloat2(name, value)
    foreign setVariableFloat3(name, value)
    foreign setVariableFloat4(name, value)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenHandle* handle = nullptr;
     
      /////////////////////////////////////////////////////////////////////////
      asset::ShaderHandle* make(
        WrenVM* vm, 
        asset::ShaderHandle val = asset::ShaderHandle())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Shader", "Shader", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        asset::ShaderHandle* data = MakeForeign<asset::ShaderHandle>(vm, 0, 1);
        memcpy(data, &val, sizeof(asset::ShaderHandle));
        return data;
      }
     
      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          make(vm);
        },
          [](void* data) {
          asset::ShaderHandle& handle = *((asset::ShaderHandle*)data);
          handle.reset();
        }
        };
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "load(_)") == 0) return [](WrenVM* vm) {
          const char* ch = wrenGetSlotString(vm, 1);
          String str(strlen(ch), '\0');
          memcpy((void*)str.c_str(), ch, strlen(ch));
          Name name(str);
          asset::ShaderHandle& handle = *make(vm);
          static uint32_t s_idx = 0u;
          auto shader_01 = io::ShaderIO::load(name.getName());
          auto shader_02 = io::ShaderIO::asAsset(shader_01);
          auto shader = foundation::Memory::constructShared<asset::Shader>(
            shader_02
            );
          auto shader_name = 
            Name("__generated_shader_" + toString(s_idx++) + "__");
          handle = 
            asset::AssetManager::getInstance().createAsset<asset::Shader>(
            shader_name, shader
         );
        };
        if (strcmp(signature, "setVariableFloat1(_,_)") == 0) 
          return [](WrenVM* vm) {
          asset::ShaderHandle& handle = *GetForeign<asset::ShaderHandle>(vm);
          handle->setShaderVariable(
            platform::ShaderVariable(
              Name(wrenGetSlotString(vm, 1)), 
              (float)wrenGetSlotDouble(vm, 2)
            )
          );
        };
        if (strcmp(signature, "setVariableFloat2(_,_)") == 0) 
          return [](WrenVM* vm) {
          asset::ShaderHandle& handle = *GetForeign<asset::ShaderHandle>(vm);
          handle->setShaderVariable(
            platform::ShaderVariable(
              Name(wrenGetSlotString(vm, 1)), 
              *GetForeign<glm::vec2>(vm, 2)
            )
          );
        };
        if (strcmp(signature, "setVariableFloat3(_,_)") == 0) 
          return [](WrenVM* vm) {
          asset::ShaderHandle& handle = *GetForeign<asset::ShaderHandle>(vm);
          handle->setShaderVariable(
            platform::ShaderVariable(
              Name(wrenGetSlotString(vm, 1)), 
              *GetForeign<glm::vec3>(vm, 2)
            )
          );
        };
        if (strcmp(signature, "setVariableFloat4(_,_)") == 0) 
          return [](WrenVM* vm) {
          asset::ShaderHandle& handle = *GetForeign<asset::ShaderHandle>(vm);
          handle->setShaderVariable(
            platform::ShaderVariable(
              Name(wrenGetSlotString(vm, 1)),
              *GetForeign<glm::vec4>(vm, 2)
            )
          );
        };
        return nullptr;
      }
    }
    namespace Wave
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// wave.wren ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Wave {
    foreign static load(name)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenHandle* handle = nullptr;
      
      /////////////////////////////////////////////////////////////////////////
      asset::VioletWaveHandle* make(
        WrenVM* vm, 
        asset::VioletWaveHandle val = asset::VioletWaveHandle())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Wave", "Wave", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        asset::VioletWaveHandle* data = 
          MakeForeign<asset::VioletWaveHandle>(vm, 0, 1);
        memcpy(data, &val, sizeof(asset::VioletWaveHandle));
        return data;
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          make(vm);
        },
          [](void* data) {
        }
        };
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "load(_)") == 0) return [](WrenVM* vm) {
          Name name(wrenGetSlotString(vm, 1));
          asset::VioletWaveHandle& handle = *make(vm);
          handle = asset::WaveManager::getInstance()->get(name);
        };
        return nullptr;
      }
    }
    namespace Mesh
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// mesh.wren ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Mesh {
    foreign static load(name)
    foreign static generate(type)
    foreign static create()

    foreign subMeshCount

    foreign positions
    foreign positions=(vec)
    foreign normals
    foreign normals=(vec)
    foreign texCoords
    foreign texCoords=(vec)
    foreign colours
    foreign colours=(vec)
    foreign tangents
    foreign tangents=(vec)
    foreign joints
    foreign joints=(vec)
    foreign weights
    foreign weights=(vec)
    foreign indices
    foreign indices=(vec)

    foreign recalculateTangents()
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenHandle* handle = nullptr;
    
      /////////////////////////////////////////////////////////////////////////
      asset::MeshHandle* make(
        WrenVM* vm, 
        asset::MeshHandle val = asset::MeshHandle())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Mesh", "Mesh", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        asset::MeshHandle* data = MakeForeign<asset::MeshHandle>(vm, 0, 1);
        memcpy(data, &val, sizeof(asset::MeshHandle));
        return data;
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          make(vm);
        },
          [](void* data) {
          asset::MeshHandle& handle = *((asset::MeshHandle*)data);
          handle.reset();
        }
        };
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "load(_)") == 0) return [](WrenVM* vm) {
          Name name(wrenGetSlotString(vm, 1));
          asset::MeshHandle& handle = *make(vm);
          auto io_mesh = io::MeshIO::load(name.getName().c_str());
          handle = asset::AssetManager::getInstance().createAsset<asset::Mesh>(
            name, 
            foundation::Memory::constructShared<asset::Mesh>(
              io::MeshIO::asAsset(io_mesh)
            )
          );
        };
        if (strcmp(signature, "generate(_)") == 0) return [](WrenVM* vm) {
          static uint32_t s_idx = 0u;
          String type = wrenGetSlotString(vm, 1);
          Name name("__generated_mesh_" + toString(s_idx++) + "__");
          asset::MeshHandle& handle = *make(vm);
          if (type == "cube")
            handle = asset::AssetManager::getInstance().createAsset(
              name, 
              foundation::Memory::constructShared<asset::Mesh>(
                asset::Mesh::createCube()
              )
            );
          else if (type == "cylinder")
            handle = asset::AssetManager::getInstance().createAsset(
              name, 
              foundation::Memory::constructShared<asset::Mesh>(
                asset::Mesh::createCylinder()
              )
            );
          else if (type == "sphere")
            handle = asset::AssetManager::getInstance().createAsset(
              name,
              foundation::Memory::constructShared<asset::Mesh>(
                asset::Mesh::createSphere()
              )
            );
        };
        if (strcmp(signature, "create()") == 0) return [](WrenVM* vm) {
          static uint32_t s_idx = 0u;
          Name name("__created_mesh_" + toString(s_idx++) + "__");
          asset::MeshHandle& handle = *make(vm);
          handle = asset::AssetManager::getInstance().createAsset(
            name, foundation::Memory::constructShared<asset::Mesh>()
          );
        };
        if (strcmp(signature, "subMeshCount") == 0) return [](WrenVM* vm) {
          asset::MeshHandle& handle = *GetForeign<asset::MeshHandle>(vm);
          wrenSetSlotDouble(vm, 0, (double)handle->getSubMeshes().size());
        };
        if (strcmp(signature, "positions=(_)") == 0) return [](WrenVM* vm) {
          asset::MeshHandle mesh = *GetForeign<asset::MeshHandle>(vm);

          Vector<glm::vec3> positions(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)positions.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            positions[i] = *GetForeign<glm::vec3>(vm, 2);
          }

          mesh->set(asset::MeshElements::kPositions, positions);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offset.at(asset::MeshElements::kPositions).count = 
            positions.size();
          sub_meshes.back().offset.at(asset::MeshElements::kPositions).stride =
            sizeof(float) * 3u;

          glm::vec3 min(FLT_MAX);
          glm::vec3 max(FLT_MIN);
          for (const auto& it : positions)
          {
            if (it.x < min.x) min.x = it.x;
            if (it.y < min.y) min.y = it.y;
            if (it.z < min.z) min.z = it.z;
            if (it.x > max.x) max.x = it.x;
            if (it.y > max.y) max.y = it.y;
            if (it.z > max.z) max.z = it.z;
          }
          sub_meshes.back().min = min;
          sub_meshes.back().max = max;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "normals=(_)") == 0) return [](WrenVM* vm) {
          asset::MeshHandle mesh = *GetForeign<asset::MeshHandle>(vm);

          Vector<glm::vec3> normals(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)normals.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            normals[i] = *GetForeign<glm::vec3>(vm, 2);
          }

          mesh->set(asset::MeshElements::kNormals, normals);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offset.at(asset::MeshElements::kNormals).count =
            normals.size();
          sub_meshes.back().offset.at(asset::MeshElements::kNormals).stride = 
            sizeof(float) * 3u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "texCoords=(_)") == 0) return [](WrenVM* vm) {
          asset::MeshHandle mesh = *GetForeign<asset::MeshHandle>(vm);

          Vector<glm::vec2> tex_coords(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)tex_coords.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            tex_coords[i] = *GetForeign<glm::vec2>(vm, 2);
          }

          mesh->set(asset::MeshElements::kTexCoords, tex_coords);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offset.at(asset::MeshElements::kTexCoords).count = 
            tex_coords.size();
          sub_meshes.back().offset.at(asset::MeshElements::kTexCoords).stride = 
            sizeof(float) * 2u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "colours=(_)") == 0) return [](WrenVM* vm) {
          asset::MeshHandle mesh = *GetForeign<asset::MeshHandle>(vm);

          Vector<glm::vec4> colours(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)colours.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            colours[i] = *GetForeign<glm::vec4>(vm, 2);
          }

          mesh->set(asset::MeshElements::kColours, colours);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offset.at(asset::MeshElements::kColours).count = 
            colours.size();
          sub_meshes.back().offset.at(asset::MeshElements::kColours).stride = 
            sizeof(float) * 4u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "tangents=(_)") == 0) return [](WrenVM* vm) {
          asset::MeshHandle mesh = *GetForeign<asset::MeshHandle>(vm);

          Vector<glm::vec3> tangents(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)tangents.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            tangents[i] = *GetForeign<glm::vec3>(vm, 2);
          }

          mesh->set(asset::MeshElements::kTangents, tangents);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offset.at(asset::MeshElements::kTangents).count = 
            tangents.size();
          sub_meshes.back().offset.at(asset::MeshElements::kTangents).stride = 
            sizeof(float) * 3u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "joints=(_)") == 0) return [](WrenVM* vm) {
          asset::MeshHandle mesh = *GetForeign<asset::MeshHandle>(vm);

          Vector<glm::vec4> joints(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)joints.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            joints[i] = *GetForeign<glm::vec4>(vm, 2);
          }

          mesh->set(asset::MeshElements::kJoints, joints);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offset.at(asset::MeshElements::kJoints).count =
            joints.size();
          sub_meshes.back().offset.at(asset::MeshElements::kJoints).stride = 
            sizeof(float) * 4u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "weights=(_)") == 0) return [](WrenVM* vm) {
          asset::MeshHandle mesh = *GetForeign<asset::MeshHandle>(vm);

          Vector<glm::vec4> weights(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)weights.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            weights[i] = *GetForeign<glm::vec4>(vm, 2);
          }

          mesh->set(asset::MeshElements::kWeights, weights);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offset.at(asset::MeshElements::kWeights).count = 
            weights.size();
          sub_meshes.back().offset.at(asset::MeshElements::kWeights).stride =
            sizeof(float) * 4u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "indices=(_)") == 0) return [](WrenVM* vm) {
          asset::MeshHandle mesh = *GetForeign<asset::MeshHandle>(vm);

          Vector<uint32_t> indices(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)indices.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            indices[i] = (uint32_t)wrenGetSlotDouble(vm, 2);
          }

          mesh->set(asset::MeshElements::kIndices, indices);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offset.at(asset::MeshElements::kIndices).count = 
            indices.size();
          sub_meshes.back().offset.at(asset::MeshElements::kIndices).stride = 
            sizeof(uint32_t);
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "positions") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        if (strcmp(signature, "normals") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        if (strcmp(signature, "texCoords") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        if (strcmp(signature, "colours") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        if (strcmp(signature, "tangents") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        if (strcmp(signature, "joints") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        if (strcmp(signature, "weights") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        if (strcmp(signature, "indices") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        if (strcmp(signature, "recalculateTangents()") == 0) 
          return [](WrenVM* vm) {
          asset::MeshHandle mesh = *GetForeign<asset::MeshHandle>(vm);
          mesh->recalculateTangents();
        };
        return nullptr;
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    enum WrenComponentTypes
    {
      kUnknown = 0u << 1u,
      kTransform = 1u << 1u,
      kCamera = 2u << 1u,
      kMeshRender = 3u << 1u,
      kLOD = 4u << 1u,
      kRigidBody = 5u << 1u,
      kWaveSource = 6u << 1u,
      kCollider = 7u << 1u,
      kLight = 8u << 1u,
      kMonoBehaviour = 9u << 1u,
    };

    ///////////////////////////////////////////////////////////////////////////
    template<typename T>
    struct IWrenComponent
    {
      uint32_t type   = 0u;
      T        handle = T();
      entity::Entity entity;
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace Transform
    {
      using TransformHandle = IWrenComponent<components::TransformComponent>;
      WrenHandle* handle = nullptr;
    
      /////////////////////////////////////////////////////////////////////////
      TransformHandle* make(
        WrenVM* vm, 
        components::TransformComponent val = {})
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Transform", "Transform", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        TransformHandle* data = MakeForeign<TransformHandle>(vm, 0, 1);
        data->handle = val;
        return data;
      }
    }
    namespace Camera
    {
      using CameraHandle = IWrenComponent<components::CameraComponent>;
      WrenHandle* handle = nullptr;
     
      /////////////////////////////////////////////////////////////////////////
      CameraHandle* make(WrenVM* vm, components::CameraComponent val = {})
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Camera", "Camera", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        CameraHandle* data = MakeForeign<CameraHandle>(vm, 0, 1);
        data->handle = val;
        return data;
      }
    }
    namespace MeshRender
    {
      using MeshRenderHandle = IWrenComponent<components::MeshRenderComponent>;
      WrenHandle* handle = nullptr;
      
      /////////////////////////////////////////////////////////////////////////
      MeshRenderHandle* make(
        WrenVM* vm, 
        components::MeshRenderComponent val = {})
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/MeshRender", "MeshRender", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        MeshRenderHandle* data = MakeForeign<MeshRenderHandle>(vm, 0, 1);
        data->handle = val;
        return data;
      }
    }
    namespace LOD
    {
      using LODHandle = IWrenComponent<components::LODComponent>;
      WrenHandle* handle = nullptr;

      /////////////////////////////////////////////////////////////////////////
      LODHandle* make(WrenVM* vm, components::LODComponent val = {})
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Lod", "Lod", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        LODHandle* data = MakeForeign<LODHandle>(vm, 0, 1);
        data->handle = val;
        return data;
      }
    }
    namespace RigidBody
    {
      using RigidBodyHandle = IWrenComponent<components::RigidBodyComponent>;
      WrenHandle* handle = nullptr;
      
      /////////////////////////////////////////////////////////////////////////
      RigidBodyHandle* make(
        WrenVM* vm, 
        components::RigidBodyComponent val = {})
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/RigidBody", "RigidBody", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        RigidBodyHandle* data = MakeForeign<RigidBodyHandle>(vm, 0, 1);
        data->handle = val;
        return data;
      }
    }
    namespace WaveSource
    {
      using WaveSourceHandle = IWrenComponent<components::WaveSourceComponent>;
      WrenHandle* handle = nullptr;

      /////////////////////////////////////////////////////////////////////////
      WaveSourceHandle* make(
        WrenVM* vm, 
        components::WaveSourceComponent val = {})
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/WaveSource", "WaveSource", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        WaveSourceHandle* data = MakeForeign<WaveSourceHandle>(vm, 0, 1);
        data->handle = val;
        return data;
      }
    }
    namespace Collider
    {
      using ColliderHandle = IWrenComponent<components::ColliderComponent>;
      WrenHandle* handle = nullptr;

      /////////////////////////////////////////////////////////////////////////
      ColliderHandle* make(WrenVM* vm, components::ColliderComponent val = {})
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Collider", "Collider", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        ColliderHandle* data = MakeForeign<ColliderHandle>(vm, 0, 1);
        data->handle = val;
        return data;
      }
    }
    namespace Light
    {
      using LightHandle = IWrenComponent<components::BaseLightComponent>;
      WrenHandle* handle = nullptr;
      
      /////////////////////////////////////////////////////////////////////////
      LightHandle* make(WrenVM* vm, components::BaseLightComponent val = {})
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Light", "Light", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        LightHandle* data = MakeForeign<LightHandle>(vm, 0, 1);
        data->handle = val;
        return data;
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    struct ScriptingComponentData
    {
      entity::Entity entity;
      WrenHandle* transform;
      WrenHandle* camera;
      WrenHandle* mesh_render;
      WrenHandle* lod;
      WrenHandle* rigid_body;
      WrenHandle* wave_source;
      WrenHandle* collider;
      WrenHandle* light;
      WrenHandle* mono_behaviour;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct ScriptingData
    {
      ScriptingComponentData& getData(uint64_t id)
      {
        auto it = entity_to_data.find(id);
        if (it == entity_to_data.end())
        {
          entity_to_data[id] = data.size();
          data.push_back(ScriptingComponentData{});
          return data.back();
        }
        else
          return data[it->second];
      }

    private:
      Vector<ScriptingComponentData> data;
      UnorderedMap<uint64_t, uint64_t> entity_to_data;
    };

    ScriptingData g_scriptingData;

    ///////////////////////////////////////////////////////////////////////////
    namespace GameObject
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// gameObject.wren /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
import "Core/Transform" for Transform

foreign class GameObject {
    construct new() {
      addComponent(Transform)
    }

    addComponent(component) { 
      var c = component.new()
      c.goAdd(this)
      return c
    }
    getComponent(component) { 
      return component.goGet(this)
    }
    removeComponent(component) { 
      getComponent(component).goRemove(this)
    }

    [component] { getComponent(component) }

    transform { getComponent(Transform) }
    toString { "[%(id)]" }
    foreign id
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenHandle* handle = nullptr;
      
      /////////////////////////////////////////////////////////////////////////
      entity::Entity* make(WrenVM* vm, entity::Entity val = {})
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/GameObject", "GameObject", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        entity::Entity* data = MakeForeign<entity::Entity>(vm, 0, 1);
        memcpy(data, &val, sizeof(entity::Entity));
        return data;
      }
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          entity::Entity& handle = *make(vm);
          handle = g_entitySystem->createEntity();
        },
          [](void* data) {
          entity::Entity& handle = *((entity::Entity*)data);
          g_entitySystem->destroyEntity(handle);
          handle = entity::Entity();
        }
        };
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "id") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<entity::Entity>(vm)->id()
          );
        };
        return nullptr;
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace Transform
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// Transform.wren //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Transform {
    construct new() {
    }
    toString { "[%(gameObject.id): Transform]" }

    foreign gameObject

    foreign goAdd(gameObject)
    foreign static goGet(gameObject)
    foreign goRemove(gameObject)

    moveLocal(movement) { localPosition = localPosition + movement }
    moveWorld(movement) { worldPosition = worldPosition + movement }
    scaleLocal(scale) { localScale = localScale * scale }
    scaleWorld(scale) { worldScale = worldScale * scale }
    rotateEulerLocal(euler) { localEuler = localEuler * euler }
    rotateEulerWorld(euler) { worldEuler = worldEuler * euler }
    
    foreign parent
    foreign parent=(parent)

    foreign worldPosition    
    foreign worldPosition=(p)
    foreign worldScale       
    foreign worldScale=(s)   
    foreign worldRotation
    foreign worldRotation=(e)   
    foreign worldEuler
    foreign worldEuler=(e)   
    
    foreign localPosition    
    foreign localPosition=(p)
    foreign localScale       
    foreign localScale=(s)   
    foreign localRotation
    foreign localRotation=(e)   
    foreign localEuler       
    foreign localEuler=(e)   
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
  
      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          TransformHandle& handle = *make(vm);
          handle.type = WrenComponentTypes::kTransform;
        },
          [](void* data) {
          TransformHandle& handle = *((TransformHandle*)data);
          handle.handle = components::TransformComponent();
          handle.type   = WrenComponentTypes::kUnknown;
          handle.entity = entity::Entity();
        }
        };
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
          GameObject::make(
            vm, 
            GetForeign<TransformHandle>(vm)->handle.entity()
          );
        };
        if (strcmp(signature, "goAdd(_)") == 0) return [](WrenVM* vm) {
          TransformHandle* handle = GetForeign<TransformHandle>(vm);
          entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          g_scriptingData.getData(e.id()).transform = wrenGetSlotHandle(vm, 0);
          handle->handle = g_transformSystem->addComponent(e);
          handle->entity = e;
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData.getData(
              GetForeign<entity::Entity>(vm, 1)->id()
            ).transform
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          TransformHandle* handle = GetForeign<TransformHandle>(vm);
          g_transformSystem->removeComponent(handle->handle.entity());
          wrenReleaseHandle(
            vm, 
            g_scriptingData.getData(handle->handle.entity()).transform
          );
          g_scriptingData.getData(handle->handle.entity()).transform = nullptr;
          handle->handle = components::TransformComponent();
          handle->entity = entity::Entity();
        };
        if (strcmp(signature, "parent") == 0) return [](WrenVM* vm) {
          Transform::make(
            vm, 
            GetForeign<TransformHandle>(vm)->handle.getParent()
          );
        };
        if (strcmp(signature, "parent=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<TransformHandle>(vm)->handle.setParent(
            GetForeign<TransformHandle>(vm, 1)->handle
          );
        };
        if (strcmp(signature, "worldPosition") == 0) return [](WrenVM* vm) {
          Vec3::make(
            vm, 
            GetForeign<TransformHandle>(vm)->handle.getWorldTranslation()
          );
        };
        if (strcmp(signature, "worldPosition=(_)") == 0)
          return [](WrenVM* vm) {
          GetForeign<TransformHandle>(vm)->handle.setWorldTranslation(
            *GetForeign<glm::vec3>(vm, 1)
          );
        };
        if (strcmp(signature, "worldScale") == 0) return [](WrenVM* vm) {
          Vec3::make(
            vm, 
            GetForeign<TransformHandle>(vm)->handle.getWorldScale()
          );
        };
        if (strcmp(signature, "worldScale=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<TransformHandle>(vm)->handle.setWorldScale(
            *GetForeign<glm::vec3>(vm, 1)
          );
        };
        if (strcmp(signature, "worldRotation") == 0) return [](WrenVM* vm) {
          Quat::make(
            vm, 
            GetForeign<TransformHandle>(vm)->handle.getWorldRotation()
          );
        };
        if (strcmp(signature, "worldRotation=(_)") == 0) 
          return [](WrenVM* vm) {
          GetForeign<TransformHandle>(vm)->handle.setWorldRotation(
            *GetForeign<glm::quat>(vm, 1)
          );
        };
        if (strcmp(signature, "worldEuler") == 0) return [](WrenVM* vm) {
          Vec3::make(
            vm, 
            glm::eulerAngles(
              GetForeign<TransformHandle>(vm)->handle.getWorldRotation()
            )
          );
        };
        if (strcmp(signature, "worldEuler=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<TransformHandle>(vm)->handle.setWorldRotation(
            *GetForeign<glm::vec3>(vm, 1)
          );
        };
        if (strcmp(signature, "localPosition") == 0) return [](WrenVM* vm) {
          Vec3::make(
            vm, 
            GetForeign<TransformHandle>(vm)->handle.getLocalTranslation()
          );
        };
        if (strcmp(signature, "localPosition=(_)") == 0)
          return [](WrenVM* vm) {
          GetForeign<TransformHandle>(vm)->handle.setLocalTranslation(
            *GetForeign<glm::vec3>(vm, 1)
          );
        };
        if (strcmp(signature, "localScale") == 0) return [](WrenVM* vm) {
          Vec3::make(
            vm,
            GetForeign<TransformHandle>(vm)->handle.getLocalScale()
          );
        };
        if (strcmp(signature, "localScale=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<TransformHandle>(vm)->handle.setLocalScale(
            *GetForeign<glm::vec3>(vm, 1)
          );
        };
        if (strcmp(signature, "localRotation") == 0) return [](WrenVM* vm) {
          Quat::make(
            vm, 
            GetForeign<TransformHandle>(vm)->handle.getLocalRotation()
          );
        };
        if (strcmp(signature, "localRotation=(_)") == 0) 
          return [](WrenVM* vm) {
          GetForeign<TransformHandle>(vm)->handle.setLocalRotation(
            *GetForeign<glm::quat>(vm, 1)
          );
        };
        if (strcmp(signature, "localEuler") == 0) return [](WrenVM* vm) {
          Vec3::make(
            vm, 
            glm::eulerAngles(
              GetForeign<TransformHandle>(vm)->handle.getLocalRotation()
            )
          );
        };
        if (strcmp(signature, "localEuler=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<TransformHandle>(vm)->handle.setLocalRotation(
            *GetForeign<glm::vec3>(vm, 1)
          );
        };
        return nullptr;
      }
    }
    namespace Camera
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// camera.wren /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Camera {
    construct new() {
    }
    toString { "[%(gameObject.id): Camera]" }
    
    foreign gameObject
    
    foreign goAdd(gameObject)
    foreign static goGet(gameObject)
    foreign goRemove(gameObject)

    foreign addShaderPass(name, shader, input, output)

    foreign near
    foreign near=(near)
    foreign far
    foreign far=(far)
    foreign fovDeg
    foreign fovDeg=(deg)
    foreign fovRad
    foreign fovRad=(rad)
    fov { fovRad }
    fov(rad) { fovRad = rad }
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          CameraHandle& handle = *make(vm);
          handle.type = WrenComponentTypes::kCamera;
        },
          [](void* data) {
          CameraHandle& handle = *((CameraHandle*)data);
          handle.handle = components::CameraComponent();
          handle.type = WrenComponentTypes::kUnknown;
          handle.entity = entity::Entity();
        }
        };
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
          GameObject::make(vm, GetForeign<CameraHandle>(vm)->handle.entity());
        };
        if (strcmp(signature, "goAdd(_)") == 0) return [](WrenVM* vm) {
          CameraHandle* handle = GetForeign<CameraHandle>(vm);
          entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          handle->handle = g_cameraSystem->addComponent(e);
          handle->entity = e;
          
          g_scriptingData.getData(handle->handle.entity().id()).camera = 
            wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData.getData(
              GetForeign<entity::Entity>(vm, 1)->id()
            ).camera
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          CameraHandle* handle = GetForeign<CameraHandle>(vm);
          /*Handle*/ wrenReleaseHandle(
            vm, 
            g_scriptingData.getData(handle->handle.entity()).camera
          );
          g_scriptingData.getData(handle->handle.entity()).camera = nullptr;
          g_cameraSystem->removeComponent(handle->handle.entity());
          handle->handle = components::CameraComponent();
          handle->entity = entity::Entity();
        };
        if (strcmp(signature, "near") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<CameraHandle>(
              vm
              )->handle.getNearPlane().asMeter()
          );
        };
        if (strcmp(signature, "near=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<CameraHandle>(vm)->handle.setNearPlane(
            utilities::Distance::fromMeter((float)wrenGetSlotDouble(vm, 1))
          );
        };
        if (strcmp(signature, "far") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<CameraHandle>(
              vm
              )->handle.getFarPlane().asMeter()
          );
        };
        if (strcmp(signature, "far=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<CameraHandle>(vm)->handle.setFarPlane(
            utilities::Distance::fromMeter((float)wrenGetSlotDouble(vm, 1))
          );
        };
        if (strcmp(signature, "fovRad") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<CameraHandle>(vm)->handle.getFov().asRad()
          );
        };
        if (strcmp(signature, "fovRad=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<CameraHandle>(vm)->handle.setFov(
            utilities::Angle::fromRad((float)wrenGetSlotDouble(vm, 1))
          );
        };
        if (strcmp(signature, "fovDeg") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<CameraHandle>(vm)->handle.getFov().asDeg()
          );
        };
        if (strcmp(signature, "fovDeg=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<CameraHandle>(vm)->handle.setFov(
            utilities::Angle::fromDeg((float)wrenGetSlotDouble(vm, 1))
          );
        };
        if (strcmp(signature, "addShaderPass(_,_,_,_)") == 0)
          return [](WrenVM* vm)
        {
          components::CameraComponent camera = 
            GetForeign<CameraHandle>(vm)->handle;
          
          Name name(wrenGetSlotString(vm, 1));
          asset::ShaderHandle shader = *GetForeign<asset::ShaderHandle>(vm, 2);
          Vector<platform::RenderTarget> inputs(wrenGetListCount(vm, 3));
          Vector<platform::RenderTarget> outputs(wrenGetListCount(vm, 4));

          for (int i = 0; i < (int)inputs.size(); ++i)
          {
            wrenGetListElement(vm, 3, i, 0);
            inputs[i] = 
              g_world->getPostProcessManager().getTarget(
                Name(wrenGetSlotString(vm, 0))
              );
          }
          for (int i = 0; i < (int)outputs.size(); ++i)
          {
            wrenGetListElement(vm, 4, i, 0);
            outputs[i] = 
              g_world->getPostProcessManager().getTarget(
                Name(wrenGetSlotString(vm, 0))
              );
          }

          camera.addShaderPass(
            platform::ShaderPass(name, shader, inputs, outputs)
          );
        };
        return nullptr;
      }
    }
    namespace MeshRender
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// msehRender.wren /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class MeshRender {
    construct new() {
    }
    toString { "[%(gameObject.id): MeshRender]" }

    foreign gameObject
    
    foreign goAdd(gameObject)
    foreign static goGet(gameObject)
    foreign goRemove(gameObject)

    foreign attach(mesh)
    
    foreign makeStatic()
    foreign makeStaticRecursive()
    foreign makeDynamic()
    foreign makeDynamicRecursive()

    foreign mesh
    foreign mesh=(mesh)
    foreign subMesh
    foreign subMesh=(subMesh)
    foreign albedo
    foreign albedo=(albedo)
    foreign normal
    foreign normal=(normal)
    foreign metallicRoughness
    foreign metallicRoughness=(mr)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          MeshRenderHandle& handle = *make(vm);
          handle.type = WrenComponentTypes::kMeshRender;
        },
          [](void* data) {
          MeshRenderHandle& handle = *((MeshRenderHandle*)data);
          handle.handle = components::MeshRenderComponent();
          handle.type = WrenComponentTypes::kUnknown;
          handle.entity = entity::Entity();
        }
        };
      }
     
      /////////////////////////////////////////////////////////////////////////
      void MakeStaticRecursive(const uint64_t& id)
      {
        const entity::Entity e(id, g_entitySystem);
        if (g_meshRenderSystem->hasComponent(e))
          g_meshRenderSystem->makeStatic(e);

        if (g_transformSystem->hasComponent(e))
          for (const auto& child : g_transformSystem->getChildren(e))
            MakeStaticRecursive(child.id());
      }
    
      /////////////////////////////////////////////////////////////////////////
      void MakeDynamicRecursive(const uint64_t& id)
      {
        const entity::Entity e(id, g_entitySystem);
        if (g_meshRenderSystem->hasComponent(e))
          g_meshRenderSystem->makeDynamic(e);

        if (g_transformSystem->hasComponent(e))
          for (const auto& child : g_transformSystem->getChildren(e))
            MakeDynamicRecursive(child.id());
      }
     
      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
          GameObject::make(
            vm, 
            GetForeign<MeshRenderHandle>(vm)->handle.entity()
          );
        };
        if (strcmp(signature, "goAdd(_)") == 0) return [](WrenVM* vm) {
          MeshRenderHandle* handle = GetForeign<MeshRenderHandle>(vm);
          entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          handle->handle = g_meshRenderSystem->addComponent(e);
          handle->entity = e;
          g_scriptingData.getData(handle->handle.entity().id()).mesh_render = 
            wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData.getData(
              GetForeign<entity::Entity>(vm, 1)->id()
            ).mesh_render
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          MeshRenderHandle* handle = GetForeign<MeshRenderHandle>(vm);
          g_meshRenderSystem->removeComponent(handle->handle.entity());
          handle->handle = components::MeshRenderComponent();
          handle->entity = entity::Entity();
          wrenReleaseHandle(
            vm, 
            g_scriptingData.getData(handle->handle.entity()).mesh_render
          );
          g_scriptingData.getData(handle->handle.entity()).mesh_render = 
            nullptr;
        };
        if (strcmp(signature, "attach(_)") == 0) return [](WrenVM* vm) {
          GetForeign<MeshRenderHandle>(vm)->handle.attachMesh(
            *GetForeign<asset::MeshHandle>(vm, 1)
          );
        };
        if (strcmp(signature, "mesh=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<MeshRenderHandle>(vm)->handle.setMesh(
            *GetForeign<asset::MeshHandle>(vm, 1)
          );
        };
        if (strcmp(signature, "mesh") == 0) return [](WrenVM* vm) {
          Mesh::make(vm, GetForeign<MeshRenderHandle>(vm)->handle.getMesh());
        };
        if (strcmp(signature, "subMesh=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<MeshRenderHandle>(vm)->handle.setSubMesh(
            (uint32_t)wrenGetSlotDouble(vm, 1)
          );
        };
        if (strcmp(signature, "subMesh") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<MeshRenderHandle>(vm)->handle.getSubMesh()
          );
        };
        if (strcmp(signature, "albedo=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<MeshRenderHandle>(vm)->handle.setAlbedoTexture(
            *GetForeign<asset::VioletTextureHandle>(vm, 1)
          );
        };
        if (strcmp(signature, "albedo") == 0) return [](WrenVM* vm) {
          Texture::make(
            vm, 
            GetForeign<MeshRenderHandle>(vm)->handle.getAlbedoTexture()
          );
        };
        if (strcmp(signature, "normal=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<MeshRenderHandle>(vm)->handle.setNormalTexture(
            *GetForeign<asset::VioletTextureHandle>(vm, 1)
          );
        };
        if (strcmp(signature, "normal") == 0) return [](WrenVM* vm) {
          Texture::make(
            vm, 
            GetForeign<MeshRenderHandle>(vm)->handle.getNormalTexture()
          );
        };
        if (strcmp(signature, "metallicRoughness=(_)") == 0) 
          return [](WrenVM* vm) {
          GetForeign<MeshRenderHandle>(vm)->handle.setMetallicRoughnessTexture(
            *GetForeign<asset::VioletTextureHandle>(vm, 1)
          );
        };
        if (strcmp(signature, "metallicRoughness") == 0) 
          return [](WrenVM* vm) {
          Texture::make(
            vm, 
            GetForeign<MeshRenderHandle>(
              vm
              )->handle.getMetallicRoughnessTexture()
          );
        };
        if (strcmp(signature, "makeStatic()") == 0) return [](WrenVM* vm) {
          g_meshRenderSystem->makeStatic(
            GetForeign<MeshRenderHandle>(vm)->handle.entity()
          );
        };
        if (strcmp(signature, "makeDynamic()") == 0) return [](WrenVM* vm) {
          g_meshRenderSystem->makeDynamic(
            GetForeign<MeshRenderHandle>(vm)->handle.entity()
          );
        };
        if (strcmp(signature, "makeStaticRecursive()") == 0) 
          return [](WrenVM* vm) {
          MakeStaticRecursive(
            GetForeign<MeshRenderHandle>(vm)->handle.entity()
          );
        };
        if (strcmp(signature, "makeDynamicRecursive()") == 0) 
          return [](WrenVM* vm) {
          MakeDynamicRecursive(
            GetForeign<MeshRenderHandle>(vm)->handle.entity()
          );
        };
        return nullptr;
      }
    }
    namespace LOD
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// lod.wren ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Lod {
    construct new() {
    }
    toString { "[%(gameObject.id): Lod]" }

    foreign gameObject

    foreign goAdd(gameObject)
    foreign static goGet(gameObject)
    foreign goRemove(gameObject)

    foreign addLod(mesh, distance)
    foreign addLodRecursive(mesh, distance)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          LODHandle& handle = *make(vm);
          handle.type = WrenComponentTypes::kLOD;
        },
          [](void* data) {
          LODHandle& handle = *((LODHandle*)data);
          handle.handle = components::LODComponent();
          handle.type = WrenComponentTypes::kUnknown;
          handle.entity = entity::Entity();
        }
        };
      }
 
      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
          GameObject::make(vm, GetForeign<LODHandle>(vm)->handle.entity());
        };
        if (strcmp(signature, "goAdd(_)") == 0) return [](WrenVM* vm) {
          LODHandle* handle = GetForeign<LODHandle>(vm);
          entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          handle->handle = g_lodSystem->addComponent(e);
          handle->entity = e;
          g_scriptingData.getData(handle->handle.entity().id()).lod = 
            wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData.getData(
              GetForeign<entity::Entity>(vm, 1)->id()
            ).lod
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          LODHandle* handle = GetForeign<LODHandle>(vm);
          g_lodSystem->removeComponent(handle->handle.entity());
          handle->handle = components::LODComponent();
          handle->entity = entity::Entity();
          wrenReleaseHandle(
            vm, 
            g_scriptingData.getData(handle->handle.entity()).lod
          );
          g_scriptingData.getData(handle->handle.entity()).lod = nullptr;
        };
        if (strcmp(signature, "addLod(_,_)") == 0) return [](WrenVM* vm) {
          components::LOD lod;
          lod.setMesh(*GetForeign<asset::MeshHandle>(vm, 1));
          lod.setDistance((float)wrenGetSlotDouble(vm, 2));
          GetForeign<LODHandle>(vm)->handle.addLOD(lod);
        };
        if (strcmp(signature, "addLodRecursive(_,_)") == 0) 
          return [](WrenVM* vm) {
          components::LOD lod;
          lod.setMesh(*GetForeign<asset::MeshHandle>(vm, 1));
          lod.setDistance((float)wrenGetSlotDouble(vm, 2));

          entity::Entity e = GetForeign<LODHandle>(vm)->handle.entity();
          std::function<void(entity::Entity, const lambda::components::LOD&)> 
            addLOD =  [e, lod, &addLOD]
            (entity::Entity entity, const lambda::components::LOD& lod)->void {
            if (g_meshRenderSystem->hasComponent(entity) &&
              g_meshRenderSystem->getMesh(entity) != nullptr)
            {
              if (false == g_lodSystem->hasComponent(entity))
                g_lodSystem->addComponent(entity);

              g_lodSystem->addLOD(entity, lod);
            }

            for (const auto& child : g_transformSystem->getChildren(entity))
              addLOD(child, lod);
          };
        };
        return nullptr;
      }
    }
    namespace RigidBody
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// rigidBody.wren //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RigidBody {
    construct new() {
    }
    toString { "[%(gameObject.id): RigidBody]" }

    foreign gameObject

    foreign goAdd(gameObject)
    foreign static goGet(gameObject)
    foreign goRemove(gameObject)

    foreign applyImpulse(impulse)

    foreign velocity
    foreign velocity=(velocity)
    foreign angularVelocity                           
    foreign angularVelocity=(angularVelocity)         
    foreign velocityConstraints                       
    foreign velocityConstraints=(velocityConstraints) 
    foreign angularConstraints                        
    foreign angularConstraints=(angularConstraints)   
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          RigidBodyHandle& handle = *make(vm);
          handle.type = WrenComponentTypes::kRigidBody;
        },
          [](void* data) {
          RigidBodyHandle& handle = *((RigidBodyHandle*)data);
          handle.handle = components::RigidBodyComponent();
          handle.type = WrenComponentTypes::kUnknown;
          handle.entity = entity::Entity();
        }
        };
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
          GameObject::make(
            vm, 
            GetForeign<RigidBodyHandle>(vm)->handle.entity()
          );
        };
        if (strcmp(signature, "goAdd(_)") == 0) return [](WrenVM* vm) {
          RigidBodyHandle* handle = GetForeign<RigidBodyHandle>(vm);
          entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          handle->handle = g_rigidBodySystem->addComponent(e);
          handle->entity = e;
          g_scriptingData.getData(handle->handle.entity().id()).rigid_body = 
            wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData.getData(
              GetForeign<entity::Entity>(vm, 1)->id()
            ).rigid_body
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          RigidBodyHandle* handle = GetForeign<RigidBodyHandle>(vm);
          g_rigidBodySystem->removeComponent(handle->handle.entity());
          handle->handle = components::RigidBodyComponent();
          handle->entity = entity::Entity();
          wrenReleaseHandle(
            vm, 
            g_scriptingData.getData(handle->handle.entity()).rigid_body
          );
          g_scriptingData.getData(handle->handle.entity()).rigid_body = 
            nullptr;
        };
        if (strcmp(signature, "applyImpulse(_)") == 0) return [](WrenVM* vm) {
          GetForeign<RigidBodyHandle>(vm)->handle.applyImpulse(
            *GetForeign<glm::vec3>(vm, 1)
          );
        };
        if (strcmp(signature, "velocity=(_)") == 0) return [](WrenVM* vm) {
          g_rigidBodySystem->setVelocity(
            GetForeign<RigidBodyHandle>(vm)->handle.entity(), 
            *GetForeign<glm::vec3>(vm, 1)
          );
        };
        if (strcmp(signature, "velocity") == 0) return [](WrenVM* vm) {
          Vec3::make(
            vm, 
            g_rigidBodySystem->getVelocity(
              GetForeign<RigidBodyHandle>(vm)->handle.entity()
            )
          );
        };
        if (strcmp(signature, "angularVelocity=(_)") == 0) 
          return [](WrenVM* vm) {
          GetForeign<RigidBodyHandle>(vm)->handle.setAngularVelocity(
            *GetForeign<glm::vec3>(vm, 1)
          );
        };
        if (strcmp(signature, "angularVelocity") == 0) return [](WrenVM* vm) {
          Vec3::make(
            vm,
            GetForeign<RigidBodyHandle>(vm)->handle.getAngularVelocity()
          );
        };
        if (strcmp(signature, "velocityConstraints=(_)") == 0)
          return [](WrenVM* vm) {
          GetForeign<RigidBodyHandle>(vm)->handle.setVelocityConstraints(
            (uint8_t)wrenGetSlotDouble(vm, 1)
          );
        };
        if (strcmp(signature, "velocityConstraints") == 0) 
          return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<RigidBodyHandle>(
              vm
              )->handle.getVelocityConstraints()
          );
        };
        if (strcmp(signature, "angularConstraints=(_)") == 0)
          return [](WrenVM* vm) {
          g_rigidBodySystem->setAngularConstraints(
            GetForeign<RigidBodyHandle>(vm)->handle.entity(), 
            (uint8_t)wrenGetSlotDouble(vm, 1)
          );
        };
        if (strcmp(signature, "angularConstraints") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<RigidBodyHandle>(
              vm
              )->handle.getAngularConstraints()
          );
        };
        return nullptr;
      }
    }
    namespace WaveSource
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// waveSource.wren /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class WaveSource {
    construct new() {
    }
    toString { "[%(gameObject.id): WaveSource]" }
    
    foreign gameObject

    foreign goAdd(gameObject)
    foreign static goGet(gameObject)
    foreign goRemove(gameObject)

    foreign play()
    foreign pause()
    foreign stop()
    foreign makeMainListener()

    foreign buffer
    foreign buffer=(buffer)
    foreign state
    foreign relativeToListener
    foreign relativeToListener=(relative)
    foreign loop
    foreign loop=(loop)
    foreign offset=(offset)
    foreign volume
    foreign volume=(volume)
    foreign gain
    foreign gain=(gain)
    foreign pitch 
    foreign pitch=(pitch) 
    foreign radius 
    foreign radius=(radius) 
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          WaveSourceHandle& handle = *make(vm);
          handle.type = WrenComponentTypes::kWaveSource;
        },
          [](void* data) {
          WaveSourceHandle& handle = *((WaveSourceHandle*)data);
          handle.handle = components::WaveSourceComponent();
          handle.type = WrenComponentTypes::kUnknown;
          handle.entity = entity::Entity();
        }
        };
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
          GameObject::make(
            vm, 
            GetForeign<WaveSourceHandle>(vm)->handle.entity()
          );
        };
        if (strcmp(signature, "goAdd(_)") == 0) return [](WrenVM* vm) {
          WaveSourceHandle* handle = GetForeign<WaveSourceHandle>(vm);
          entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          handle->handle = g_waveSourceSystem->addComponent(e);
          handle->entity = e;
          g_scriptingData.getData(
            handle->handle.entity().id()
          ).wave_source = wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData.getData(
              GetForeign<entity::Entity>(vm, 1)->id()
            ).wave_source
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          WaveSourceHandle* handle = GetForeign<WaveSourceHandle>(vm);
          g_waveSourceSystem->removeComponent(handle->handle.entity());
          handle->handle = components::WaveSourceComponent();
          handle->entity = entity::Entity();
          wrenReleaseHandle(
            vm, 
            g_scriptingData.getData(handle->handle.entity()).wave_source
          );
          g_scriptingData.getData(handle->handle.entity()).wave_source = 
            nullptr;
        };
        if (strcmp(signature, "buffer=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.setBuffer(
            *GetForeign<asset::VioletWaveHandle>(vm, 1)
          );
        };
        if (strcmp(signature, "buffer") == 0) return [](WrenVM* vm) {
          Wave::make(vm, GetForeign<WaveSourceHandle>(vm)->handle.getBuffer());
        };
        if (strcmp(signature, "play()") == 0) return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.play();
        };
        if (strcmp(signature, "pause()") == 0) return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.pause();
        };
        if (strcmp(signature, "stop()") == 0) return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.stop();
        };
        if (strcmp(signature, "state") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<WaveSourceHandle>(vm)->handle.getState()
          );
        };
        if (strcmp(signature, "relativeToListener=(_)") == 0)
          return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.setRelativeToListener(
            wrenGetSlotBool(vm, 1)
          );
        };
        if (strcmp(signature, "relativeToListener") == 0)
          return [](WrenVM* vm) {
          wrenSetSlotBool(
            vm, 
            0, 
            GetForeign<WaveSourceHandle>(vm)->handle.getRelativeToListener()
          );
        };
        if (strcmp(signature, "loop=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.setLoop(
            wrenGetSlotBool(vm, 1)
          );
        };
        if (strcmp(signature, "loop") == 0) return [](WrenVM* vm) {
          wrenSetSlotBool(
            vm, 
            0, 
            GetForeign<WaveSourceHandle>(vm)->handle.getLoop()
          );
        };
        if (strcmp(signature, "offset=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.setOffset(
            (float)wrenGetSlotDouble(vm, 1)
          );
        };
        if (strcmp(signature, "volume=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.setVolume(
            (float)wrenGetSlotDouble(vm, 1)
          );
        };
        if (strcmp(signature, "volume") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<WaveSourceHandle>(vm)->handle.getVolume()
          );
        };
        if (strcmp(signature, "gain=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.setGain(
            (float)wrenGetSlotDouble(vm, 1)
          );
        };
        if (strcmp(signature, "gain") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<WaveSourceHandle>(vm)->handle.getGain()
          );
        };
        if (strcmp(signature, "pitch=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.setPitch(
            (float)wrenGetSlotDouble(vm, 1)
          );
        };
        if (strcmp(signature, "pitch") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<WaveSourceHandle>(vm)->handle.getPitch()
          );
        };
        if (strcmp(signature, "radius=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<WaveSourceHandle>(vm)->handle.setRadius(
            (float)wrenGetSlotDouble(vm, 1)
          );
        };
        if (strcmp(signature, "radius") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(
            vm, 
            0, 
            (double)GetForeign<WaveSourceHandle>(vm)->handle.getRadius()
          );
        };
        if (strcmp(signature, "makeMainListener()") == 0)
          return [](WrenVM* vm) {
          g_waveSourceSystem->setListener(
            GetForeign<WaveSourceHandle>(vm)->handle.entity()
          );
        };
        return nullptr;
      }
    }
    namespace Collider
    {
      /////////////////////////////////////////////////////////////////////////
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////
///// collider.wren ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
foreign class Collider {
    construct new() {
    }
    toString { "[%(gameObject.id): Collider]" }
    
    foreign gameObject

    foreign goAdd(gameObject)
    foreign static goGet(gameObject)
    foreign goRemove(gameObject)

    foreign makeBoxCollider()
    foreign makeSphereCollider()
    foreign makeCapsuleCollider()
    foreign makeMeshCollider(mesh, subMesh)
    foreign makeMeshColliderRecursive(mesh)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          ColliderHandle& handle = *make(vm);
          handle.type = WrenComponentTypes::kCollider;
        },
          [](void* data) {
          ColliderHandle& handle = *((ColliderHandle*)data);
          handle.handle = components::ColliderComponent();
          handle.type = WrenComponentTypes::kUnknown;
          handle.entity = entity::Entity();
        }
        };
      }
      void addMeshCollider(entity::Entity entity, asset::MeshHandle mesh)
      {
        bool add_mesh_collider = false;

        if (mesh &&
          (true == g_meshRenderSystem->hasComponent(entity) &&
          mesh == g_meshRenderSystem->getMesh(entity) &&
          mesh->getSubMeshes().at(g_meshRenderSystem->getSubMesh(entity)).offset.at(asset::MeshElements::kPositions).count > 0u) &&
          (false == (g_meshRenderSystem->getAlbedoTexture(entity) != nullptr &&
            g_meshRenderSystem->getAlbedoTexture(entity)->getLayer(0u).containsAlpha())))
          add_mesh_collider = true;

        if (add_mesh_collider)
        {
          if (false == g_colliderSystem->hasComponent(entity))
            g_colliderSystem->addComponent(entity);

          g_colliderSystem->makeMeshCollider(entity, mesh, g_meshRenderSystem->getSubMesh(entity));
        }
        else if (true == g_colliderSystem->hasComponent(entity))
          g_colliderSystem->removeComponent(entity);

        for (const auto& child : g_transformSystem->getChildren(entity))
          addMeshCollider(child, mesh);
      }
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
          GameObject::make(vm, GetForeign<ColliderHandle>(vm)->handle.entity());
        };
        if (strcmp(signature, "goAdd(_)") == 0) return [](WrenVM* vm) {
          ColliderHandle* handle = GetForeign<ColliderHandle>(vm);
          entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          handle->handle = g_colliderSystem->addComponent(e);
          handle->entity = e;
          /*Handle*/ g_scriptingData.getData(handle->handle.entity().id()).collider = wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          /*Handle*/ wrenSetSlotHandle(vm, 0, g_scriptingData.getData(GetForeign<entity::Entity>(vm, 1)->id()).collider);
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          ColliderHandle* handle = GetForeign<ColliderHandle>(vm);
          g_colliderSystem->removeComponent(handle->handle.entity());
          handle->handle = components::ColliderComponent();
          handle->entity = entity::Entity();
          /*Handle*/ wrenReleaseHandle(vm, g_scriptingData.getData(handle->handle.entity()).collider);
          /*Handle*/ g_scriptingData.getData(handle->handle.entity()).collider = nullptr;
        };
        if (strcmp(signature, "makeBoxCollider()") == 0) return [](WrenVM* vm) {
          GetForeign<ColliderHandle>(vm)->handle.makeBoxCollider();
        };
        if (strcmp(signature, "makeSphereCollider()") == 0) return [](WrenVM* vm) {
          GetForeign<ColliderHandle>(vm)->handle.makeSphereCollider();
        };
        if (strcmp(signature, "makeCapsuleCollider()") == 0) return [](WrenVM* vm) {
          GetForeign<ColliderHandle>(vm)->handle.makeCapsuleCollider();
        };
        if (strcmp(signature, "makeMeshCollider(_,_)") == 0) return [](WrenVM* vm) {
          GetForeign<ColliderHandle>(vm)->handle.makeMeshCollider(*GetForeign<asset::MeshHandle>(vm, 1), (uint32_t)wrenGetSlotDouble(vm, 2));
        };

        if (strcmp(signature, "makeMeshColliderRecursive(_)") == 0) return [](WrenVM* vm) {
          addMeshCollider(GetForeign<ColliderHandle>(vm)->handle.entity(), *GetForeign<asset::MeshHandle>(vm, 1));
        };
        return nullptr;
      }
    }
    namespace Light
    {
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////////////////////////
///// light.wren //////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
foreign class Light {
    construct new() {
    }
    toString { "[%(gameObject.id): Light]" }
    
    foreign gameObject

    foreign goAdd(gameObject)
    foreign static goGet(gameObject)
    foreign goRemove(gameObject)
    
    foreign type
    foreign type=(type)
    foreign isRSM
    foreign isRSM=(isRSM)
    foreign lightColour
    foreign lightColour=(colour)
    foreign ambientColour
    foreign ambientColour=(colour)
    foreign lightIntensity
    foreign lightIntensity=(intensity)
    foreign shadowType
    foreign shadowType=(shadowType)
    foreign renderTargets
    foreign renderTargets=(renderTargets)
    foreign depth
    foreign depth=(depth)
    foreign size
    foreign size=(size)
    foreign enabled
    foreign enabled=(enabled)
    foreign texture
    foreign texture=(texture)
    foreign innerCutOff
    foreign innerCutOff=(innerCutOff)
    foreign outerCutOff
    foreign outerCutOff=(outerCutOff)
}

class LightTypes {
    static Unknown     { 0 }
    static Directional { 1 }
    static Spot        { 2 }
    static Point       { 3 }
    static Cascade     { 4 }
}

class ShadowTypes {
    static Off          { 0 }
    static Generated    { 1 }
    static GenerateOnce { 2 }
    static Dynamic      { 3 }
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          LightHandle& handle = *make(vm);
          handle.type = WrenComponentTypes::kLight;
        },
          [](void* data) {
          LightHandle& handle = *((LightHandle*)data);
          handle.handle = components::BaseLightComponent();
          handle.type   = WrenComponentTypes::kUnknown;
          handle.entity = entity::Entity();
        }
        };
      }
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
          GameObject::make(vm, GetForeign<LightHandle>(vm)->handle.entity());
        };
        if (strcmp(signature, "goAdd(_)") == 0) return [](WrenVM* vm) {
          LightHandle* handle = GetForeign<LightHandle>(vm);
          entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          handle->handle = g_lightSystem->addComponent(e);
          handle->entity = e;
          /*Handle*/ g_scriptingData.getData(handle->handle.entity().id()).light = wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          /*Handle*/ wrenSetSlotHandle(vm, 0, g_scriptingData.getData(GetForeign<entity::Entity>(vm, 1)->id()).light);
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          LightHandle* handle = GetForeign<LightHandle>(vm);
          g_colliderSystem->removeComponent(handle->handle.entity());
          handle->handle = components::BaseLightComponent();
          handle->entity = entity::Entity();
          /*Handle*/ wrenReleaseHandle(vm, g_scriptingData.getData(handle->handle.entity()).light);
          /*Handle*/ g_scriptingData.getData(handle->handle.entity()).light = nullptr;
        };
        if (strcmp(signature, "type") == 0) return [](WrenVM* vm) {
          uint32_t type;
          switch (GetForeign<LightHandle>(vm)->handle.getLightType())
          {
          case components::LightType::kUnknown: // Unknown
            type = 0;
            break;
          case components::LightType::kDirectional: // Directional
            type = 1;
            break;
          case components::LightType::kSpot: // Spot
            type = 2;
            break;
          case components::LightType::kPoint: // Point
            type = 3;
            break;
          case components::LightType::kCascade: // Cascade
            type = 4;
            break;
          }
          wrenSetSlotDouble(vm, 0, (double)type);
        };
        if (strcmp(signature, "type=(_)") == 0) return [](WrenVM* vm) {
          components::LightType type;
          switch ((uint32_t)wrenGetSlotDouble(vm, 1))
          {
          case 0: // Unknown
            type = components::LightType::kUnknown;
            break;
          case 1: // Directional
            type = components::LightType::kDirectional;
            break;
          case 2: // Spot
            type = components::LightType::kSpot;
            break;
          case 3: // Point
            type = components::LightType::kPoint;
            break;
          case 4: // Cascade
            type = components::LightType::kCascade;
            break;
          }
          GetForeign<LightHandle>(vm)->handle.setLightType(type);
        };
        if (strcmp(signature, "isRSM") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, GetForeign<LightHandle>(vm)->handle.getRSM());
        };
        if (strcmp(signature, "isRSM=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setRSM(wrenGetSlotBool(vm, 1));
        };
        if (strcmp(signature, "lightColour") == 0) return [](WrenVM* vm) {
          Vec3::make(vm, GetForeign<LightHandle>(vm)->handle.getColour());
        };
        if (strcmp(signature, "lightColour=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setColour(*GetForeign<glm::vec3>(vm, 1));
        };
        if (strcmp(signature, "ambientColour") == 0) return [](WrenVM* vm) {
          Vec3::make(vm, GetForeign<LightHandle>(vm)->handle.getAmbient());
        };
        if (strcmp(signature, "ambientColour=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setAmbient(*GetForeign<glm::vec3>(vm, 1));
        };
        if (strcmp(signature, "lightIntensity") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)GetForeign<LightHandle>(vm)->handle.getIntensity());
        };
        if (strcmp(signature, "lightIntensity=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setIntensity((float)wrenGetSlotDouble(vm, 1));
        };
        if (strcmp(signature, "shadowType") == 0) return [](WrenVM* vm) {
          uint32_t shadow_type;
          switch (GetForeign<LightHandle>(vm)->handle.getShadowType())
          {
          case components::ShadowType::kNone:
            shadow_type = 0;
            break;
          case components::ShadowType::kGenerated:
            shadow_type = 1;
            break;
          case components::ShadowType::kGenerateOnce:
            shadow_type = 2;
            break;
          case components::ShadowType::kDynamic:
            shadow_type = 3;
            break;
          }
          wrenSetSlotDouble(vm, 0, (double)shadow_type);
        };
        if (strcmp(signature, "shadowType=(_)") == 0) return [](WrenVM* vm) {
          components::ShadowType shadow_type;
          switch ((uint32_t)wrenGetSlotDouble(vm, 1))
          {
          case 0:
            shadow_type = components::ShadowType::kNone;
            break;
          case 1:
            shadow_type = components::ShadowType::kGenerated;
            break;
          case 2:
            shadow_type = components::ShadowType::kGenerateOnce;
            break;
          case 3:
            shadow_type = components::ShadowType::kDynamic;
            break;
          }
          GetForeign<LightHandle>(vm)->handle.setShadowType(shadow_type);
        };
        if (strcmp(signature, "depth") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)GetForeign<LightHandle>(vm)->handle.getDepth());
        };
        if (strcmp(signature, "depth=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setDepth((float)wrenGetSlotDouble(vm, 1));
        };
        if (strcmp(signature, "size") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)GetForeign<LightHandle>(vm)->handle.getSize());
        };
        if (strcmp(signature, "size=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setSize((float)wrenGetSlotDouble(vm, 1));
        };
        if (strcmp(signature, "innerCutOff") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)GetForeign<LightHandle>(vm)->handle.getCutOff().asRad());
        };
        if (strcmp(signature, "innerCutOff=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setCutOff((float)wrenGetSlotDouble(vm, 1));
        };
        if (strcmp(signature, "outerCutOff") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)GetForeign<LightHandle>(vm)->handle.getOuterCutOff().asRad());
        };
        if (strcmp(signature, "outerCutOff=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setOuterCutOff((float)wrenGetSlotDouble(vm, 1));
        };
        if (strcmp(signature, "enabled") == 0) return [](WrenVM* vm) {
          wrenSetSlotBool(vm, 0, GetForeign<LightHandle>(vm)->handle.getEnabled());
        };
        if (strcmp(signature, "enabled=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setEnabled(wrenGetSlotBool(vm, 1));
        };
        if (strcmp(signature, "texture") == 0) return [](WrenVM* vm) {
          wrenSetSlotBool(vm, 0, GetForeign<LightHandle>(vm)->handle.getTexture());
        };
        if (strcmp(signature, "texture=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setTexture(*GetForeign<asset::VioletTextureHandle>(vm, 1));
        };
        if (strcmp(signature, "renderTargets") == 0) return [](WrenVM* vm) {
          
          Vector<platform::RenderTarget> render_targets = GetForeign<LightHandle>(vm)->handle.getRenderTarget();
          wrenSetSlotNewList(vm, 0);

          for (uint32_t i = 0u; i < render_targets.size(); ++i)
          {
            String str = render_targets[i].getName().getName();
            const char* c_str = (const char*)foundation::Memory::allocate(str.size() + 1u);
            memcpy((void*)c_str, str.data(), str.size() + 1u);
            wrenSetSlotString(vm, 1, c_str);
            wrenInsertInList(vm, 0, -1, 1);
          }
        };
        if (strcmp(signature, "renderTargets=(_)") == 0) return [](WrenVM* vm) {
          Vector<platform::RenderTarget> render_targets(wrenGetListCount(vm, 1));

          for (uint32_t i = 0u; i < render_targets.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            render_targets[i] = g_world->getPostProcessManager().getTarget(Name(wrenGetSlotString(vm, 2)));
          }

          GetForeign<LightHandle>(vm)->handle.setRenderTarget(render_targets);
        };
        return nullptr;
      }
    }

    namespace MonoBehaviour
    {
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////////////////////////
///// mono_behaviour.wren /////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class MonoBehaviour {
    construct new() {
    }
    toString { "[%(gameObject.id): MonoBehaviour]" }
    
    gameObject { _game_object }
    transform { _transform }

    goAdd(gameObject) {
        _game_object = gameObject
        _transform = gameObject.transform
        goAddPrivate(gameObject)
        initialize()
    }

    goRemove(gameObject) {
        deinitialize()
        goRemovePrivate(gameObject)
    }
    
    initialize() {}
    deinitialize() {}

    update() {}
    fixedUpdate() {}

    onCollisionEnter(other) {}
    onCollisionStay(other) {}
    onCollisionExit(other) {}

    onTriggerEnter(other) {}
    onTriggerStay(other) {}
    onTriggerExit(other) {}

    foreign goAddPrivate(gameObject)
    foreign static goGet(gameObject)
    foreign goRemovePrivate(gameObject)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }

      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "goAddPrivate(_)") == 0) return [](WrenVM* vm) {
          entity::Entity entity = *GetForeign<entity::Entity>(vm, 1);
          g_monoBehaviourSystem->addComponent(entity);
          /*Handle*/ g_scriptingData.getData(entity.id()).mono_behaviour = wrenGetSlotHandle(vm, 0);


          g_monoBehaviourSystem->setObject          (entity, wrenGetSlotHandle (vm, 0));
          g_monoBehaviourSystem->setInitialize      (entity, wrenMakeCallHandle(vm, "initialize()"));
          g_monoBehaviourSystem->setDeinitialize    (entity, wrenMakeCallHandle(vm, "deinitialize()"));
          g_monoBehaviourSystem->setUpdate          (entity, wrenMakeCallHandle(vm, "update()"));
          g_monoBehaviourSystem->setFixedUpdate     (entity, wrenMakeCallHandle(vm, "fixedUpdate()"));
          g_monoBehaviourSystem->setOnCollisionEnter(entity, wrenMakeCallHandle(vm, "onCollisionEnter(_)"));
          g_monoBehaviourSystem->setOnCollisionEnter(entity, wrenMakeCallHandle(vm, "onCollisionStay(_)"));
          g_monoBehaviourSystem->setOnCollisionExit (entity, wrenMakeCallHandle(vm, "onCollisionExit(_)"));
          g_monoBehaviourSystem->setOnTriggerEnter  (entity, wrenMakeCallHandle(vm, "onTriggerEnter(_)"));
          g_monoBehaviourSystem->setOnTriggerEnter  (entity, wrenMakeCallHandle(vm, "onTriggerStay(_)"));
          g_monoBehaviourSystem->setOnTriggerExit   (entity, wrenMakeCallHandle(vm, "onTriggerExit(_)"));
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          /*Handle*/ wrenSetSlotHandle(vm, 0, g_scriptingData.getData(GetForeign<entity::Entity>(vm, 1)->id()).mono_behaviour);
        };
        if (strcmp(signature, "goRemovePrivate(_)") == 0) return [](WrenVM* vm) {
          entity::Entity entity = *GetForeign<entity::Entity>(vm, 1);
#define FREE(x) wrenReleaseHandle(vm, (WrenHandle*)g_monoBehaviourSystem->get##x(entity)); g_monoBehaviourSystem->set##x(entity, nullptr);

          FREE(Object);
          FREE(Initialize);
          FREE(Deinitialize);
          FREE(Update);
          FREE(FixedUpdate);
          FREE(OnCollisionEnter);
          FREE(OnCollisionExit);
          FREE(OnTriggerEnter);
          FREE(OnTriggerExit);

          /*Handle*/ wrenReleaseHandle(vm, g_scriptingData.getData(entity.id()).mono_behaviour);
          /*Handle*/ g_scriptingData.getData(entity.id()).mono_behaviour = nullptr;
        };
        return nullptr;
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    namespace Graphics
    {
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////////////////////////
///// graphics.wren ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
class Graphics {
    foreign static setVSync(enabled)
    foreign static getVSync()
    foreign static setRenderScale(scale)
    foreign static getRenderScale()
    foreign static setDirectionalShaders(generate, modify, publish)
    foreign static setSpotLightShaders(generate, modify, publish)
    foreign static setPointLightShaders(generate, modify, publish)
    foreign static setCascadeShaders(generate, modify, publish)
    foreign static setDirectionalShadersRSM(generate, modify, publish)
    foreign static setSpotLightShadersRSM(generate, modify, publish)
    foreign static setPointLightShadersRSM(generate, modify, publish)
    foreign static setCascadeShadersRSM(generate, modify, publish)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "setVSync(_)") == 0) return [](WrenVM* vm) {
          g_world->getRenderer()->setVSync(wrenGetSlotBool(vm, 1));
        };
        if (strcmp(signature, "getVSync()") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, g_world->getRenderer()->getVSync() ? 1.0 : 0.0);
        };
        if (strcmp(signature, "setRenderScale(_)") == 0) return [](WrenVM* vm) {
          g_world->getRenderer()->setRenderScale((float)wrenGetSlotDouble(vm, 1));
        };
        if (strcmp(signature, "getRenderScale()") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (float)g_world->getRenderer()->getRenderScale());
        };
        if (strcmp(signature, "setDirectionalShaders(_,_,_)") == 0) return [](WrenVM* vm) {
          const asset::ShaderHandle& generate = *GetForeign<asset::ShaderHandle>(vm, 1);
          Vector<asset::ShaderHandle> modify(wrenGetListCount(vm, 2));
          const asset::ShaderHandle& publish = *GetForeign<asset::ShaderHandle>(vm, 3);
          for (int i = 0; i < (int)modify.size(); ++i)
          {
            wrenGetListElement(vm, 2, i, 0);
            modify[i] = *GetForeign<asset::ShaderHandle>(vm, 0);
          }

          g_lightSystem->setShadersDirectional(generate, modify, publish);
        };
        if (strcmp(signature, "setSpotLightShaders(_,_,_)") == 0) return [](WrenVM* vm) {
          const asset::ShaderHandle& generate = *GetForeign<asset::ShaderHandle>(vm, 1);
          Vector<asset::ShaderHandle> modify(wrenGetListCount(vm, 2));
          const asset::ShaderHandle& publish = *GetForeign<asset::ShaderHandle>(vm, 3);
          for (int i = 0; i < (int)modify.size(); ++i)
          {
            wrenGetListElement(vm, 2, i, 0);
            modify[i] = *GetForeign<asset::ShaderHandle>(vm, 1);
          }

          g_lightSystem->setShadersSpot(generate, modify, publish);
        };
        if (strcmp(signature, "setPointLightShaders(_,_,_)") == 0) return [](WrenVM* vm) {
          const asset::ShaderHandle& generate = *GetForeign<asset::ShaderHandle>(vm, 1);
          Vector<asset::ShaderHandle> modify(wrenGetListCount(vm, 2));
          const asset::ShaderHandle& publish = *GetForeign<asset::ShaderHandle>(vm, 3);
          for (int i = 0; i < (int)modify.size(); ++i)
          {
            wrenGetListElement(vm, 2, i, 0);
            modify[i] = *GetForeign<asset::ShaderHandle>(vm, 1);
          }

          g_lightSystem->setShadersPoint(generate, modify, publish);
        };
        if (strcmp(signature, "setCascadeShaders(_,_,_)") == 0) return [](WrenVM* vm) {
          const asset::ShaderHandle& generate = *GetForeign<asset::ShaderHandle>(vm, 1);
          Vector<asset::ShaderHandle> modify(wrenGetListCount(vm, 2));
          const asset::ShaderHandle& publish = *GetForeign<asset::ShaderHandle>(vm, 3);
          for (int i = 0; i < (int)modify.size(); ++i)
          {
            wrenGetListElement(vm, 2, i, 0);
            modify[i] = *GetForeign<asset::ShaderHandle>(vm, 1);
          }

          g_lightSystem->setShadersCascade(generate, modify, publish);
        };
        if (strcmp(signature, "setDirectionalShadersRSM(_,_,_)") == 0) return [](WrenVM* vm) {
          const asset::ShaderHandle& generate = *GetForeign<asset::ShaderHandle>(vm, 1);
          Vector<asset::ShaderHandle> modify(wrenGetListCount(vm, 2));
          const asset::ShaderHandle& publish = *GetForeign<asset::ShaderHandle>(vm, 3);
          for (int i = 0; i < (int)modify.size(); ++i)
          {
            wrenGetListElement(vm, 2, i, 0);
            modify[i] = *GetForeign<asset::ShaderHandle>(vm, 1);
          }

          g_lightSystem->setShadersDirectionalRSM(generate, modify, publish);
        };
        if (strcmp(signature, "setSpotLightShadersRSM(_,_,_)") == 0) return [](WrenVM* vm) {
          const asset::ShaderHandle& generate = *GetForeign<asset::ShaderHandle>(vm, 1);
          Vector<asset::ShaderHandle> modify(wrenGetListCount(vm, 2));
          const asset::ShaderHandle& publish = *GetForeign<asset::ShaderHandle>(vm, 3);
          for (int i = 0; i < (int)modify.size(); ++i)
          {
            wrenGetListElement(vm, 2, i, 0);
            modify[i] = *GetForeign<asset::ShaderHandle>(vm, 1);
          }

          g_lightSystem->setShadersSpotRSM(generate, modify, publish);
        };
        if (strcmp(signature, "setPointLightShadersRSM(_,_,_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        if (strcmp(signature, "setCascadeShadersRSM(_,_,_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        return nullptr;
      }
    }
    namespace PostProcess
    {
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////////////////////////
///// postProcess.wren ////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
class PostProcess {
    foreign static addRenderTarget(name, render_scale, format)
    foreign static addRenderTarget(name, width, height, format)
    foreign static addRenderTarget(name, texture)
    foreign static setRenderTargetFlag(name, flag, value)
    foreign static setFinalRenderTarget(name)
    foreign static addShaderPass(name, shader, input, output)
    foreign static setShaderPassEnabled(name, enabled)
    foreign static setShaderVariableFloat1(name, value)
    foreign static setShaderVariableFloat2(name, value)
    foreign static setShaderVariableFloat3(name, value)
    foreign static setShaderVariableFloat4(name, value)
    foreign static irradianceConvolution(input, output)
    foreign static hammerhead(input, output)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "addRenderTarget(_,_,_,_)") == 0) return [](WrenVM* vm) {
          auto handle = asset::TextureManager::getInstance()->create(Name(wrenGetSlotString(vm, 1)), (uint32_t)wrenGetSlotDouble(vm, 2), (uint32_t)wrenGetSlotDouble(vm, 3), 1u, (TextureFormat)(uint32_t)wrenGetSlotDouble(vm, 4), kTextureFlagIsRenderTarget);
          g_world->getPostProcessManager().addTarget(platform::RenderTarget(Name(wrenGetSlotString(vm, 1)), handle));
        };
        if (strcmp(signature, "addRenderTarget(_,_,_)") == 0) return [](WrenVM* vm) {
          g_world->getPostProcessManager().addTarget(
            platform::RenderTarget(Name(wrenGetSlotString(vm, 1)), (float)wrenGetSlotDouble(vm, 2), (TextureFormat)(uint32_t)wrenGetSlotDouble(vm, 3))
          );
        };
        if (strcmp(signature, "addRenderTarget(_,_,_)") == 0) return [](WrenVM* vm) {
          g_world->getPostProcessManager().addTarget(
            platform::RenderTarget(Name(wrenGetSlotString(vm, 1)), (float)wrenGetSlotDouble(vm, 2), (TextureFormat)(uint32_t)wrenGetSlotDouble(vm, 3))
          );
        };
        if (strcmp(signature, "addRenderTarget(_,_)") == 0) return [](WrenVM* vm) {
          g_world->getPostProcessManager().addTarget(
            platform::RenderTarget(Name(wrenGetSlotString(vm, 1)), 0.0f, *GetForeign<asset::VioletTextureHandle>(vm, 2), true)
          );
        };
        if (strcmp(signature, "setRenderTargetFlag(_,_,_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, "NOT YET IMPLEMENTED");
        };
        if (strcmp(signature, "setFinalRenderTarget(_)") == 0) return [](WrenVM* vm) {
          g_world->getPostProcessManager().setFinalTarget(Name(wrenGetSlotString(vm, 1)));
        };
        if (strcmp(signature, "addShaderPass(_,_,_,_)") == 0) return [](WrenVM* vm) {
          Name name(wrenGetSlotString(vm, 1));
          asset::ShaderHandle shader = *GetForeign<asset::ShaderHandle>(vm, 2);
          Vector<platform::RenderTarget> input(wrenGetListCount(vm, 3));
          Vector<platform::RenderTarget> output(wrenGetListCount(vm, 4));
          for (int i = 0; i < (int)input.size(); ++i)
          {
            wrenGetListElement(vm, 3, i, 0);
            const char* target_cstr = wrenGetSlotString(vm, 0);
            Name target_name = Name(target_cstr);
            const auto& target = g_world->getPostProcessManager().getTarget(target_name);
            input.at(i) = target;
          }
          for (int i = 0; i < (int)output.size(); ++i)
          {
            wrenGetListElement(vm, 4, i, 0);
            output.at(i) = g_world->getPostProcessManager().getTarget(Name(wrenGetSlotString(vm, 0)));
          }

          g_world->getPostProcessManager().addPass(platform::ShaderPass(name, shader, input, output));
        };
        if (strcmp(signature, "setShaderPassEnabled(_,_)") == 0) return [](WrenVM* vm) {
          Name name(wrenGetSlotString(vm, 1));
          for (auto& pass : g_world->getPostProcessManager().getPasses())
          {
            if (pass.getName() == name)
            {
              pass.setEnabled(wrenGetSlotBool(vm, 2));
              return;
            }
          }
        };
        if (strcmp(signature, "setShaderVariableFloat1(_,_)") == 0) return [](WrenVM* vm) {
          g_world->getShaderVariableManager().setVariable(platform::ShaderVariable(Name(wrenGetSlotString(vm, 1)), (float)wrenGetSlotDouble(vm, 2)));
        };
        if (strcmp(signature, "setShaderVariableFloat2(_,_)") == 0) return [](WrenVM* vm) {
          g_world->getShaderVariableManager().setVariable(platform::ShaderVariable(Name(wrenGetSlotString(vm, 1)), *GetForeign<glm::vec2>(vm, 2)));
        };
        if (strcmp(signature, "setShaderVariableFloat3(_,_)") == 0) return [](WrenVM* vm) {
          g_world->getShaderVariableManager().setVariable(platform::ShaderVariable(Name(wrenGetSlotString(vm, 1)), *GetForeign<glm::vec3>(vm, 2)));
        };
        if (strcmp(signature, "setShaderVariableFloat4(_,_)") == 0) return [](WrenVM* vm) {
          g_world->getShaderVariableManager().setVariable(platform::ShaderVariable(Name(wrenGetSlotString(vm, 1)), *GetForeign<glm::vec4>(vm, 2)));
        };
        if (strcmp(signature, "irradianceConvolution(_,_)") == 0) return [](WrenVM* vm) {
          String input = wrenGetSlotString(vm, 1);
          String output = wrenGetSlotString(vm, 2);

          platform::RenderTarget& rt_input = g_world->getPostProcessManager().getTarget(input);
          asset::ShaderHandle shader = asset::AssetManager::getInstance().createAsset(Name(output + "_IrradianceConvolution"),
            foundation::Memory::constructShared<asset::Shader>(
              io::ShaderIO::asAsset(io::ShaderIO::load("resources/shaders/irradiance_convolution.fx"))
              )
          );

          float as = (float)rt_input.getTexture()->getLayer(0u).getHeight() / (float)rt_input.getTexture()->getLayer(0u).getWidth();

          VioletTexture violet_texture;
          violet_texture.width = 512u;
          violet_texture.height = (uint32_t)(512.0f * as);
          violet_texture.mip_count = 1u;
          violet_texture.file = output;
          violet_texture.flags = kTextureFlagIsRenderTarget;
          violet_texture.format = TextureFormat::kR16G16B16A16;
          violet_texture.hash = hash(output);
          asset::VioletTextureHandle texture = asset::TextureManager::getInstance()->create(output, violet_texture);

          auto mesh = asset::AssetManager::getInstance().createAsset<asset::Mesh>(Name("__irradiance_convolution_mesh__"),
            foundation::Memory::constructShared<asset::Mesh>(asset::Mesh::createScreenQuad())
            );
          g_world->getRenderer()->setMesh(mesh);
          g_world->getRenderer()->setSubMesh(0u);

          platform::RenderTarget rt_out(Name("IrradianceConvolution_Out"), texture);

          platform::ShaderPass shader_pass(
            Name("IrradianceConvolution"),
            shader,
            { rt_input },
            { rt_out }
          );

          g_world->getRenderer()->bindShaderPass(shader_pass);
          g_world->getRenderer()->draw();

          g_world->getPostProcessManager().addTarget(platform::RenderTarget(Name(output), texture));
        };
        if (strcmp(signature, "hammerhead(_,_)") == 0) return [](WrenVM* vm) {
          String input = wrenGetSlotString(vm, 1);
          String output = wrenGetSlotString(vm, 2);

          platform::RenderTarget& rt_input = g_world->getPostProcessManager().getTarget(input);
          asset::ShaderHandle shader = asset::AssetManager::getInstance().createAsset(Name(output + "_Hammerhead"),
            foundation::Memory::constructShared<asset::Shader>(
              io::ShaderIO::asAsset(io::ShaderIO::load("resources/shaders/hammerhead.fx"))
              )
          );

          float as = (float)rt_input.getTexture()->getLayer(0u).getHeight() / (float)rt_input.getTexture()->getLayer(0u).getWidth();
          VioletTexture violet_texture;
          violet_texture.width = 512u;
          violet_texture.height = (uint32_t)(512.0f * as);
          violet_texture.mip_count = 1u + (uint16_t)floor(log10((float)std::max(violet_texture.width, violet_texture.height)) / log10(2.0f));
          violet_texture.file = output;
          violet_texture.flags = kTextureFlagIsRenderTarget;
          violet_texture.format = TextureFormat::kR16G16B16A16;
          violet_texture.hash = hash(output);
          asset::VioletTextureHandle texture = asset::TextureManager::getInstance()->create(output, violet_texture);

          auto mesh = asset::AssetManager::getInstance().createAsset<asset::Mesh>(Name("__hammerhead_mesh__"),
            foundation::Memory::constructShared<asset::Mesh>(asset::Mesh::createScreenQuad())
            );
          g_world->getRenderer()->setMesh(mesh);
          g_world->getRenderer()->setSubMesh(0u);

          platform::RenderTarget rt_in(Name("Hammerhead_In"), texture);
          for (uint32_t i = 0u; i < violet_texture.mip_count; ++i)
          {
            g_world->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("roughness"), ((float)i / (float)violet_texture.mip_count)));
            platform::RenderTarget rt_out(Name("Hammerhead_Out_" + toString(i)), texture);
            rt_out.setMipMap(i);

            platform::ShaderPass shader_pass(
              Name("Hammerhead_" + toString(i)),
              shader,
              { rt_input },
              //{ i == 0u ? rt_input : rt_in },
              { rt_out }
            );

            g_world->getRenderer()->bindShaderPass(shader_pass);
            g_world->getRenderer()->draw();
          }

          g_world->getPostProcessManager().addTarget(platform::RenderTarget(Name(output), texture));
        };
        return nullptr;
      }
    }
    namespace Input
    {
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////////////////////////
///// input.wren //////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
class Input {
    foreign static getKey(key)
    foreign static getAxis(axis)
    foreign static getButton(button)
}

class Axes {
    static LeftStickX   { 0  }
    static LeftStickY   { 1  }
    static RightStickX  { 2  }
    static RightStickY  { 3  }
    static LeftTrigger  { 4  }
    static RightTrigger { 5  }
    
    static MouseX       { 20 }
    static MouseY       { 21 }
    static Scroll       { 22 }
}

class Buttons {
    static A                 { 0  }
    static B                 { 1  }
    static X                 { 2  }
    static Y                 { 3  }
    static Up                { 4  }
    static Down              { 5  }
    static Left              { 6  }
    static Right             { 7  }
    static LeftBumper        { 8  }
    static RightBumper       { 9  }
    static LeftStick         { 0  }
    static RightStick        { 11 }
    static Start             { 12 }
    static Back              { 13 }

    static LMB               { 20 }
    static LeftMouseButton   { 20 }
    static MMB               { 21 }
    static MiddleMouseButton { 21 }
    static RMB               { 22 }
    static RightMouseButton  { 22 }
    static LMBD              { 23 }
    static MMBD              { 24 }
    static RMBD              { 25 }
}

class Keys {
    static A            { 65   }
    static B            { 66   }
    static C            { 67   }
    static D            { 68   }
    static E            { 69   }
    static F            { 70   }
    static G            { 71   }
    static H            { 72   }
    static I            { 73   }
    static J            { 74   }
    static K            { 75   }
    static L            { 76   }
    static M            { 77   }
    static N            { 78   }
    static O            { 79   }
    static P            { 80   }
    static Q            { 81   }
    static R            { 82   }
    static S            { 83   }
    static T            { 84   }
    static U            { 85   }
    static V            { 86   }
    static W            { 87   }
    static X            { 88   }
    static Y            { 89   }
    static Z            { 90   }

    static Tilde        { 126  }

    static Num0         { 48   }
    static Num1         { 49   }
    static Num2         { 50   }
    static Num3         { 51   }
    static Num4         { 52   }
    static Num5         { 53   }
    static Num6         { 54   }
    static Num7         { 55   }
    static Num8         { 56   }
    static Num9         { 57   }

    static F1           { 0x70 }
    static F2           { 0x71 }
    static F3           { 0x72 }
    static F4           { 0x73 }
    static F5           { 0x74 }
    static F6           { 0x75 }
    static F7           { 0x76 }
    static F8           { 0x77 }
    static F9           { 0x78 }
    static F10          { 0x79 }
    static F11          { 0x7A }
    static F12          { 0x7B }
    static F13          { 0x7C }
    static F14          { 0x7D }
    static F15          { 0x7E }
    static F16          { 0x7F }
    static F17          { 0x80 }
    static F18          { 0x81 }
    static F19          { 0x82 }
    static F20          { 0x83 }
    static F21          { 0x84 }
    static F22          { 0x85 }
    static F23          { 0x86 }
    static F24          { 0x87 }
    static F25          { 0x88 }

    static Numpad0      { 0x60 }
    static Numpad1      { 0x61 }
    static Numpad2      { 0x62 }
    static Numpad3      { 0x63 }
    static Numpad4      { 0x64 }
    static Numpad5      { 0x65 }
    static Numpad6      { 0x66 }
    static Numpad7      { 0x67 }
    static Numpad8      { 0x68 }
    static Numpad9      { 0x69 }

    static Tab          { 0x09 }
    static Backspace    { 0x08 }
    static Insert       { 0x2D }
    static Delete       { 0x2E }
    static Return       { 0x0D }
    static Enter        { 0x0D }
    static Space        { 0x20 }
    static Minus        { 0xBD }
    static Shift        { 0x10 }
    static LShift       { 0xA0 }
    static RShift       { 0xA1 }
    static Control      { 0x11 }
    static LControl     { 0xA2 }
    static RControl     { 0xA3 }

    static Left         { 0x25 }
    static Up           { 0x26 }
    static Right        { 0x27 }
    static Down         { 0x28 }
      
    static PageUp       { 0x21 }
    static PageDown     { 0x22 }
    static Home         { 0x24 }
    static End          { 0x23 }
    static CapsLock     { 0x14 }
    static ScrollLock   { 0x91 }
    static NumLock      { 0x90 }
    static PrintScreen  { 0x2C }
    static Pause        { 0x13 }
    static Decimal      { 0x6E }
    static Divide       { 0x6F }
    static Multiply     { 0x6A }
    static Subtract     { 0x6D }
    static Add          { 0x6B }
    static Equal        { 0xBB }
    static LeftBracket  { 219  }
    static RightBracket { 221  }
    static BackSlash    { 0xE2 }
    static Semicolon    { 186  }
    static Apostrophe   { 0xDE }
    static Grave        { 126  }
    static Comma        { 0xBC }
    static Period       { 0xBE }
    static Slash        { 191  }
    static LAlt         { 0x12 }
    static LSuper       { 0x5B }
    static RAlt         { 0x12 }
    static RSuper       { 0x5C }
    static Menu         { 0xA4 }

    static Escape       { 0x1B }
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "getKey(_)") == 0) return [](WrenVM* vm) {

          io::KeyboardKeys key = (io::KeyboardKeys)(uint32_t)wrenGetSlotDouble(vm, 1);
          wrenSetSlotBool(vm, 0, g_world->getKeyboard().getCurrentState().getKey(key));
        };
        if (strcmp(signature, "getAxis(_)") == 0) return [](WrenVM* vm) {

          uint32_t val = (uint32_t)wrenGetSlotDouble(vm, 1);
          if (val < 20u) // Controller.
          {
            io::Controller::Axes axis = (io::Controller::Axes)val;
            wrenSetSlotDouble(vm, 0, (double)g_world->getControllerManager().getController(0u).getAxis(axis));
          }
          else // Mouse.
          {
            io::MouseAxes axis = (io::MouseAxes)(val - 20u);
            wrenSetSlotDouble(vm, 0, (double)g_world->getMouse().getCurrentState().getAxis(axis));
          }
        };
        if (strcmp(signature, "getButton(_)") == 0) return [](WrenVM* vm) {

          uint32_t val = (uint32_t)wrenGetSlotDouble(vm, 1);
          if (val < 20u) // Controller.
          {
            io::Controller::Buttons button = (io::Controller::Buttons)val;
            wrenSetSlotBool(vm, 0, g_world->getControllerManager().getController(0u).getButton(button));
          }
          else // Mouse.
          {
            io::MouseButtons button = (io::MouseButtons)(val - 20u);
            wrenSetSlotBool(vm, 0, g_world->getMouse().getCurrentState().getButton(button));
          }
        };
        return nullptr;
      }
    }
    namespace Math
    {
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////////////////////////
///// math.wren ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
class Math {
    foreign static random(min, max)
    foreign static random(max)
    foreign static random()
    
    foreign static clamp(val, min, max)
    foreign static clamp(val)

    foreign static wrap(val, min, max)
    foreign static wrapMax(val, max)

    foreign static lerp(lhs, rhs, val)

    foreign static sqrt(val)
    foreign static sqr(val)
    foreign static pow(val, pow)

    foreign static min(lhs, rhs)
    foreign static max(lhs, rhs)

    foreign static abs(val)
    foreign static floor(val)
    foreign static ceil(val)
    foreign static round(val)

    foreign static cos(val)
    foreign static cosh(val)
    foreign static acos(val)

    foreign static sin(val)
    foreign static sinh(val)
    foreign static asin(val)

    foreign static tan(val)
    foreign static tanh(val)
    foreign static atan(val)

    foreign static atan2(y, x)

    foreign static lookRotation(forward, up)

    static pi { 3.14159265359 }
    static tau { 6.28318530718 }
    static deg2Rad { 0.0174533 }
    static degToRad { 0.0174533 }
    static rad2Deg { 57.2958 }
    static radToDeg { 57.2958 }
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "random(_,_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)utilities::randomRange((float)wrenGetSlotDouble(vm, 1), (float)wrenGetSlotDouble(vm, 2)));
        };
        if (strcmp(signature, "random(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)utilities::randomRange(0.0f, (float)wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "random()") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)utilities::random());
        };
        if (strcmp(signature, "clamp(_,_,_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::min(std::max(wrenGetSlotDouble(vm, 1), wrenGetSlotDouble(vm, 2)), wrenGetSlotDouble(vm, 3)));
        };
        if (strcmp(signature, "clamp(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::min(std::max(wrenGetSlotDouble(vm, 1), 0.0), 1.0));
        };
        if (strcmp(signature, "lerp(_,_,_)") == 0) return [](WrenVM* vm) {
          double x = wrenGetSlotDouble(vm, 1);
          double y = wrenGetSlotDouble(vm, 2);
          double s = wrenGetSlotDouble(vm, 3);
          wrenSetSlotDouble(vm, 0, x + s * (y - x));
        };
        if (strcmp(signature, "sqrt(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::sqrt(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "sqr(_)") == 0) return [](WrenVM* vm) {
          double val = wrenGetSlotDouble(vm, 1);
          wrenSetSlotDouble(vm, 0, val * val);
        };
        if (strcmp(signature, "pow(_,_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::pow(wrenGetSlotDouble(vm, 1), wrenGetSlotDouble(vm, 2)));
        };
        if (strcmp(signature, "min(_,_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::min(wrenGetSlotDouble(vm, 1), wrenGetSlotDouble(vm, 2)));
        };
        if (strcmp(signature, "max(_,_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::min(wrenGetSlotDouble(vm, 1), wrenGetSlotDouble(vm, 2)));
        };
        if (strcmp(signature, "abs(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::abs(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "floor(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::floor(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "ceil(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::ceil(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "round(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::round(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "cos(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::cos(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "cosh(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::cosh(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "acos(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::acos(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "sin(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::sin(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "sinh(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::sinh(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "asin(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::asin(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "tan(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::tan(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "tanh(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::tanh(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "atan(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::atan(wrenGetSlotDouble(vm, 1)));
        };
        if (strcmp(signature, "atan2(_,_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, std::atan2(wrenGetSlotDouble(vm, 1), wrenGetSlotDouble(vm, 2)));
        };
        if (strcmp(signature, "wrapMax(_,_)") == 0) return [](WrenVM* vm) {
          double val = wrenGetSlotDouble(vm, 1);
          double max = wrenGetSlotDouble(vm, 2);
          wrenSetSlotDouble(vm, 0, std::fmod(max + std::fmod(val, max), max));
        };
        if (strcmp(signature, "wrap(_,_,_)") == 0) return [](WrenVM* vm) {
          double val = wrenGetSlotDouble(vm, 1);
          double min = wrenGetSlotDouble(vm, 2);
          double max = wrenGetSlotDouble(vm, 3);
          max = max - min;
          val = val - min;
          wrenSetSlotDouble(vm, 0, min + std::fmod(max + std::fmod(val, max), max));
        };
        if (strcmp(signature, "lookRotation(_,_)") == 0) return [](WrenVM* vm) {
          Quat::make(vm, components::TransformSystem::lookRotation(*GetForeign<glm::vec3>(vm, 1), *GetForeign<glm::vec3>(vm, 2)));
        };
        return nullptr;
      }
    }
    namespace Time
    {
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////////////////////////
///// time.wren ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
class Time {
    static fixedDeltaTime { 0.01666666666 }
    foreign static deltaTime
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "deltaTime") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, g_world->getDeltaTime());
        };
        return nullptr;
      }
    }
    namespace File
    {
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////////////////////////
///// file.wren ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
foreign class File {
    construct Open(file) {}
    construct Open(file, writeMode) {}
    foreign Close()
    foreign Read()
    foreign Write(text)

    static ReadOnly  { 1 }
    static WriteOnly { 2 }
    static ReadWrite { 3 }
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      struct File
      {
        FILE* fp;
      };
      WrenHandle* handle = nullptr;
      File* make(WrenVM* vm, File val = File())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/File", "File", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        File* data = MakeForeign<File>(vm, 0, 1);
        memcpy(data, &val, sizeof(File));
        return data;
      }
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          String file = wrenGetSlotString(vm, 1);
          uint32_t type = (wrenGetSlotCount(vm) > 2) ? (uint32_t)wrenGetSlotDouble(vm, 2) : 1u;
          String mode = "";
          if (type == 1)
            mode = "rb";
          if (type == 2)
            mode = "wb";
          if (type == 3)
            mode = "rwb";
          make(vm)->fp = FileSystem::fopen(file, mode);
        },
          [](void* data) {
          File* file = (File*)data;
          if (file->fp != nullptr) {
            FileSystem::fclose(file->fp);
            file->fp = nullptr;
          }
        }
        };
      }
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "Close()") == 0) return [](WrenVM* vm) {
          File* file = GetForeign<File>(vm);
          LMB_ASSERT(file->fp != nullptr, "WREN: File was already closed");
          FileSystem::fclose(file->fp);
          file->fp = nullptr;
        };
        if (strcmp(signature, "Read()") == 0) return [](WrenVM* vm) {
          File* file = GetForeign<File>(vm);
          LMB_ASSERT(file->fp != nullptr, "WREN: No file was opened");
          fseek(file->fp, 0, SEEK_END);
          long ret = ftell(file->fp);
          String buffer(ret, '\0');
          fseek(file->fp, 0, SEEK_SET);
          fread((void*)buffer.data(), 1u, ret, file->fp);
          wrenSetSlotString(vm, 0, buffer.c_str());
        };
        if (strcmp(signature, "Write(_)") == 0) return [](WrenVM* vm) {
          File* file = GetForeign<File>(vm);
          LMB_ASSERT(file->fp != nullptr, "WREN: No file was opened");
          String str = wrenGetSlotString(vm, 1);
          fwrite(str.c_str(), sizeof(char), str.size(), file->fp);
        };
        return nullptr;
      }
    }
    namespace Noise
    {
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////////////////////////
///// noise.wren //////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
foreign class Noise {
    construct new() {}

    foreign seed
    foreign seed=(seed)
    foreign frequency
    foreign frequency=(frequency)
    foreign interpolation
    foreign interpolation=(interpolation)

    foreign getPerlin(id)
    foreign getPerlinFractal(id)
    foreign getCellular(id)
    foreign getCubic(id)
    foreign getCubicFractal(id)
    foreign getSimplex(id)
    foreign getSimplexFractal(id)
    foreign getValue(id)
    foreign getValueFractal(id)
    foreign getWhiteNoise(id)
}

foreign class NoiseInterpolation {
    static Linear  { 0 }
    static Hermite { 1 }
    static Quintic { 2 }
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      struct Noise
      {
        FastNoise noise;
      };
      WrenHandle* handle = nullptr;
      Noise* make(WrenVM* vm, Noise val = Noise())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core/Noise", "Noise", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        Noise* data = MakeForeign<Noise>(vm, 0, 1);
        memcpy(data, &val, sizeof(Noise));
        return data;
      }
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          make(vm);
        },
          [](void* data) {
          }
        };
      }
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "seed") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetSeed());
        };
        if (strcmp(signature, "seed=(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          noise->noise.SetSeed((int)wrenGetSlotDouble(vm, 1));
        };
        if (strcmp(signature, "frequency") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetFrequency());
        };
        if (strcmp(signature, "frequency=(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          noise->noise.SetFrequency((FN_DECIMAL)wrenGetSlotDouble(vm, 1));
        };
        if (strcmp(signature, "interpolation") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetSeed());
        };
        if (strcmp(signature, "interpolation=(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          uint32_t interpolation = (uint32_t)wrenGetSlotDouble(vm, 1);
          noise->noise.SetInterp(interpolation != 0 ? interpolation != 1 ? FastNoise::Quintic : FastNoise::Hermite : FastNoise::Linear);
        };
        if (strcmp(signature, "getPerlin(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          glm::vec2 id = *GetForeign<glm::vec2>(vm, 1);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetPerlin((FN_DECIMAL)id.x, (FN_DECIMAL)id.y));
        };
        if (strcmp(signature, "getPerlinFractal(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          glm::vec2 id = *GetForeign<glm::vec2>(vm, 1);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetPerlinFractal((FN_DECIMAL)id.x, (FN_DECIMAL)id.y));
        };
        if (strcmp(signature, "getCellular(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          glm::vec2 id = *GetForeign<glm::vec2>(vm, 1);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetCellular((FN_DECIMAL)id.x, (FN_DECIMAL)id.y));
        };
        if (strcmp(signature, "getCubic(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          glm::vec2 id = *GetForeign<glm::vec2>(vm, 1);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetCubic((FN_DECIMAL)id.x, (FN_DECIMAL)id.y));
        };
        if (strcmp(signature, "getCubicFractal(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          glm::vec2 id = *GetForeign<glm::vec2>(vm, 1);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetCubicFractal((FN_DECIMAL)id.x, (FN_DECIMAL)id.y));
        };
        if (strcmp(signature, "getSimplex(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          glm::vec2 id = *GetForeign<glm::vec2>(vm, 1);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetSimplex((FN_DECIMAL)id.x, (FN_DECIMAL)id.y));
        };
        if (strcmp(signature, "getSimplexFractal(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          glm::vec2 id = *GetForeign<glm::vec2>(vm, 1);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetSimplexFractal((FN_DECIMAL)id.x, (FN_DECIMAL)id.y));
        };
        if (strcmp(signature, "getValue(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          glm::vec2 id = *GetForeign<glm::vec2>(vm, 1);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetValue((FN_DECIMAL)id.x, (FN_DECIMAL)id.y));
        };
        if (strcmp(signature, "getValueFractal(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          glm::vec2 id = *GetForeign<glm::vec2>(vm, 1);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetValueFractal((FN_DECIMAL)id.x, (FN_DECIMAL)id.y));
        };
        if (strcmp(signature, "getWhiteNoise(_)") == 0) return [](WrenVM* vm) {
          Noise* noise = GetForeign<Noise>(vm);
          glm::vec2 id = *GetForeign<glm::vec2>(vm, 1);
          wrenSetSlotDouble(vm, 0, (double)noise->noise.GetWhiteNoise((FN_DECIMAL)id.x, (FN_DECIMAL)id.y));
        };
        return nullptr;
      }
    }
    namespace Assert
    {
      char* Load()
      {
        String str = R"(
///////////////////////////////////////////////////////////////////////////////////////////////////
///// assert.wren /////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
class Assert {
    foreign static isTrue(expr)
    foreign static isTrue(expr, msg)
    foreign static isFalse(expr)
    foreign static isFalse(expr, msg)
    foreign static throw(msg)
}
)";
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "isTrue(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(wrenGetSlotBool(vm, 1), "WREN: An assert happened!");
        };
        if (strcmp(signature, "isTrue(_,_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(wrenGetSlotBool(vm, 1), wrenGetSlotString(vm, 2));
        };
        if (strcmp(signature, "isFalse(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(!wrenGetSlotBool(vm, 1), "WREN: An assert happened!");
        };
        if (strcmp(signature, "isFalse(_,_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(!wrenGetSlotBool(vm, 1), wrenGetSlotString(vm, 2));
        };
        if (strcmp(signature, "throw(_)") == 0) return [](WrenVM* vm) {
          LMB_ASSERT(false, wrenGetSlotString(vm, 1));
        };
        return nullptr;
      }
    }

    bool hashEqual(const char* lhs, const char* rhs)
    {
      return (hash(lhs) == hash(rhs));
    }

    WrenForeignClassMethods wrenBindForeignClass(WrenVM* vm, const char* module, const char* className)
    {
      if (strstr(module, "Core") != 0)
      {
        if (hashEqual(className, "Vec2"))       return Vec2::Construct();
        if (hashEqual(className, "Vec3"))       return Vec3::Construct();
        if (hashEqual(className, "Vec4"))       return Vec4::Construct();
        if (hashEqual(className, "Quat"))       return Quat::Construct();
        if (hashEqual(className, "Texture"))    return Texture::Construct();
        if (hashEqual(className, "Shader"))     return Shader::Construct();
        if (hashEqual(className, "Wave"))       return Wave::Construct();
        if (hashEqual(className, "Mesh"))       return Mesh::Construct();
        if (hashEqual(className, "GameObject")) return GameObject::Construct();
        if (hashEqual(className, "Transform"))  return Transform::Construct();
        if (hashEqual(className, "Camera"))     return Camera::Construct();
        if (hashEqual(className, "MeshRender")) return MeshRender::Construct();
        if (hashEqual(className, "Lod"))        return LOD::Construct();
        if (hashEqual(className, "RigidBody"))  return RigidBody::Construct();
        if (hashEqual(className, "WaveSource")) return WaveSource::Construct();
        if (hashEqual(className, "Collider"))   return Collider::Construct();
        if (hashEqual(className, "Light"))      return Light::Construct();
        if (hashEqual(className, "File"))       return File::Construct();
        if (hashEqual(className, "Noise"))      return Noise::Construct();
      }

      return WrenForeignClassMethods{};
    }

    WrenForeignMethodFn wrenBindForeignMethod(WrenVM* vm, const char* module, const char* className, bool isStatic, const char* signature)
    {
      if (strstr(module, "Core") != 0)
      {
        if (hashEqual(className, "Console"))     return Console::Bind(signature);
        if (hashEqual(className, "Vec2"))        return Vec2::Bind(signature);
        if (hashEqual(className, "Vec3"))        return Vec3::Bind(signature);
        if (hashEqual(className, "Vec4"))        return Vec4::Bind(signature);
        if (hashEqual(className, "Quat"))        return Quat::Bind(signature);
        if (hashEqual(className, "Texture"))     return Texture::Bind(signature);
        if (hashEqual(className, "Shader"))      return Shader::Bind(signature);
        if (hashEqual(className, "Wave"))        return Wave::Bind(signature);
        if (hashEqual(className, "Mesh"))        return Mesh::Bind(signature);
        if (hashEqual(className, "GameObject"))  return GameObject::Bind(signature);
        if (hashEqual(className, "Transform"))   return Transform::Bind(signature);
        if (hashEqual(className, "Camera"))      return Camera::Bind(signature);
        if (hashEqual(className, "MeshRender"))  return MeshRender::Bind(signature);
        if (hashEqual(className, "Lod"))         return LOD::Bind(signature);
        if (hashEqual(className, "RigidBody"))   return RigidBody::Bind(signature);
        if (hashEqual(className, "WaveSource"))  return WaveSource::Bind(signature);
        if (hashEqual(className, "Collider"))    return Collider::Bind(signature);
        if (hashEqual(className, "Light"))       return Light::Bind(signature);
        if (hashEqual(className, "MonoBehaviour")) return MonoBehaviour::Bind(signature);
        if (hashEqual(className, "Graphics"))    return Graphics::Bind(signature);
        if (hashEqual(className, "PostProcess")) return PostProcess::Bind(signature);
        if (hashEqual(className, "Input"))       return Input::Bind(signature);
        if (hashEqual(className, "Math"))        return Math::Bind(signature);
        if (hashEqual(className, "Time"))        return Time::Bind(signature);
        if (hashEqual(className, "File"))        return File::Bind(signature);
        if (hashEqual(className, "Noise"))       return Noise::Bind(signature);
        if (hashEqual(className, "Assert"))      return Assert::Bind(signature);
      }

      return nullptr;
    }

    char* wrenLoadModule(WrenVM* vm, const char* name_cstr)
    {
      if (strstr(name_cstr, "Core") != 0)
      {
        if (hashEqual(name_cstr + 5u, "Console"))     return Console::Load();
        if (hashEqual(name_cstr + 5u, "Vec2"))        return Vec2::Load();
        if (hashEqual(name_cstr + 5u, "Vec3"))        return Vec3::Load();
        if (hashEqual(name_cstr + 5u, "Vec4"))        return Vec4::Load();
        if (hashEqual(name_cstr + 5u, "Quat"))        return Quat::Load();
        if (hashEqual(name_cstr + 5u, "Texture"))     return Texture::Load();
        if (hashEqual(name_cstr + 5u, "Shader"))      return Shader::Load();
        if (hashEqual(name_cstr + 5u, "Wave"))        return Wave::Load();
        if (hashEqual(name_cstr + 5u, "Mesh"))        return Mesh::Load();
        if (hashEqual(name_cstr + 5u, "GameObject"))  return GameObject::Load();
        if (hashEqual(name_cstr + 5u, "Transform"))   return Transform::Load();
        if (hashEqual(name_cstr + 5u, "Camera"))      return Camera::Load();
        if (hashEqual(name_cstr + 5u, "MeshRender"))  return MeshRender::Load();
        if (hashEqual(name_cstr + 5u, "Lod"))         return LOD::Load();
        if (hashEqual(name_cstr + 5u, "RigidBody"))   return RigidBody::Load();
        if (hashEqual(name_cstr + 5u, "WaveSource"))  return WaveSource::Load();
        if (hashEqual(name_cstr + 5u, "Collider"))    return Collider::Load();
        if (hashEqual(name_cstr + 5u, "Light"))       return Light::Load();
        if (hashEqual(name_cstr + 5u, "MonoBehaviour")) return MonoBehaviour::Load();
        if (hashEqual(name_cstr + 5u, "Graphics"))    return Graphics::Load();
        if (hashEqual(name_cstr + 5u, "PostProcess")) return PostProcess::Load();
        if (hashEqual(name_cstr + 5u, "Input"))       return Input::Load();
        if (hashEqual(name_cstr + 5u, "Math"))        return Math::Load();
        if (hashEqual(name_cstr + 5u, "Time"))        return Time::Load();
        if (hashEqual(name_cstr + 5u, "File"))        return File::Load();
        if (hashEqual(name_cstr + 5u, "Noise"))       return Noise::Load();
        if (hashEqual(name_cstr + 5u, "Assert"))      return Assert::Load();
      }
      else
      {
        String str = FileSystem::FileToString(String(name_cstr) + ".wren");
        char* data = (char*)foundation::Memory::allocate(str.size() + 1u);
        memcpy(data, str.data(), str.size() + 1u);
        return data;
      }

      return nullptr;
    };

    extern void WrenBind(void* config)
    {
      WrenConfiguration* configuration = (WrenConfiguration*)config;
      configuration->bindForeignClassFn  = wrenBindForeignClass;
      configuration->bindForeignMethodFn = wrenBindForeignMethod;
      configuration->loadModuleFn        = wrenLoadModule;

    }

    extern void WrenSetWorld(world::IWorld* world)
    {
      g_world               = world;
      g_entitySystem        = world->getScene().getSystem<entity::EntitySystem>().get();
      g_transformSystem     = world->getScene().getSystem<components::TransformSystem>().get();
      g_cameraSystem        = world->getScene().getSystem<components::CameraSystem>().get();
      g_lightSystem         = world->getScene().getSystem<components::LightSystem>().get();
      g_meshRenderSystem    = world->getScene().getSystem<components::MeshRenderSystem>().get();
      g_rigidBodySystem     = world->getScene().getSystem<components::RigidBodySystem>().get();
      g_waveSourceSystem    = world->getScene().getSystem<components::WaveSourceSystem>().get();
      g_colliderSystem      = world->getScene().getSystem<components::ColliderSystem>().get();
      g_monoBehaviourSystem = world->getScene().getSystem<components::MonoBehaviourSystem>().get();
    }

    extern void WrenRelease(WrenVM* vm)
    {
      wrenReleaseHandle(vm, Vec2::handle);
      wrenReleaseHandle(vm, Vec3::handle);
      wrenReleaseHandle(vm, Vec4::handle);
      wrenReleaseHandle(vm, Quat::handle);
      wrenReleaseHandle(vm, Texture::handle);
      wrenReleaseHandle(vm, Shader::handle);
      wrenReleaseHandle(vm, Wave::handle);
      wrenReleaseHandle(vm, Mesh::handle);
      wrenReleaseHandle(vm, GameObject::handle);
      wrenReleaseHandle(vm, Transform::handle);
      wrenReleaseHandle(vm, Camera::handle);
      wrenReleaseHandle(vm, MeshRender::handle);
      wrenReleaseHandle(vm, LOD::handle);
      wrenReleaseHandle(vm, RigidBody::handle);
      wrenReleaseHandle(vm, WaveSource::handle);
      wrenReleaseHandle(vm, Collider::handle);
      wrenReleaseHandle(vm, Light::handle);
      wrenReleaseHandle(vm, File::handle);
      wrenReleaseHandle(vm, Noise::handle);
      
      g_world            = nullptr;
      g_entitySystem     = nullptr;
      g_transformSystem  = nullptr;
      g_cameraSystem     = nullptr;
      g_lightSystem      = nullptr;
      g_meshRenderSystem = nullptr;
    }
  }
}
