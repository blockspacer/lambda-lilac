#include "scripting/wren/wren_binding.h"
#include <assets/texture.h>
#include <assets/mesh.h>
#include <assets/mesh_io.h>
#include <assets/shader.h>
#include <assets/shader_io.h>
#include <systems/entity_system.h>
#include <systems/name_system.h>
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
#include <gui/gui.h>

#include <memory/memory.h>
#include <memory/frame_heap.h>
#include <containers/containers.h>
#include <utils/file_system.h>
#include <utils/console.h>
#include <utils/utilities.h>
#include <utils/nav_mesh.h>

#include <wren.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <FastNoise.h>

#include <algorithm>

namespace lambda
{
  namespace scripting
  {
#define WREN_ALLOC foundation::Memory::allocate

    ///////////////////////////////////////////////////////////////////////////
    world::IWorld* g_world;
		scene::Scene* g_scene;

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
      WrenHandle* handle = nullptr;
			/////////////////////////////////////////////////////////////////////////
			glm::vec2* makeAt(WrenVM* vm, int slot, int class_slot, glm::vec2 val = glm::vec2())
			{
				if (handle == nullptr)
				{
					wrenGetVariable(vm, "Core", "Vec2", class_slot);
					handle = wrenGetSlotHandle(vm, class_slot);
				}
				wrenSetSlotHandle(vm, class_slot, handle);
				glm::vec2* data = MakeForeign<glm::vec2>(vm, slot, class_slot);
				memcpy(data, &val, sizeof(glm::vec2));
				return data;
			}

      /////////////////////////////////////////////////////////////////////////
      glm::vec2* make(WrenVM* vm, const glm::vec2 val = glm::vec2())
      {
				return makeAt(vm, 0, 1, val);
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
          const char* c_str = (const char*)WREN_ALLOC(str.size() + 1u);
          memcpy((void*)c_str, str.data(), str.size() + 1u);
          wrenSetSlotString(vm, 0, c_str);
        };
        return nullptr;
      }
    }
    namespace Vec3
    {
      WrenHandle* handle = nullptr;
			/////////////////////////////////////////////////////////////////////////
			glm::vec3* makeAt(WrenVM* vm, int slot, int class_slot, glm::vec3 val = glm::vec3())
			{
				if (handle == nullptr)
				{
					wrenGetVariable(vm, "Core", "Vec3", class_slot);
					handle = wrenGetSlotHandle(vm, class_slot);
				}
				wrenSetSlotHandle(vm, class_slot, handle);
				glm::vec3* data = MakeForeign<glm::vec3>(vm, slot, class_slot);
				memcpy(data, &val, sizeof(glm::vec3));
				return data;
			}

			/////////////////////////////////////////////////////////////////////////
			glm::vec3* make(WrenVM* vm, const glm::vec3 val = glm::vec3())
			{
				return makeAt(vm, 0, 1, val);
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
		  Vec3::make(vm, glm::cross(
			  *GetForeign<glm::vec3>(vm, 0),
			  *GetForeign<glm::vec3>(vm, 1)
		  ));
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
				(const char*)WREN_ALLOC(str.size() + 1u);
			memcpy((void*)c_str, str.data(), str.size() + 1u);
			wrenSetSlotString(vm, 0, c_str);
		};
		if (strcmp(signature, "fromString(_)") == 0) return [](WrenVM* vm) {
			String str = wrenGetSlotString(vm, 1);
			LMB_ASSERT(!str.empty() && str.front() == '[' && str.back() == ']', "WREN: Invalid string format \"%s\"", str.c_str());
			Vector<String> s = split(str.substr(1, str.size() - 2), ',');
			*MakeForeign<glm::vec3>(vm) = glm::vec3(
				std::stof(s[0].c_str()),
				std::stof(s[1].c_str()),
				std::stof(s[2].c_str())
			);
		};
        return nullptr;
      }
    }
    namespace Vec4
    {
      WrenHandle* handle = nullptr;
			/////////////////////////////////////////////////////////////////////////
			glm::vec4* makeAt(WrenVM* vm, int slot, int class_slot, glm::vec4 val = glm::vec4())
			{
				if (handle == nullptr)
				{
					wrenGetVariable(vm, "Core", "Vec4", class_slot);
					handle = wrenGetSlotHandle(vm, class_slot);
				}
				wrenSetSlotHandle(vm, class_slot, handle);
				glm::vec4* data = MakeForeign<glm::vec4>(vm, slot, class_slot);
				memcpy(data, &val, sizeof(glm::vec4));
				return data;
			}

			/////////////////////////////////////////////////////////////////////////
			glm::vec4* make(WrenVM* vm, const glm::vec4 val = glm::vec4())
			{
				return makeAt(vm, 0, 1, val);
			}

			/////////////////////////////////////////////////////////////////////////
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
            (const char*)WREN_ALLOC(str.size() + 1u);
          memcpy((void*)c_str, str.data(), str.size() + 1u);
          wrenSetSlotString(vm, 0, c_str);
        };
        return nullptr;
      }
    }
    namespace Quat
    {
      WrenHandle* handle = nullptr;
      /////////////////////////////////////////////////////////////////////////
      glm::quat* make(WrenVM* vm, const glm::quat val = glm::quat())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core", "Quat", 0);
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
					else
						make(vm, glm::quat());
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
            (const char*)WREN_ALLOC(str.size() + 1u);
          memcpy((void*)c_str, str.data(), str.size() + 1u);
          wrenSetSlotString(vm, 0, c_str);
        };
				if (strcmp(signature, "toEuler") == 0) return [](WrenVM* vm) {
					Vec3::make(vm, glm::eulerAngles(*GetForeign<glm::quat>(vm)));
				};
				if (strcmp(signature, "inverse()") == 0) return [](WrenVM* vm) {
					glm::quat& quat = *GetForeign<glm::quat>(vm);
					quat = glm::inverse(quat);
				};
				if (strcmp(signature, "inverted") == 0) return [](WrenVM* vm) {
					glm::quat& quat = *GetForeign<glm::quat>(vm);
					make(vm, glm::inverse(quat));
				};
				if (strcmp(signature, "mulVec3(_)") == 0) return [](WrenVM* vm) {
					const glm::quat& quat = *GetForeign<glm::quat>(vm);
					const glm::vec3& vec3 = *GetForeign<glm::vec3>(vm, 1);
					Vec3::make(vm, quat * vec3);
				};
				if (strcmp(signature, "mulQuat(_)") == 0) return [](WrenVM* vm) {
					const glm::quat& quat1 = *GetForeign<glm::quat>(vm);
					const glm::quat& quat2 = *GetForeign<glm::quat>(vm, 1);
					make(vm, quat1 * quat2);
				};
        return nullptr;
      }
    }
    namespace Texture
    {
      WrenHandle* handle = nullptr;
      /////////////////////////////////////////////////////////////////////////
      asset::VioletTextureHandle* make(
        WrenVM* vm, 
        asset::VioletTextureHandle val = asset::VioletTextureHandle())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core", "Texture", 0);
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
          Vector<unsigned char> bytes(wrenGetListCount(vm, 2));
		  for (int i = 0; i < (int)bytes.size(); ++i)
		  {
            wrenGetListElement(vm, 2, i, 0);
			bytes[i] = (unsigned char)wrenGetSlotDouble(vm, 0);
		  }

          const glm::vec2& size = *GetForeign<glm::vec2>(vm, 1);
          asset::VioletTextureHandle& handle = *make(vm);
          TextureFormat format = 
            (TextureFormat)(uint32_t)wrenGetSlotDouble(vm, 3);
          handle = asset::TextureManager::getInstance()->create(
            Name(toString(rand())), 
            (uint32_t)size.x,
            (uint32_t)size.y,
            1u, 
            format,
			0u,
			bytes
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
				if (strcmp(signature, "format") == 0) return [](WrenVM* vm) {
					asset::VioletTextureHandle& handle =
						*GetForeign<asset::VioletTextureHandle>(vm);
					wrenSetSlotDouble(vm, 0, (double)handle->getLayer(0u).getFormat());
				};
        return nullptr;
      }
    }
    namespace Shader
    {
      WrenHandle* handle = nullptr;
      /////////////////////////////////////////////////////////////////////////
      asset::VioletShaderHandle* make(
        WrenVM* vm, 
        asset::VioletShaderHandle val = asset::VioletShaderHandle())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core", "Shader", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        asset::VioletShaderHandle* data = MakeForeign<asset::VioletShaderHandle>(vm, 0, 1);
        memcpy(data, &val, sizeof(asset::VioletShaderHandle));
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
          asset::VioletShaderHandle& handle = *((asset::VioletShaderHandle*)data);
          handle = asset::VioletShaderHandle();
        }
        };
      }
      
      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "load(_)") == 0) return [](WrenVM* vm) {
          Name name(wrenGetSlotString(vm, 1));
          asset::VioletShaderHandle& handle = *make(vm);
		  handle = asset::ShaderManager::getInstance()->get(name.getName());
        };
        return nullptr;
      }
    }
    namespace Wave
    {
      WrenHandle* handle = nullptr;
      /////////////////////////////////////////////////////////////////////////
      asset::VioletWaveHandle* make(
        WrenVM* vm, 
        asset::VioletWaveHandle val = asset::VioletWaveHandle())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core", "Wave", 0);
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
      WrenHandle* handle = nullptr;
      /////////////////////////////////////////////////////////////////////////
      asset::VioletMeshHandle* make(
        WrenVM* vm, 
        asset::VioletMeshHandle val = asset::VioletMeshHandle())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core", "Mesh", 0);
          handle = wrenGetSlotHandle(vm, 0);
        }
        wrenSetSlotHandle(vm, 1, handle);
        asset::VioletMeshHandle* data = MakeForeign<asset::VioletMeshHandle>(vm, 0, 1);
        memcpy(data, &val, sizeof(asset::VioletMeshHandle));
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
          asset::VioletMeshHandle& handle = *((asset::VioletMeshHandle*)data);
          handle = nullptr;
        }
        };
      }

      /////////////////////////////////////////////////////////////////////////
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "load(_)") == 0) return [](WrenVM* vm) {
          Name name(wrenGetSlotString(vm, 1));
					*make(vm) = asset::MeshManager::getInstance()->get(name);
        };
				if (strcmp(signature, "generate(_)") == 0) return [](WrenVM* vm) {
					static uint32_t s_idx = 0u;
					String type = wrenGetSlotString(vm, 1);
					Name name("__generated_mesh_" + toString(s_idx++) + "__");
					asset::VioletMeshHandle& handle = *make(vm);
					if (type == "cube")
						handle = asset::MeshManager::getInstance()->create(name, asset::Mesh::createCube());
					else if (type == "cylinder")
						handle = asset::MeshManager::getInstance()->create(name, asset::Mesh::createCylinder());
					else if (type == "sphere")
						handle = asset::MeshManager::getInstance()->create(name, asset::Mesh::createSphere());
				};
				if (strcmp(signature, "generateCube(_,_)") == 0) return [](WrenVM* vm) {
					static uint32_t s_idx = 0u;
					glm::vec3 min = *GetForeign<glm::vec3>(vm, 1);
					glm::vec3 max = *GetForeign<glm::vec3>(vm, 2);
					Name name("__generated_cube_" + toString(s_idx++) + "__");
					asset::VioletMeshHandle& handle = *make(vm);
					handle = asset::MeshManager::getInstance()->create(name, asset::Mesh::createCube(min, max));
				};
				if (strcmp(signature, "create()") == 0) return [](WrenVM* vm) {
          static uint32_t s_idx = 0u;
          Name name("__created_mesh_" + toString(s_idx++) + "__");
          asset::VioletMeshHandle& handle = *make(vm);
					handle = asset::MeshManager::getInstance()->create(name);
        };
        if (strcmp(signature, "subMeshCount") == 0) return [](WrenVM* vm) {
          asset::VioletMeshHandle& handle = *GetForeign<asset::VioletMeshHandle>(vm);
          wrenSetSlotDouble(vm, 0, (double)handle->getSubMeshes().size());
        };
        if (strcmp(signature, "positions=(_)") == 0) return [](WrenVM* vm) {
          asset::VioletMeshHandle mesh = *GetForeign<asset::VioletMeshHandle>(vm);

          Vector<glm::vec3> positions(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)positions.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            positions[i] = *GetForeign<glm::vec3>(vm, 2);
          }

          mesh->set(asset::MeshElements::kPositions, positions);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kPositions].count =
            positions.size();
          sub_meshes.back().offsets[asset::MeshElements::kPositions].stride =
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
          asset::VioletMeshHandle mesh = *GetForeign<asset::VioletMeshHandle>(vm);

          Vector<glm::vec3> normals(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)normals.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            normals[i] = *GetForeign<glm::vec3>(vm, 2);
          }

          mesh->set(asset::MeshElements::kNormals, normals);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kNormals].count =
            normals.size();
          sub_meshes.back().offsets[asset::MeshElements::kNormals].stride =
            sizeof(float) * 3u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "texCoords=(_)") == 0) return [](WrenVM* vm) {
          asset::VioletMeshHandle mesh = *GetForeign<asset::VioletMeshHandle>(vm);

          Vector<glm::vec2> tex_coords(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)tex_coords.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            tex_coords[i] = *GetForeign<glm::vec2>(vm, 2);
          }

          mesh->set(asset::MeshElements::kTexCoords, tex_coords);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kTexCoords].count =
            tex_coords.size();
          sub_meshes.back().offsets[asset::MeshElements::kTexCoords].stride =
            sizeof(float) * 2u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "colours=(_)") == 0) return [](WrenVM* vm) {
          asset::VioletMeshHandle mesh = *GetForeign<asset::VioletMeshHandle>(vm);

          Vector<glm::vec4> colours(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)colours.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            colours[i] = *GetForeign<glm::vec4>(vm, 2);
          }

          mesh->set(asset::MeshElements::kColours, colours);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kColours].count =
            colours.size();
          sub_meshes.back().offsets[asset::MeshElements::kColours].stride =
            sizeof(float) * 4u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "tangents=(_)") == 0) return [](WrenVM* vm) {
          asset::VioletMeshHandle mesh = *GetForeign<asset::VioletMeshHandle>(vm);

		  Vector<glm::vec3> tangents(wrenGetListCount(vm, 1));
		  for (int i = 0; i < (int)tangents.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
			tangents[i]  = *GetForeign<glm::vec3>(vm, 2);
          }

          mesh->set(asset::MeshElements::kTangents, tangents);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kTangents].count =
            tangents.size();
          sub_meshes.back().offsets[asset::MeshElements::kTangents].stride = 
            sizeof(float) * 3u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "joints=(_)") == 0) return [](WrenVM* vm) {
          asset::VioletMeshHandle mesh = *GetForeign<asset::VioletMeshHandle>(vm);

          Vector<glm::vec4> joints(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)joints.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            joints[i] = *GetForeign<glm::vec4>(vm, 2);
          }

          mesh->set(asset::MeshElements::kJoints, joints);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kJoints].count =
            joints.size();
          sub_meshes.back().offsets[asset::MeshElements::kJoints].stride =
            sizeof(float) * 4u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "weights=(_)") == 0) return [](WrenVM* vm) {
          asset::VioletMeshHandle mesh = *GetForeign<asset::VioletMeshHandle>(vm);

          Vector<glm::vec4> weights(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)weights.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            weights[i] = *GetForeign<glm::vec4>(vm, 2);
          }

          mesh->set(asset::MeshElements::kWeights, weights);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kWeights].count =
            weights.size();
          sub_meshes.back().offsets[asset::MeshElements::kWeights].stride =
            sizeof(float) * 4u;
          mesh->setSubMeshes(sub_meshes);
        };
        if (strcmp(signature, "indices=(_)") == 0) return [](WrenVM* vm) {
          asset::VioletMeshHandle mesh = *GetForeign<asset::VioletMeshHandle>(vm);

          Vector<uint32_t> indices(wrenGetListCount(vm, 1));
          for (int i = 0; i < (int)indices.size(); ++i)
          {
            wrenGetListElement(vm, 1, i, 2);
            indices[i] = (uint32_t)wrenGetSlotDouble(vm, 2);
          }

          mesh->set(asset::MeshElements::kIndices, indices);

          Vector<asset::SubMesh> sub_meshes = mesh->getSubMeshes();
          if (sub_meshes.empty()) sub_meshes.push_back(asset::SubMesh());
          sub_meshes.back().offsets[asset::MeshElements::kIndices].count =
            indices.size();
          sub_meshes.back().offsets[asset::MeshElements::kIndices].stride =
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
          asset::VioletMeshHandle mesh = *GetForeign<asset::VioletMeshHandle>(vm);
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
          wrenGetVariable(vm, "Core", "Transform", 0);
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
          wrenGetVariable(vm, "Core", "Camera", 0);
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
          wrenGetVariable(vm, "Core", "MeshRender", 0);
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
          wrenGetVariable(vm, "Core", "Lod", 0);
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
          wrenGetVariable(vm, "Core", "RigidBody", 0);
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
          wrenGetVariable(vm, "Core", "WaveSource", 0);
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
          wrenGetVariable(vm, "Core", "Collider", 0);
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
      using LightHandle = IWrenComponent<components::LightComponent>;
      WrenHandle* handle = nullptr;
      
      /////////////////////////////////////////////////////////////////////////
      LightHandle* make(WrenVM* vm, components::LightComponent val = {})
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core", "Light", 0);
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
			ScriptingComponentData(entity::Entity entity)
				: entity(entity)
			{}

      entity::Entity entity;
      WrenHandle* transform      = nullptr;
      WrenHandle* camera         = nullptr;
      WrenHandle* mesh_render    = nullptr;
      WrenHandle* lod            = nullptr;
      WrenHandle* rigid_body     = nullptr;
      WrenHandle* wave_source    = nullptr;
      WrenHandle* collider       = nullptr;
      WrenHandle* light          = nullptr;
      WrenHandle* mono_behaviour = nullptr;
    };

		void release(entity::Entity e)
		{
			if (components::TransformSystem::hasComponent(e, *g_scene))     components::TransformSystem::removeComponent(e, *g_scene);
			if (components::MeshRenderSystem::hasComponent(e, *g_scene))    components::MeshRenderSystem::removeComponent(e, *g_scene);
			if (components::NameSystem::hasComponent(e, *g_scene))          components::NameSystem::removeComponent(e, *g_scene);
			if (components::CameraSystem::hasComponent(e, *g_scene))        components::CameraSystem::removeComponent(e, *g_scene);
			if (components::LODSystem::hasComponent(e, *g_scene))           components::LODSystem::removeComponent(e, *g_scene);
			if (components::RigidBodySystem::hasComponent(e, *g_scene))     components::RigidBodySystem::removeComponent(e, *g_scene);
			if (components::ColliderSystem::hasComponent(e, *g_scene))      components::ColliderSystem::removeComponent(e, *g_scene);
			if (components::MonoBehaviourSystem::hasComponent(e, *g_scene)) components::MonoBehaviourSystem::removeComponent(e, *g_scene);
			if (components::WaveSourceSystem::hasComponent(e, *g_scene))    components::WaveSourceSystem::removeComponent(e, *g_scene);
			if (components::LightSystem::hasComponent(e, *g_scene))					components::LightSystem::removeComponent(e, *g_scene);
			// TODO (Hilze): Free used entities.
			//g_entitySystem->destroyEntity(e);
		}

    ///////////////////////////////////////////////////////////////////////////
    struct ScriptingData
    {
      ScriptingComponentData& getData(entity::Entity id)
      {
        auto it = entity_to_data_.find(id);
        if (it == entity_to_data_.end())
        {
					if (!unused_data_entries_.empty())
					{
						uint32_t idx = unused_data_entries_.front();
						unused_data_entries_.pop();

						data_[idx] = ScriptingComponentData(id);
						data_to_entity_[idx] = id;
						entity_to_data_[id] = idx;
						return data_[idx];
					}
					else
					{
						data_.push_back(ScriptingComponentData(id));
						uint32_t idx = (uint32_t)data_.size() - 1u;
						data_to_entity_[idx] = id;
						entity_to_data_[id] = idx;
						return data_[idx];
					}
        }
        return data_[it->second];
      }

			void getAll(Vector<entity::Entity>& vec, entity::Entity e)
			{
				if (!components::TransformSystem::hasComponent(e, *g_scene))
					return;

				vec.push_back(e);
				for (auto child : components::TransformSystem::getChildren(e, *g_scene))
					getAll(vec, child);
			}

			void freeAll(WrenVM* vm)
			{
				Vector<entity::Entity> entities;
				for (const auto& it : entity_to_data_)
					entities.push_back(it.first);

				for (const auto& entity : entities)
					free(vm, entity);
			}

			void free(WrenVM* vm, entity::Entity e)
			{
				Vector<entity::Entity> entities;
				getAll(entities, e);

				for (int i = (int)entities.size() - 1; i >= 0; --i)
				{
					entity::Entity entity = entities[i];
					
					// Destroy this entity.
					release(entity);

					// Release the scripting data.
					const auto& it = entity_to_data_.find(entity);
					if (it != entity_to_data_.end())
					{
						uint32_t idx = it->second;

						{
							// Free the resources.
							auto& data = data_[idx];

#define FREE(x) if (x) wrenReleaseHandle(vm, x), x = nullptr;
							FREE(data.transform);
							FREE(data.camera);
							FREE(data.mesh_render);
							FREE(data.lod);
							FREE(data.rigid_body);
							FREE(data.wave_source);
							FREE(data.collider);
							FREE(data.light);
							FREE(data.mono_behaviour);
#undef FREE
						}

						unused_data_entries_.push(idx);
						data_to_entity_.erase(idx);
						entity_to_data_.erase(entity);
					}
				}
			}

    private:
			Vector<ScriptingComponentData> data_;
			Map<entity::Entity, uint32_t> entity_to_data_;
			Map<uint32_t, entity::Entity> data_to_entity_;
			Queue<uint32_t> unused_data_entries_;
    };

    ScriptingData* g_scriptingData = nullptr;

    ///////////////////////////////////////////////////////////////////////////
    namespace GameObject
    {
      WrenHandle* handle = nullptr;
      /////////////////////////////////////////////////////////////////////////
			entity::Entity* makeAt(WrenVM* vm, int slot, int class_slot, entity::Entity val = {})
			{
				if (handle == nullptr)
				{
					wrenGetVariable(vm, "Core", "GameObject", class_slot);
					handle = wrenGetSlotHandle(vm, class_slot);
				}
				wrenSetSlotHandle(vm, class_slot, handle);
				entity::Entity* data = MakeForeign<entity::Entity>(vm, slot, class_slot);
				memcpy(data, &val, sizeof(entity::Entity));
				return data;
			}
			entity::Entity* make(WrenVM* vm, entity::Entity val = {})
			{
				return makeAt(vm, 0, 1, val);
			}
      WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          entity::Entity& handle = *make(vm);
          handle = g_scene->entity.create();
		  components::NameSystem::addComponent(handle, *g_scene);
        },
          [](void* data) {
          entity::Entity& handle = *((entity::Entity*)data);
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
						(double)*GetForeign<entity::Entity>(vm)
					);
				};
				if (strcmp(signature, "name") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm);


					if (!components::NameSystem::hasComponent(e, *g_scene))
						components::NameSystem::addComponent(e, *g_scene);

					String name = components::NameSystem::getName(e, *g_scene);
					const size_t len = name.size() + 1;
					char* c_str = (char*)WREN_ALLOC(len);
					memcpy(c_str, name.c_str(), len);
					wrenSetSlotString(vm, 0, c_str);
				};
				if (strcmp(signature, "name=(_)") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm);

					if (!components::NameSystem::hasComponent(e, *g_scene))
						components::NameSystem::addComponent(e, *g_scene);

					String name = wrenGetSlotString(vm, 1);
					components::NameSystem::setName(e, name, *g_scene);
				};
				if (strcmp(signature, "tags") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm);

					if (!components::NameSystem::hasComponent(e, *g_scene))
						components::NameSystem::addComponent(e, *g_scene);

					Vector<String> tags = components::NameSystem::getTags(e, *g_scene);

					wrenSetSlotNewList(vm, 0);

					for (String tag : tags)
					{
						const size_t len = tag.size() + 1;
						const char* c_str = (const char*)WREN_ALLOC(len);
						memcpy((char*)c_str, tag.c_str(), len);
						wrenSetSlotString(vm, 1, c_str);
						wrenInsertInList(vm, 0, -1, 1);
					}
				};
				if (strcmp(signature, "tags=(_)") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm);

					if (!components::NameSystem::hasComponent(e, *g_scene))
						components::NameSystem::addComponent(e, *g_scene);

					Vector<String> tags((size_t)wrenGetListCount(vm, 1));

					for (uint32_t i = 0; i < tags.size(); ++i)
					{
						wrenGetListElement(vm, 1, (int)i, 2);
						tags[i] = wrenGetSlotString(vm, 2);
					}

					components::NameSystem::setTags(e, tags, *g_scene);
				};
				if (strcmp(signature, "destroy()") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm);

					components::NameSystem::removeComponent(e, *g_scene);
					g_scriptingData->free(vm, e);
				};
        return nullptr;
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace Transform
    {
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
					g_scriptingData->getData(e).transform = wrenGetSlotHandle(vm, 0);
          handle->handle = components::TransformSystem::addComponent(e, *g_scene);
          handle->entity = e;
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData->getData(
              *GetForeign<entity::Entity>(vm, 1)
            ).transform
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          TransformHandle* handle = GetForeign<TransformHandle>(vm);
          components::TransformSystem::removeComponent(handle->handle.entity(), *g_scene);
          wrenReleaseHandle(
            vm, 
            g_scriptingData->getData(handle->handle.entity()).transform
          );
          g_scriptingData->getData(handle->handle.entity()).transform = nullptr;
          handle->handle = components::TransformComponent();
          handle->entity = entity::Entity();
        };
        if (strcmp(signature, "parent") == 0) return [](WrenVM* vm) {
          GameObject::make(
            vm, 
            GetForeign<TransformHandle>(vm)->handle.getParent().entity()
          );
        };
        if (strcmp(signature, "parent=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<TransformHandle>(vm)->handle.setParent(
            components::TransformSystem::getComponent(*GetForeign<entity::Entity>(vm, 1), *g_scene)
          );
        };
        if (strcmp(signature, "children") == 0) return [](WrenVM* vm) {
		  Vector<components::TransformComponent> children = 
			  GetForeign<TransformHandle>(vm)->handle.getChildren();

		  wrenSetSlotNewList(vm, 0);
		  for (components::TransformComponent child : children)
		  {
			  GameObject::makeAt(vm, 1, 2, child.entity());
			  wrenInsertInList(vm, 0, -1, 1);
		  }
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
				if (strcmp(signature, "worldForward") == 0) return [](WrenVM* vm) {
					Vec3::make(
						vm,
						GetForeign<TransformHandle>(vm)->handle.getWorldForward()
					);
				};
				if (strcmp(signature, "worldRight") == 0) return [](WrenVM* vm) {
					Vec3::make(
						vm,
						GetForeign<TransformHandle>(vm)->handle.getWorldRight()
					);
				};
				if (strcmp(signature, "worldUp") == 0) return [](WrenVM* vm) {
					Vec3::make(
						vm,
						GetForeign<TransformHandle>(vm)->handle.getWorldUp()
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
				if (strcmp(signature, "localForward") == 0) return [](WrenVM* vm) {
					Vec3::make(
						vm,
						GetForeign<TransformHandle>(vm)->handle.getLocalForward()
					);
				};
				if (strcmp(signature, "localRight") == 0) return [](WrenVM* vm) {
					Vec3::make(
						vm,
						GetForeign<TransformHandle>(vm)->handle.getLocalRight()
					);
				};
				if (strcmp(signature, "localUp") == 0) return [](WrenVM* vm) {
					Vec3::make(
						vm,
						GetForeign<TransformHandle>(vm)->handle.getLocalUp()
					);
				};
        return nullptr;
      }
    }
    namespace Camera
    {
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
          handle->handle = components::CameraSystem::addComponent(e, *g_scene);
          handle->entity = e;
          
          g_scriptingData->getData(handle->handle.entity()).camera = 
            wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData->getData(
              *GetForeign<entity::Entity>(vm, 1)
            ).camera
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          CameraHandle* handle = GetForeign<CameraHandle>(vm);
          /*Handle*/ wrenReleaseHandle(
            vm, 
            g_scriptingData->getData(handle->handle.entity()).camera
          );
          g_scriptingData->getData(handle->handle.entity()).camera = nullptr;
		  components::CameraSystem::removeComponent(handle->handle.entity(), *g_scene);
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
          asset::VioletShaderHandle shader = *GetForeign<asset::VioletShaderHandle>(vm, 2);
          Vector<platform::RenderTarget> inputs(wrenGetListCount(vm, 3));
          Vector<platform::RenderTarget> outputs(wrenGetListCount(vm, 4));

          for (int i = 0; i < (int)inputs.size(); ++i)
          {
            wrenGetListElement(vm, 3, i, 0);
            inputs[i] = 
              g_scene->post_process_manager->getTarget(
                Name(wrenGetSlotString(vm, 0))
              );
          }
          for (int i = 0; i < (int)outputs.size(); ++i)
          {
            wrenGetListElement(vm, 4, i, 0);
            outputs[i] = 
              g_scene->post_process_manager->getTarget(
                Name(wrenGetSlotString(vm, 0))
              );
          }

          camera.addShaderPass(
            platform::ShaderPass(name, shader, inputs, outputs)
          );
        };
		if (strcmp(signature, "ndcToWorld(_)") == 0)
			return [](WrenVM* vm) 
		{
			components::CameraComponent camera = GetForeign<CameraHandle>(vm)->handle;
			glm::vec2 ndc = *GetForeign<glm::vec2>(vm, 1);
			glm::vec4 ray_clip(ndc.x, ndc.y, 1.0f, 1.0f);

			glm::mat4x4 projection_matrix = glm::inverse(camera.getProjectionMatrix());
			glm::vec4 ray_eye = projection_matrix * ray_clip;
			ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
			
			glm::mat4x4 view_matrix = glm::inverse(camera.getViewMatrix());

			glm::vec4 ray_wor4 = view_matrix * ray_eye;
			glm::vec3 ray_wor(ray_wor4.x, ray_wor4.y, ray_wor4.z);
			ray_wor = glm::normalize(ray_wor);

			Vec3::make(vm, glm::normalize(glm::vec3(ray_wor.x, ray_wor.y, ray_wor.z)));
		};
        return nullptr;
      }
    }
    namespace MeshRender
    {
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
      void MakeStaticRecursive(const entity::Entity& id)
      {
        const entity::Entity e = id;
        if (components::MeshRenderSystem::hasComponent(e, *g_scene))
          components::MeshRenderSystem::makeStatic(e, *g_scene);

        if (components::TransformSystem::hasComponent(e, *g_scene))
          for (const auto& child : components::TransformSystem::getChildren(e, *g_scene))
            MakeStaticRecursive(child);
      }
    
      /////////////////////////////////////////////////////////////////////////
      void MakeDynamicRecursive(const entity::Entity& id)
      {
        const entity::Entity e = id;
        if (components::MeshRenderSystem::hasComponent(e, *g_scene))
          components::MeshRenderSystem::makeDynamic(e, *g_scene);

        if (components::TransformSystem::hasComponent(e, *g_scene))
          for (const auto& child : components::TransformSystem::getChildren(e, *g_scene))
            MakeDynamicRecursive(child);
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
          handle->handle = components::MeshRenderSystem::addComponent(e, *g_scene);
          handle->entity = e;
          g_scriptingData->getData(handle->handle.entity()).mesh_render = 
            wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData->getData(
              *GetForeign<entity::Entity>(vm, 1)
            ).mesh_render
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          MeshRenderHandle* handle = GetForeign<MeshRenderHandle>(vm);
          components::MeshRenderSystem::removeComponent(handle->handle.entity(), *g_scene);
          wrenReleaseHandle(
            vm, 
            g_scriptingData->getData(handle->handle.entity()).mesh_render
          );
          g_scriptingData->getData(handle->handle.entity()).mesh_render = 
            nullptr;
					handle->handle = components::MeshRenderComponent();
					handle->entity = entity::Entity();
        };
        if (strcmp(signature, "attach(_)") == 0) return [](WrenVM* vm) {
          GetForeign<MeshRenderHandle>(vm)->handle.attachMesh(
            *GetForeign<asset::VioletMeshHandle>(vm, 1)
          );
        };
        if (strcmp(signature, "mesh=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<MeshRenderHandle>(vm)->handle.setMesh(
            *GetForeign<asset::VioletMeshHandle>(vm, 1)
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
				if (strcmp(signature, "metallicnessFactor=(_)") == 0) return [](WrenVM* vm) {
					GetForeign<MeshRenderHandle>(vm)->handle.setMetallicness(
						(float)wrenGetSlotDouble(vm, 1)
					);
				};
				if (strcmp(signature, "metallicnessFactor") == 0) return [](WrenVM* vm) {
					wrenSetSlotDouble(
						vm,
						0,
						(double)GetForeign<MeshRenderHandle>(vm)->handle.getMetallicness()
					);
				};
				if (strcmp(signature, "roughnessFactor=(_)") == 0) return [](WrenVM* vm) {
					GetForeign<MeshRenderHandle>(vm)->handle.setRoughness(
						(float)wrenGetSlotDouble(vm, 1)
					);
				};
				if (strcmp(signature, "roughnessFactor") == 0) return [](WrenVM* vm) {
					wrenSetSlotDouble(
						vm,
						0,
						(double)GetForeign<MeshRenderHandle>(vm)->handle.getRoughness()
					);
				};
				if (strcmp(signature, "emissivenessFactor=(_)") == 0) return [](WrenVM* vm) {
					GetForeign<MeshRenderHandle>(vm)->handle.setEmissiveness(
						*GetForeign<glm::vec3>(vm, 1)
					);
				};
				if (strcmp(signature, "emissivenessFactor") == 0) return [](WrenVM* vm) {
					Vec3::make(
						vm,
						GetForeign<MeshRenderHandle>(vm)->handle.getEmissiveness()
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
				if (strcmp(signature, "DMRA=(_)") == 0)
					return [](WrenVM* vm) {
					GetForeign<MeshRenderHandle>(vm)->handle.setDMRATexture(
						*GetForeign<asset::VioletTextureHandle>(vm, 1)
					);
				};
				if (strcmp(signature, "DMRA") == 0)
					return [](WrenVM* vm) {
					Texture::make(
						vm,
						GetForeign<MeshRenderHandle>(
							vm
							)->handle.getDMRATexture()
					);
				};
				if (strcmp(signature, "emissive=(_)") == 0)
					return [](WrenVM* vm) {
					GetForeign<MeshRenderHandle>(vm)->handle.setEmissiveTexture(
						*GetForeign<asset::VioletTextureHandle>(vm, 1)
					);
				};
				if (strcmp(signature, "emissive") == 0)
					return [](WrenVM* vm) {
					Texture::make(
						vm,
						GetForeign<MeshRenderHandle>(
							vm
							)->handle.getEmissiveTexture()
					);
				};
        if (strcmp(signature, "makeStatic()") == 0) return [](WrenVM* vm) {
          components::MeshRenderSystem::makeStatic(GetForeign<MeshRenderHandle>(vm)->handle.entity(), *g_scene);
        };
        if (strcmp(signature, "makeDynamic()") == 0) return [](WrenVM* vm) {
          components::MeshRenderSystem::makeDynamic(GetForeign<MeshRenderHandle>(vm)->handle.entity(), *g_scene);
        };
        if (strcmp(signature, "makeStaticRecursive()") == 0) 
          return [](WrenVM* vm) {
					entity::Entity uptop = 
						GetForeign<MeshRenderHandle>(vm)->handle.entity();
					while (components::TransformSystem::getParent(uptop, *g_scene) != entity::Entity())
						uptop = components::TransformSystem::getParent(uptop, *g_scene);
					
					MakeStaticRecursive(uptop);
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
          handle->handle = components::LODSystem::addComponent(e, *g_scene);
          handle->entity = e;
          g_scriptingData->getData(handle->handle.entity()).lod = 
            wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData->getData(
              *GetForeign<entity::Entity>(vm, 1)
            ).lod
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          LODHandle* handle = GetForeign<LODHandle>(vm);
		  components::LODSystem::removeComponent(handle->handle.entity(), *g_scene);
          wrenReleaseHandle(
            vm, 
            g_scriptingData->getData(handle->handle.entity()).lod
          );
          g_scriptingData->getData(handle->handle.entity()).lod = nullptr;
					handle->handle = components::LODComponent();
					handle->entity = entity::Entity();
        };
        if (strcmp(signature, "addLod(_,_)") == 0) return [](WrenVM* vm) {
          components::LOD lod;
          lod.setMesh(*GetForeign<asset::VioletMeshHandle>(vm, 1));
          lod.setDistance((float)wrenGetSlotDouble(vm, 2));
          GetForeign<LODHandle>(vm)->handle.addLOD(lod);
        };
        if (strcmp(signature, "addLodRecursive(_,_)") == 0) 
          return [](WrenVM* vm) {
          components::LOD lod;
          lod.setMesh(*GetForeign<asset::VioletMeshHandle>(vm, 1));
          lod.setDistance((float)wrenGetSlotDouble(vm, 2));

          entity::Entity e = GetForeign<LODHandle>(vm)->handle.entity();
          std::function<void(entity::Entity, const lambda::components::LOD&)> 
            addLOD =  [lod, &addLOD]
            (entity::Entity entity, const lambda::components::LOD& lod)->void {
            if (components::MeshRenderSystem::hasComponent(entity, *g_scene) &&
              components::MeshRenderSystem::getMesh(entity, *g_scene) != nullptr)
            {
              if (!components::LODSystem::hasComponent(entity, *g_scene))
				  components::LODSystem::addComponent(entity, *g_scene);

			  components::LODSystem::addLOD(entity, lod, *g_scene);
            }

            for (const auto& child : components::TransformSystem::getChildren(entity, *g_scene))
              addLOD(child, lod);
          };
        };
        return nullptr;
      }
    }
    namespace RigidBody
    {
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
        if (strcmp(signature, "goAddForeign(_)") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          components::RigidBodySystem::addComponent(e, *g_scene);
        };
        if (strcmp(signature, "priv_applyImpulse(_,_)") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
					glm::vec3 v = *GetForeign<glm::vec3>(vm, 2);
					components::RigidBodySystem::applyImpulse(e, v, *g_scene);
				};
        if (strcmp(signature, "priv_velocity(_,_)") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
					glm::vec3 v = *GetForeign<glm::vec3>(vm, 2);
					components::RigidBodySystem::setVelocity(e, v, *g_scene);
				};
        if (strcmp(signature, "priv_velocity(_)") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
					glm::vec3 v = components::RigidBodySystem::getVelocity(e, *g_scene);
					Vec3::make(vm, v);
				};
        if (strcmp(signature, "priv_angularVelocity(_,_)") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
					glm::vec3 v = *GetForeign<glm::vec3>(vm, 2);
					components::RigidBodySystem::setAngularVelocity(e, v, *g_scene);
				};
        if (strcmp(signature, "priv_angularVelocity(_)") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
					glm::vec3 v = components::RigidBodySystem::getAngularVelocity(e, *g_scene);
					Vec3::make(vm, v);
        };
        if (strcmp(signature, "priv_velocityConstraints(_,_)") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
					double v = wrenGetSlotDouble(vm, 2);
					components::RigidBodySystem::setVelocityConstraints(e, (uint8_t)v, *g_scene);
        };
        if (strcmp(signature, "priv_velocityConstraints(_)") == 0) return [](WrenVM* vm) {
					entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
					uint8_t v = components::RigidBodySystem::getVelocityConstraints(e, *g_scene);
					wrenSetSlotDouble(vm, 0, (double)v);
				};
		if (strcmp(signature, "priv_angularConstraints(_,_)") == 0) return [](WrenVM* vm) {
			entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
			double v = wrenGetSlotDouble(vm, 2);
			components::RigidBodySystem::setAngularConstraints(e, (uint8_t)v, *g_scene);
		};
		if (strcmp(signature, "priv_angularConstraints(_)") == 0) return [](WrenVM* vm) {
			entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
			uint8_t v = components::RigidBodySystem::getAngularConstraints(e, *g_scene);
			wrenSetSlotDouble(vm, 0, (double)v);
		};
		if (strcmp(signature, "priv_friction(_,_)") == 0) return [](WrenVM* vm) {
			entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
			float v = (float)wrenGetSlotDouble(vm, 2);
			components::RigidBodySystem::setFriction(e, v, *g_scene);
		};
		if (strcmp(signature, "priv_friction(_)") == 0) return [](WrenVM* vm) {
			entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
			float v = components::RigidBodySystem::getFriction(e, *g_scene);
			wrenSetSlotDouble(vm, 0, (double)v);
		};
		if (strcmp(signature, "priv_mass(_,_)") == 0) return [](WrenVM* vm) {
			entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
			float v = (float)wrenGetSlotDouble(vm, 2);
			components::RigidBodySystem::setMass(e, v, *g_scene);
		};
		if (strcmp(signature, "priv_mass(_)") == 0) return [](WrenVM* vm) {
			entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
			float v = components::RigidBodySystem::getMass(e, *g_scene);
			wrenSetSlotDouble(vm, 0, (double)v);
		};
        return nullptr;
      }
    }
    namespace WaveSource
    {
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
          handle->handle = components::WaveSourceSystem::addComponent(e, *g_scene);
          handle->entity = e;
          g_scriptingData->getData(
            handle->handle.entity()
          ).wave_source = wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          wrenSetSlotHandle(
            vm, 
            0, 
            g_scriptingData->getData(
              *GetForeign<entity::Entity>(vm, 1)
            ).wave_source
          );
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          WaveSourceHandle* handle = GetForeign<WaveSourceHandle>(vm);
          components::WaveSourceSystem::removeComponent(handle->handle.entity(), *g_scene);
          wrenReleaseHandle(
            vm, 
            g_scriptingData->getData(handle->handle.entity()).wave_source
          );
          g_scriptingData->getData(handle->handle.entity()).wave_source = 
            nullptr;
					handle->handle = components::WaveSourceComponent();
					handle->entity = entity::Entity();
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
          components::WaveSourceSystem::setListener(
            GetForeign<WaveSourceHandle>(vm)->handle.entity(), *g_scene
          );
        };
        return nullptr;
      }
    }
    namespace Collider
    {
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
			/////////////////////////////////////////////////////////////////////////
			void addMeshCollider(entity::Entity entity, asset::VioletMeshHandle mesh)
      {
        bool add_mesh_collider = false;

        if (mesh &&
          (true == components::MeshRenderSystem::hasComponent(entity, *g_scene) &&
          mesh == components::MeshRenderSystem::getMesh(entity, *g_scene) &&
          mesh->getSubMeshes().at(components::MeshRenderSystem::getSubMesh(entity, *g_scene)).offsets[asset::MeshElements::kPositions].count > 0u) &&
          (false == (components::MeshRenderSystem::getAlbedoTexture(entity, *g_scene) != nullptr &&
            components::MeshRenderSystem::getAlbedoTexture(entity, *g_scene)->getLayer(0u).containsAlpha())))
          add_mesh_collider = true;

        if (add_mesh_collider)
        {
          if (false == components::ColliderSystem::hasComponent(entity, *g_scene))
            components::ColliderSystem::addComponent(entity, *g_scene);

          components::ColliderSystem::makeMeshCollider(entity, mesh, components::MeshRenderSystem::getSubMesh(entity, *g_scene), *g_scene);
        }
        else if (true == components::ColliderSystem::hasComponent(entity, *g_scene))
          components::ColliderSystem::removeComponent(entity, *g_scene);

        for (const auto& child : components::TransformSystem::getChildren(entity, *g_scene))
          addMeshCollider(child, mesh);
      }
			/////////////////////////////////////////////////////////////////////////
			void setLayersRecursive(entity::Entity entity, uint16_t layers)
      {
        if (components::ColliderSystem::hasComponent(entity, *g_scene))
			components::ColliderSystem::setLayers(entity, layers, *g_scene);
        
		for (const auto& child : components::TransformSystem::getChildren(entity, *g_scene))
			setLayersRecursive(child, layers);
      }
			/////////////////////////////////////////////////////////////////////////
			WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
          GameObject::make(vm, GetForeign<ColliderHandle>(vm)->handle.entity());
        };
        if (strcmp(signature, "goAdd(_)") == 0) return [](WrenVM* vm) {
          ColliderHandle* handle = GetForeign<ColliderHandle>(vm);
          entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          handle->handle = components::ColliderSystem::addComponent(e, *g_scene);
          handle->entity = e;
          /*Handle*/ g_scriptingData->getData(handle->handle.entity()).collider = wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          /*Handle*/ wrenSetSlotHandle(vm, 0, g_scriptingData->getData(*GetForeign<entity::Entity>(vm, 1)).collider);
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
					ColliderHandle* handle = GetForeign<ColliderHandle>(vm);
					components::ColliderSystem::removeComponent(handle->entity, *g_scene);
					/*Handle*/ wrenReleaseHandle(vm, g_scriptingData->getData(handle->handle.entity()).collider);
					/*Handle*/ g_scriptingData->getData(handle->handle.entity()).collider = nullptr;
					handle->handle = components::ColliderComponent();
					handle->entity = entity::Entity();
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
          GetForeign<ColliderHandle>(vm)->handle.makeMeshCollider(*GetForeign<asset::VioletMeshHandle>(vm, 1), (uint32_t)wrenGetSlotDouble(vm, 2));
        };
        if (strcmp(signature, "makeMeshColliderRecursive(_)") == 0) return [](WrenVM* vm) {
          addMeshCollider(GetForeign<ColliderHandle>(vm)->handle.entity(), *GetForeign<asset::VioletMeshHandle>(vm, 1));
        };
        if (strcmp(signature, "layers") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)GetForeign<ColliderHandle>(vm)->handle.getLayers());
        };
        if (strcmp(signature, "layers=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<ColliderHandle>(vm)->handle.setLayers((uint16_t)wrenGetSlotDouble(vm, 1));
        };
        if (strcmp(signature, "layersRecursive=(_)") == 0) return [](WrenVM* vm) {
          setLayersRecursive(GetForeign<ColliderHandle>(vm)->entity, (uint16_t)wrenGetSlotDouble(vm, 1));
        };
        return nullptr;
      }
    }
    namespace Light
    {
			/////////////////////////////////////////////////////////////////////////
			WrenForeignClassMethods Construct()
      {
        return WrenForeignClassMethods{
          [](WrenVM* vm) {
          LightHandle& handle = *make(vm);
          handle.type = WrenComponentTypes::kLight;
        },
          [](void* data) {
          LightHandle& handle = *((LightHandle*)data);
          handle.handle = components::LightComponent();
          handle.type   = WrenComponentTypes::kUnknown;
          handle.entity = entity::Entity();
        }
        };
      }
			/////////////////////////////////////////////////////////////////////////
			WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
          GameObject::make(vm, GetForeign<LightHandle>(vm)->handle.entity());
        };
        if (strcmp(signature, "goAdd(_)") == 0) return [](WrenVM* vm) {
          LightHandle* handle = GetForeign<LightHandle>(vm);
          entity::Entity e = *GetForeign<entity::Entity>(vm, 1);
          handle->handle = components::LightSystem::addComponent(e, *g_scene);
          handle->entity = e;
          /*Handle*/ g_scriptingData->getData(handle->handle.entity()).light = wrenGetSlotHandle(vm, 0);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          /*Handle*/ wrenSetSlotHandle(vm, 0, g_scriptingData->getData(*GetForeign<entity::Entity>(vm, 1)).light);
        };
        if (strcmp(signature, "goRemove(_)") == 0) return [](WrenVM* vm) {
          LightHandle* handle = GetForeign<LightHandle>(vm);
          components::LightSystem::removeComponent(handle->handle.entity(), *g_scene);
          /*Handle*/ wrenReleaseHandle(vm, g_scriptingData->getData(handle->handle.entity()).light);
          /*Handle*/ g_scriptingData->getData(handle->handle.entity()).light = nullptr;
					handle->handle = components::LightComponent();
					handle->entity = entity::Entity();
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
        if (strcmp(signature, "shadowMapSizePx") == 0) return [](WrenVM* vm) {
          wrenSetSlotDouble(vm, 0, (double)GetForeign<LightHandle>(vm)->handle.getShadowMapSizePx());
        };
        if (strcmp(signature, "shadowMapSizePx=(_)") == 0) return [](WrenVM* vm) {
          GetForeign<LightHandle>(vm)->handle.setShadowMapSizePx((uint32_t)wrenGetSlotDouble(vm, 1));
        };
        return nullptr;
      }
    }

    namespace MonoBehaviour
    {
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "goAddPrivate(_)") == 0) return [](WrenVM* vm) {
          entity::Entity entity = *GetForeign<entity::Entity>(vm, 1);
          components::MonoBehaviourSystem::addComponent(entity, *g_scene);
          /*Handle*/ g_scriptingData->getData(entity).mono_behaviour = wrenGetSlotHandle(vm, 0);


          components::MonoBehaviourSystem::setObject          (entity, wrenGetSlotHandle (vm, 0), *g_scene);
          components::MonoBehaviourSystem::setInitialize      (entity, wrenMakeCallHandle(vm, "initialize()"), *g_scene);
          components::MonoBehaviourSystem::setDeinitialize    (entity, wrenMakeCallHandle(vm, "deinitialize()"), *g_scene);
          components::MonoBehaviourSystem::setUpdate          (entity, wrenMakeCallHandle(vm, "update()"), *g_scene);
          components::MonoBehaviourSystem::setFixedUpdate     (entity, wrenMakeCallHandle(vm, "fixedUpdate()"), *g_scene);
          components::MonoBehaviourSystem::setOnCollisionEnter(entity, wrenMakeCallHandle(vm, "onCollisionEnter(_,_)"), *g_scene);
		  //components::MonoBehaviourSystem::setOnCollisionStay (entity, wrenMakeCallHandle(vm, "onCollisionStay(_,_)"), *g_scene);
          components::MonoBehaviourSystem::setOnCollisionExit (entity, wrenMakeCallHandle(vm, "onCollisionExit(_,_)"), *g_scene);
          components::MonoBehaviourSystem::setOnTriggerEnter  (entity, wrenMakeCallHandle(vm, "onTriggerEnter(_,_)"), *g_scene);
          //components::MonoBehaviourSystem::setOnTriggerStay   (entity, wrenMakeCallHandle(vm, "onTriggerStay(_,_)"), *g_scene);
          components::MonoBehaviourSystem::setOnTriggerExit   (entity, wrenMakeCallHandle(vm, "onTriggerExit(_,_)"), *g_scene);
        };
        if (strcmp(signature, "goGet(_)") == 0) return [](WrenVM* vm) {
          /*Handle*/ wrenSetSlotHandle(vm, 0, g_scriptingData->getData(*GetForeign<entity::Entity>(vm, 1)).mono_behaviour);
        };
        if (strcmp(signature, "goRemovePrivate(_)") == 0) return [](WrenVM* vm) {
          entity::Entity entity = *GetForeign<entity::Entity>(vm, 1);
#define FREE(x) wrenReleaseHandle(vm, (WrenHandle*)components::MonoBehaviourSystem::get##x(entity, *g_scene)); components::MonoBehaviourSystem::set##x(entity, nullptr, *g_scene);

          FREE(Object);
          FREE(Initialize);
          FREE(Deinitialize);
          FREE(Update);
          FREE(FixedUpdate);
          FREE(OnCollisionEnter);
          FREE(OnCollisionExit);
          FREE(OnTriggerEnter);
          FREE(OnTriggerExit);

          /*Handle*/ wrenReleaseHandle(vm, g_scriptingData->getData(entity).mono_behaviour);
          /*Handle*/ g_scriptingData->getData(entity).mono_behaviour = nullptr;
        };
        return nullptr;
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		namespace Graphics
		{
			WrenForeignMethodFn Bind(const char* signature)
			{
				if (strcmp(signature, "vsync=(_)") == 0) return [](WrenVM* vm) {
					g_scene->renderer->setVSync(wrenGetSlotBool(vm, 1));
				};
				if (strcmp(signature, "vsync") == 0) return [](WrenVM* vm) {
					wrenSetSlotDouble(vm, 0, g_scene->renderer->getVSync() ? 1.0 : 0.0);
				};
				if (strcmp(signature, "renderScale=(_)") == 0) return [](WrenVM* vm) {
					g_scene->renderer->setRenderScale((float)wrenGetSlotDouble(vm, 1));
				};
				if (strcmp(signature, "renderScale") == 0) return [](WrenVM* vm) {
					wrenSetSlotDouble(vm, 0, (float)g_scene->renderer->getRenderScale());
				};
				if (strcmp(signature, "setLightShaders(_,_,_,_,_)") == 0) return [](WrenVM* vm) {
					String generate = wrenGetSlotString(vm, 1);
					String modify = wrenGetSlotString(vm, 2);
					double modify_count = wrenGetSlotDouble(vm, 3);
					String publish = wrenGetSlotString(vm, 4);
					String shadow_type = wrenGetSlotString(vm, 5);
					components::LightSystem::setShaders(generate, modify, (uint32_t)modify_count, publish, shadow_type, *g_scene);
				};
				if (strcmp(signature, "windowSize") == 0) return [](WrenVM* vm) {
					Vec2::make(vm, (glm::vec2)g_scene->window->getSize());
				};
				return nullptr;
			}
		}
		namespace GUI
		{
			/////////////////////////////////////////////////////////////////////////
			struct UserDataWrapper
			{
				WrenHandle* object;
				WrenHandle* function;
			};

			Vector<UserDataWrapper*>* g_userDataWrappers;

			/////////////////////////////////////////////////////////////////////////
			void callback(Vector<lambda::gui::JSVal> js_args, const void* user_data)
			{
				const UserDataWrapper& ud = *(UserDataWrapper*)user_data;

				Vector<ScriptValue> args;
				for (uint32_t i = 0; i < js_args.size(); ++i)
				{
					if (js_args[i].isBool())
						args.push_back(ScriptValue(js_args[i].asBool()));
					else if (js_args[i].isNumber())
						args.push_back(ScriptValue(js_args[i].asNumber()));
					else if (js_args[i].isString())
						args.push_back(ScriptValue(String(js_args[i].asString())));
				}

				g_world->getScripting()->executeFunction(ud.object, ud.function, args);
			}

			/////////////////////////////////////////////////////////////////////////
			WrenForeignMethodFn Bind(const char* signature)
			{
				if (strcmp(signature, "enabled=(_)") == 0) return [](WrenVM* vm) {
					g_world->getGUI().setEnabled(wrenGetSlotBool(vm, 1));
				};
				if (strcmp(signature, "enabled") == 0) return [](WrenVM* vm) {
					wrenSetSlotDouble(vm, 0, g_world->getGUI().getEnabled() ? 1.0 : 0.0);
				};
				if (strcmp(signature, "loadURL(_)") == 0) return [](WrenVM* vm) {
					String url = wrenGetSlotString(vm, 1);
					g_world->getGUI().loadURL(url);
				};
				if (strcmp(signature, "executeJavaScript(_)") == 0) return [](WrenVM* vm) {
					g_world->getGUI().executeJavaScript(wrenGetSlotString(vm, 1));
				};
				if (strcmp(signature, "bindCallback(_,_)") == 0) return [](WrenVM* vm) {
					String name = wrenGetSlotString(vm, 1);
					UserDataWrapper* user_data =
						foundation::Memory::construct<UserDataWrapper>();
					user_data->object = wrenGetSlotHandle(vm, 2);
					wrenSetSlotHandle(vm, 0, user_data->object);
					user_data->function = wrenMakeCallHandle(vm, name.c_str());
					g_world->getGUI().bindJavaScriptCallback(
						name.substr(0, name.find("(")),
						callback,
						user_data
					);

					if (!g_userDataWrappers)
						g_userDataWrappers = foundation::Memory::construct<Vector<UserDataWrapper*>>();

					g_userDataWrappers->push_back(user_data);
				};
				return nullptr;
			}
		}
		namespace PostProcess
    {
      WrenForeignMethodFn Bind(const char* signature)
      {
        if (strcmp(signature, "addRenderTarget(_,_,_)") == 0) return [](WrenVM* vm) {
          g_scene->post_process_manager->addTarget(
            platform::RenderTarget(Name(wrenGetSlotString(vm, 1)), (float)wrenGetSlotDouble(vm, 2), (TextureFormat)(uint32_t)wrenGetSlotDouble(vm, 3))
          );
        };
        if (strcmp(signature, "addRenderTarget(_,_)") == 0) return [](WrenVM* vm) {
			auto texture = *GetForeign<asset::VioletTextureHandle>(vm, 2);
			g_scene->post_process_manager->addTarget(
            platform::RenderTarget(Name(wrenGetSlotString(vm, 1)), 0.0f, texture, true)
          );
        };
        if (strcmp(signature, "setRenderTargetFlag(_,_,_)") == 0) return [](WrenVM* vm) {
			String rt = wrenGetSlotString(vm, 1);
			uint32_t flag = (uint32_t)wrenGetSlotDouble(vm, 2);
			bool value = wrenGetSlotBool(vm, 3);
			auto texture = g_scene->post_process_manager->getTarget(Name(rt)).getTexture();

			for (uint32_t i = 0; i < texture->getLayerCount(); ++i)
			{
				if (value)
					texture->getLayer(i).setFlags(texture->getLayer(i).getFlags() | flag);
				else
					texture->getLayer(i).setFlags(texture->getLayer(i).getFlags() & ~flag);
			}
        };
        if (strcmp(signature, "setFinalRenderTarget(_)") == 0) return [](WrenVM* vm) {
          g_scene->post_process_manager->setFinalTarget(Name(wrenGetSlotString(vm, 1)));
        };
        if (strcmp(signature, "addShaderPass(_,_,_,_)") == 0) return [](WrenVM* vm) {
          Name name(wrenGetSlotString(vm, 1));
          asset::VioletShaderHandle shader = *GetForeign<asset::VioletShaderHandle>(vm, 2);
          Vector<platform::RenderTarget> input(wrenGetListCount(vm, 3));
          Vector<platform::RenderTarget> output(wrenGetListCount(vm, 4));
          for (int i = 0; i < (int)input.size(); ++i)
          {
            wrenGetListElement(vm, 3, i, 0);
            const char* target_cstr = wrenGetSlotString(vm, 0);
            Name target_name = Name(target_cstr);
            const auto& target = g_scene->post_process_manager->getTarget(target_name);
            input.at(i) = target;
          }
          for (int i = 0; i < (int)output.size(); ++i)
          {
            wrenGetListElement(vm, 4, i, 0);
            output.at(i) = g_scene->post_process_manager->getTarget(Name(wrenGetSlotString(vm, 0)));
          }

          g_scene->post_process_manager->addPass(platform::ShaderPass(name, shader, input, output));
        };
        if (strcmp(signature, "setShaderPassEnabled(_,_)") == 0) return [](WrenVM* vm) {
          Name name(wrenGetSlotString(vm, 1));
          for (auto& pass : g_scene->post_process_manager->getPasses())
          {
            if (pass.getName() == name)
            {
              pass.setEnabled(wrenGetSlotBool(vm, 2));
              return;
            }
          }
        };
        if (strcmp(signature, "irradianceConvolution(_,_)") == 0) return [](WrenVM* vm) {
          String input = wrenGetSlotString(vm, 1);
          String output = wrenGetSlotString(vm, 2);

          platform::RenderTarget& rt_input = g_scene->post_process_manager->getTarget(input);
					asset::VioletShaderHandle shader = asset::ShaderManager::getInstance()->get(Name("resources/shaders/irradiance_convolution.fx"));

          float as = (float)rt_input.getTexture()->getLayer(0u).getHeight() / (float)rt_input.getTexture()->getLayer(0u).getWidth();

		  struct RenderAction : public scene::IRenderAction
		  {
			  platform::ShaderPass shader_pass;
			  asset::VioletMeshHandle mesh;

			  virtual void execute(scene::Scene& scene) override
			  {
				  scene.renderer->setMesh(mesh);
				  scene.renderer->setSubMesh(0u);
				  scene.renderer->bindShaderPass(shader_pass);
				  scene.renderer->draw();
			  }

				virtual ~RenderAction() override
				{
					shader_pass = platform::ShaderPass();
					mesh = nullptr;
				}
		  };
		  RenderAction* render_action = foundation::GetFrameHeap()->construct<RenderAction>();
		  g_scene->render_actions.push_back(render_action);

          VioletTexture violet_texture;
          violet_texture.width = 512u;
          violet_texture.height = (uint32_t)(512.0f * as);
          violet_texture.mip_count = 1u;
          violet_texture.file = output;
          violet_texture.flags = kTextureFlagIsRenderTarget;
          violet_texture.format = TextureFormat::kR16G16B16A16;
          violet_texture.hash = hash(output);
          asset::VioletTextureHandle texture = asset::TextureManager::getInstance()->create(output, violet_texture);

          render_action->mesh = asset::MeshManager::getInstance()->create(Name("__irradiance_convolution_mesh__"), asset::Mesh::createScreenQuad());

		  render_action->shader_pass = platform::ShaderPass(
            Name("IrradianceConvolution"),
            shader,
            { rt_input },
            { platform::RenderTarget(Name("IrradianceConvolution_Out"), texture) }
          );

          g_scene->post_process_manager->addTarget(platform::RenderTarget(Name(output), texture));
        };
        if (strcmp(signature, "hammerhead(_,_)") == 0) return [](WrenVM* vm) {
          String input = wrenGetSlotString(vm, 1);
          String output = wrenGetSlotString(vm, 2);

          platform::RenderTarget& rt_input = g_scene->post_process_manager->getTarget(input);
		  asset::VioletShaderHandle shader = asset::ShaderManager::getInstance()->get(Name("resources/shaders/hammerhead.fx"));

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

		  struct RenderAction : public scene::IRenderAction
		  {
			  asset::VioletMeshHandle mesh;

			  struct Pass
			  {
				  platform::ShaderPass shader_pass;
				  float user_data;
			  };
			  Vector<Pass> passes;

			  virtual void execute(scene::Scene& scene) override
			  {
				  scene.renderer->setMesh(mesh);
				  scene.renderer->setSubMesh(0u);

				  for (auto pass : passes)
				  {
					  scene.renderer->bindShaderPass(pass.shader_pass);
					  scene.renderer->setUserData(glm::vec4(pass.user_data, 0.0f, 0.0f, 0.0f), 0);
					  scene.renderer->draw();
				  }
			  }

				virtual ~RenderAction() override 
				{
					passes.resize(0u);
				}
		  };
		  RenderAction* render_action = foundation::GetFrameHeap()->construct<RenderAction>();
		  g_scene->render_actions.push_back(render_action);

          render_action->mesh = asset::MeshManager::getInstance()->create(Name("__hammerhead_mesh__"), asset::Mesh::createScreenQuad());

          platform::RenderTarget rt_in(Name("Hammerhead_In"), texture);
          for (uint32_t i = 0u; i < violet_texture.mip_count; ++i)
          {
            platform::RenderTarget rt_out(Name("Hammerhead_Out_" + toString(i)), texture);
            rt_out.setMipMap(i);

			RenderAction::Pass pass;
            pass.user_data = (float)i / (float)violet_texture.mip_count;
            pass.shader_pass = platform::ShaderPass(
              Name("Hammerhead_" + toString(i)),
              shader,
              { rt_input },
              { rt_out }
            );
			render_action->passes.push_back(pass);
          }

		  g_scene->post_process_manager->addTarget(platform::RenderTarget(Name(output), texture));
        };
        return nullptr;
      }
    }
    namespace Input
    {
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
          wrenSetSlotDouble(vm, 0, std::max(wrenGetSlotDouble(vm, 1), wrenGetSlotDouble(vm, 2)));
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
		WrenForeignMethodFn Bind(const char* signature)
		{
			if (strcmp(signature, "fixedDeltaTime") == 0) return [](WrenVM* vm) {
				wrenSetSlotDouble(vm, 0, g_scene->fixed_time_step);
			};
			if (strcmp(signature, "deltaTime") == 0) return [](WrenVM* vm) {
				wrenSetSlotDouble(vm, 0, g_world->getDeltaTime());
			};
			if (strcmp(signature, "timeScale=(_)") == 0) return [](WrenVM* vm) {
				g_scene->fixed_time_step /= g_scene->time_scale;
				g_scene->time_scale = wrenGetSlotDouble(vm, 1);
				g_scene->fixed_time_step *= g_scene->time_scale;
			};
			if (strcmp(signature, "timeScale") == 0) return [](WrenVM* vm) {
				wrenSetSlotDouble(vm, 0, g_scene->time_scale);
			};
			return nullptr;
		}
	}
		namespace Debug
		{
			WrenForeignMethodFn Bind(const char* signature)
			{
				if (strcmp(signature, "drawLine(_,_,_)") == 0) return [](WrenVM* vm) {
					g_scene->debug_renderer.DrawLine(platform::DebugLine(
						*GetForeign<glm::vec3>(vm, 1),
						*GetForeign<glm::vec3>(vm, 2),
						*GetForeign<glm::vec4>(vm, 3)
					));
				};
				if (strcmp(signature, "drawTri(_,_,_,_)") == 0) return [](WrenVM* vm) {
					g_scene->debug_renderer.DrawTri(platform::DebugTri(
						*GetForeign<glm::vec3>(vm, 1),
						*GetForeign<glm::vec3>(vm, 2),
						*GetForeign<glm::vec3>(vm, 3),
						*GetForeign<glm::vec4>(vm, 4)
					));
				};
				return nullptr;
			}
		}
		namespace Manifold
		{
			WrenHandle* handle = nullptr;
			/////////////////////////////////////////////////////////////////////////
			physics::Manifold* makeAt(WrenVM* vm, int slot, int class_slot, physics::Manifold val = physics::Manifold())
			{
				if (handle == nullptr)
				{
					wrenGetVariable(vm, "Core", "Manifold", class_slot);
					handle = wrenGetSlotHandle(vm, class_slot);
				}
				wrenSetSlotHandle(vm, class_slot, handle);
				physics::Manifold* data = MakeForeign<physics::Manifold>(vm, slot, class_slot);
				memcpy(data, &val, sizeof(physics::Manifold));
				
				return data;
			}

			/////////////////////////////////////////////////////////////////////////
			physics::Manifold* make(WrenVM* vm, const physics::Manifold val = physics::Manifold())
			{
				return makeAt(vm, 0, 1, val);
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
				if (strcmp(signature, "gameObject") == 0) return [](WrenVM* vm) {
					GameObject::make(vm, GetForeign<physics::Manifold>(vm, 0)->rhs);
				};
				if (strcmp(signature, "normal") == 0) return [](WrenVM* vm) {
					Vec3::make(vm, GetForeign<physics::Manifold>(vm, 0)->contacts[0].normal);
				};
				if (strcmp(signature, "depth") == 0) return [](WrenVM* vm) {
					wrenSetSlotDouble(vm, 0, (double)GetForeign<physics::Manifold>(vm, 0)->contacts[0].depth);
				};
				if (strcmp(signature, "point") == 0) return [](WrenVM* vm) {
					physics::Manifold* data = GetForeign<physics::Manifold>(vm, 0);
					Vec3::make(vm, data->contacts[0].point);
				};
				return nullptr;
			}
		}
		namespace Physics
		{
			WrenForeignMethodFn Bind(const char* signature)
			{
				if (strcmp(signature, "gravity") == 0) return [](WrenVM* vm) {
					Vec3::make(vm, components::RigidBodySystem::getPhysicsWorld(*g_scene)->getGravity());
				};
				if (strcmp(signature, "gravity=(_)") == 0) return [](WrenVM* vm) {
					components::RigidBodySystem::getPhysicsWorld(*g_scene)->setGravity(*GetForeign<glm::vec3>(vm, 1));
				};
				if (strcmp(signature, "castRay(_,_)") == 0) return [](WrenVM* vm) {
					glm::vec3 from = *GetForeign<glm::vec3>(vm, 1);
					glm::vec3 to = *GetForeign<glm::vec3>(vm, 2);

					Vector<physics::Manifold> result = components::RigidBodySystem::getPhysicsWorld(*g_scene)->raycast(from, to);
					wrenSetSlotNewList(vm, 0);

					for (physics::Manifold res : result)
					{
						Manifold::makeAt(vm, 1, 2, res);
						wrenInsertInList(vm, 0, -1, 1);
					}
				};
				if (strcmp(signature, "debugDrawEnabled") == 0) return [](WrenVM* vm) {
					wrenSetSlotBool(vm, 0, components::RigidBodySystem::getPhysicsWorld(*g_scene)->getDebugDrawEnabled());
				};
				if (strcmp(signature, "debugDrawEnabled=(_)") == 0) return [](WrenVM* vm) {
					components::RigidBodySystem::getPhysicsWorld(*g_scene)->setDebugDrawEnabled(wrenGetSlotBool(vm, 1));
				};
				return nullptr;
			}
		}
    namespace File
    {
      struct File
      {
        FILE* fp;
      };
      WrenHandle* handle = nullptr;
      File* make(WrenVM* vm, File val = File())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core", "File", 0);
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
        if (strcmp(signature, "close()") == 0) return [](WrenVM* vm) {
          File* file = GetForeign<File>(vm);
          LMB_ASSERT(file->fp != nullptr, "WREN: File was already closed");
          FileSystem::fclose(file->fp);
          file->fp = nullptr;
        };
        if (strcmp(signature, "read()") == 0) return [](WrenVM* vm) {
          File* file = GetForeign<File>(vm);
          LMB_ASSERT(file->fp != nullptr, "WREN: No file was opened");
          fseek(file->fp, 0, SEEK_END);
          long ret = ftell(file->fp);
          String buffer(ret, '\0');
          fseek(file->fp, 0, SEEK_SET);
          fread((void*)buffer.data(), 1u, ret, file->fp);
          wrenSetSlotString(vm, 0, buffer.c_str());
        };
        if (strcmp(signature, "write(_)") == 0) return [](WrenVM* vm) {
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
      struct Noise
      {
        FastNoise noise;
      };
      WrenHandle* handle = nullptr;
      Noise* make(WrenVM* vm, Noise val = Noise())
      {
        if (handle == nullptr)
        {
          wrenGetVariable(vm, "Core", "Noise", 0);
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

	///////////////////////////////////////////////////////////////////////////
	namespace NavMeshNode
	{
		struct NavMeshNode
		{
			platform::NavNode* node;
		};
		WrenHandle* handle = nullptr;
		NavMeshNode* makeAt(WrenVM* vm, int slot, int class_slot, NavMeshNode val = NavMeshNode())
		{
			if (handle == nullptr)
			{
				wrenGetVariable(vm, "Core", "NavMeshNode", class_slot);
				handle = wrenGetSlotHandle(vm, class_slot);
			}
			wrenSetSlotHandle(vm, class_slot, handle);
			NavMeshNode* data = MakeForeign<NavMeshNode>(vm, slot, class_slot);
			memcpy(data, &val, sizeof(NavMeshNode));
			return data;
		}
		NavMeshNode* make(WrenVM* vm, NavMeshNode val = NavMeshNode())
		{
			return makeAt(vm, 0, 1, val);
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
			if (strcmp(signature, "addConnection(_)") == 0) return [](WrenVM* vm) {
				NavMeshNode& nav_mesh_node = *GetForeign<NavMeshNode>(vm, 0);
				NavMeshNode& other_nav_mesh_node = *GetForeign<NavMeshNode>(vm, 1);
				nav_mesh_node.node->addConnection(other_nav_mesh_node.node);
			};
			if (strcmp(signature, "removeConnection(_)") == 0) return [](WrenVM* vm) {
				NavMeshNode& nav_mesh_node = *GetForeign<NavMeshNode>(vm, 0);
				NavMeshNode& other_nav_mesh_node = *GetForeign<NavMeshNode>(vm, 1);
				nav_mesh_node.node->removeConnection(other_nav_mesh_node.node);
			};
			return nullptr;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	namespace NavMesh
	{
		struct NavMesh
		{
			platform::NavMap nav_map;
		};
		WrenHandle* handle = nullptr;
		NavMesh* make(WrenVM* vm, NavMesh val = NavMesh())
		{
			if (handle == nullptr)
			{
				wrenGetVariable(vm, "Core", "NavMesh", 0);
				handle = wrenGetSlotHandle(vm, 0);
			}
			wrenSetSlotHandle(vm, 1, handle);
			NavMesh* data = MakeForeign<NavMesh>(vm, 0, 1);
			memcpy(data, &val, sizeof(NavMesh));
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
			if (strcmp(signature, "addNode(_)") == 0) return [](WrenVM* vm) {
				NavMesh& nav_mesh = *GetForeign<NavMesh>(vm, 0);
				const glm::vec3& position = *GetForeign<glm::vec3>(vm, 1);
				NavMeshNode::make(vm, { nav_mesh.nav_map.addNode(position) });
			};
			if (strcmp(signature, "removeNode(_)") == 0) return [](WrenVM* vm) {
				NavMesh& nav_mesh = *GetForeign<NavMesh>(vm, 0);
				const glm::vec3& position = *GetForeign<glm::vec3>(vm, 1);
				nav_mesh.nav_map.removeNode(position);
			};
			if (strcmp(signature, "findPath(_,_)") == 0) return [](WrenVM* vm) {
				NavMesh& nav_mesh = *GetForeign<NavMesh>(vm, 0);
				const glm::vec3& from = *GetForeign<glm::vec3>(vm, 1);
				const glm::vec3& to = *GetForeign<glm::vec3>(vm, 2);
				Vector<glm::vec3> path = nav_mesh.nav_map.findPath(from, to);

				wrenSetSlotNewList(vm, 0);

				for (const glm::vec3& pos : path)
				{
					Vec3::makeAt(vm, 1, 2, pos);
					wrenInsertInList(vm, 0, -1, 1);
				}
			};
			return nullptr;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	namespace NavMeshPromise
	{
		struct NavMeshPromise
		{
			platform::Promise<Vector<glm::vec3>>* promise;
		};
		WrenHandle* handle = nullptr;
		NavMeshPromise* make(WrenVM* vm, NavMeshPromise val = NavMeshPromise())
		{
			if (handle == nullptr)
			{
				wrenGetVariable(vm, "Core", "NavMeshPromise", 0);
				handle = wrenGetSlotHandle(vm, 0);
			}
			wrenSetSlotHandle(vm, 1, handle);
			NavMeshPromise* data = MakeForeign<NavMeshPromise>(vm, 0, 1);
			memcpy(data, &val, sizeof(NavMeshPromise));
			return data;
		}
		WrenForeignClassMethods Construct()
		{
			return WrenForeignClassMethods{
				[](WrenVM* vm) {
				make(vm);
			},
				[](void* data) {
				delete ((NavMeshPromise*)data)->promise;
			}
			};
		}
		WrenForeignMethodFn Bind(const char* signature)
		{
			if (strcmp(signature, "finished") == 0) return [](WrenVM* vm) {
				NavMeshPromise& promise = *GetForeign<NavMeshPromise>(vm, 0);
				wrenSetSlotBool(vm, 0, promise.promise->is_finished);
			};
			if (strcmp(signature, "path") == 0) return [](WrenVM* vm) {
				NavMeshPromise& promise = *GetForeign<NavMeshPromise>(vm, 0);
				LMB_ASSERT(promise.promise, "");

				Vector<glm::vec3> path;
				if (promise.promise->get(path))
				{
					wrenSetSlotNewList(vm, 0);

					for (const glm::vec3& pos : path)
					{
						Vec3::makeAt(vm, 1, 2, pos);
						wrenInsertInList(vm, 0, -1, 1);
					}
				}
			};
			return nullptr;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	namespace TriNavMesh
	{
		struct TriNavMesh
		{
			platform::TriNavMap nav_map;
		};
		WrenHandle* handle = nullptr;
		TriNavMesh* make(WrenVM* vm, TriNavMesh val = TriNavMesh())
		{
			if (handle == nullptr)
			{
				wrenGetVariable(vm, "Core", "TriNavMesh", 0);
				handle = wrenGetSlotHandle(vm, 0);
			}
			wrenSetSlotHandle(vm, 1, handle);
			TriNavMesh* data = MakeForeign<TriNavMesh>(vm, 0, 1);
			memcpy(data, &val, sizeof(TriNavMesh));
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
			if (strcmp(signature, "addTri(_,_,_)") == 0) return [](WrenVM* vm) {
				TriNavMesh& nav_mesh = *GetForeign<TriNavMesh>(vm, 0);
				const glm::vec3& a = *GetForeign<glm::vec3>(vm, 1);
				const glm::vec3& b = *GetForeign<glm::vec3>(vm, 2);
				const glm::vec3& c = *GetForeign<glm::vec3>(vm, 3);
				nav_mesh.nav_map.addTri(a, b, c);
			};
			if (strcmp(signature, "addTriHole(_,_,_)") == 0) return [](WrenVM* vm) {
				TriNavMesh& nav_mesh = *GetForeign<TriNavMesh>(vm, 0);
				const glm::vec3& a = *GetForeign<glm::vec3>(vm, 1);
				const glm::vec3& b = *GetForeign<glm::vec3>(vm, 2);
				const glm::vec3& c = *GetForeign<glm::vec3>(vm, 3);
				nav_mesh.nav_map.addTriHole(a, b, c);
			};
			if (strcmp(signature, "addQuad(_,_)") == 0) return [](WrenVM* vm) {
				TriNavMesh& nav_mesh = *GetForeign<TriNavMesh>(vm, 0);
				const glm::vec3& min = *GetForeign<glm::vec3>(vm, 1);
				const glm::vec3& max = *GetForeign<glm::vec3>(vm, 2);
				nav_mesh.nav_map.addQuad(min, max);
			};
			if (strcmp(signature, "addQuadHole(_,_)") == 0) return [](WrenVM* vm) {
				TriNavMesh& nav_mesh = *GetForeign<TriNavMesh>(vm, 0);
				const glm::vec3& min = *GetForeign<glm::vec3>(vm, 1);
				const glm::vec3& max = *GetForeign<glm::vec3>(vm, 2);
				nav_mesh.nav_map.addQuadHole(min, max);
			};
			if (strcmp(signature, "getTriangles()") == 0) return [](WrenVM* vm) {
				TriNavMesh& nav_mesh = *GetForeign<TriNavMesh>(vm, 0);
				Vector<glm::vec3> tris = nav_mesh.nav_map.getTris();

				wrenSetSlotNewList(vm, 0);

				for (const glm::vec3& tri : tris)
				{
					Vec3::makeAt(vm, 1, 2, tri);
					wrenInsertInList(vm, 0, -1, 1);
				}
			};
			if (strcmp(signature, "findPath(_,_)") == 0) return [](WrenVM* vm) {
				TriNavMesh& nav_mesh = *GetForeign<TriNavMesh>(vm, 0);
				const glm::vec3& from = *GetForeign<glm::vec3>(vm, 1);
				const glm::vec3& to = *GetForeign<glm::vec3>(vm, 2);
				Vector<glm::vec3> path = nav_mesh.nav_map.findPath(from, to);

				wrenSetSlotNewList(vm, 0);

				for (const glm::vec3& pos : path)
				{
					Vec3::makeAt(vm, 1, 2, pos);
					wrenInsertInList(vm, 0, -1, 1);
				}
			};
			if (strcmp(signature, "findPathPromise(_,_)") == 0) return [](WrenVM* vm) {
				TriNavMesh& nav_mesh = *GetForeign<TriNavMesh>(vm, 0);
				const glm::vec3& from = *GetForeign<glm::vec3>(vm, 1);
				const glm::vec3& to = *GetForeign<glm::vec3>(vm, 2);
				auto promise = nav_mesh.nav_map.findPathPromise(&nav_mesh.nav_map, from, to);
				NavMeshPromise::make(vm, { promise });
			};

			return nullptr;
		}
	}

		///////////////////////////////////////////////////////////////////////////
    namespace Assert
    {
			/////////////////////////////////////////////////////////////////////////
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

		///////////////////////////////////////////////////////////////////////////
    bool hashEqual(const char* lhs, const char* rhs)
    {
      return (hash(lhs) == hash(rhs));
    }

		///////////////////////////////////////////////////////////////////////////
	WrenForeignClassMethods wrenBindForeignClass(
		WrenVM* vm,
		const char* module,
		const char* className)
	{
		if (strstr(module, "Core") != 0)
		{
			if (hashEqual(className, "Vec2"))
				return Vec2::Construct();
			if (hashEqual(className, "Vec3"))
				return Vec3::Construct();
			if (hashEqual(className, "Vec4"))
				return Vec4::Construct();
			if (hashEqual(className, "Quat"))
				return Quat::Construct();
			if (hashEqual(className, "Texture"))
				return Texture::Construct();
			if (hashEqual(className, "Shader"))
				return Shader::Construct();
			if (hashEqual(className, "Wave"))
				return Wave::Construct();
			if (hashEqual(className, "Mesh"))
				return Mesh::Construct();
			if (hashEqual(className, "GameObject"))
				return GameObject::Construct();
			if (hashEqual(className, "Transform"))
				return Transform::Construct();
			if (hashEqual(className, "Camera"))
				return Camera::Construct();
			if (hashEqual(className, "MeshRender"))
				return MeshRender::Construct();
			if (hashEqual(className, "Lod"))
				return LOD::Construct();
			if (hashEqual(className, "RigidBody"))
				return RigidBody::Construct();
			if (hashEqual(className, "WaveSource"))
				return WaveSource::Construct();
			if (hashEqual(className, "Collider"))
				return Collider::Construct();
			if (hashEqual(className, "Light"))
				return Light::Construct();
			if (hashEqual(className, "Manifold"))
				return Manifold::Construct();
			if (hashEqual(className, "File"))
				return File::Construct();
			if (hashEqual(className, "Noise"))
				return Noise::Construct();
			if (hashEqual(className, "NavMeshNode"))
				return NavMeshNode::Construct();
			if (hashEqual(className, "NavMesh"))
				return NavMesh::Construct();
			if (hashEqual(className, "NavMeshPromise"))
				return NavMeshPromise::Construct();
			if (hashEqual(className, "TriNavMesh"))
				return TriNavMesh::Construct();
		}

		return WrenForeignClassMethods{};
	}

	///////////////////////////////////////////////////////////////////////////
	WrenForeignMethodFn wrenBindForeignMethod(
		WrenVM* vm,
		const char* module,
		const char* className,
		bool isStatic,
		const char* signature)
	{
		if (strstr(module, "Core") != 0)
		{
			if (hashEqual(className, "Console"))
				return Console::Bind(signature);
			if (hashEqual(className, "Vec2"))
				return Vec2::Bind(signature);
			if (hashEqual(className, "Vec3"))
				return Vec3::Bind(signature);
			if (hashEqual(className, "Vec4"))
				return Vec4::Bind(signature);
			if (hashEqual(className, "Quat"))
				return Quat::Bind(signature);
			if (hashEqual(className, "Texture"))
				return Texture::Bind(signature);
			if (hashEqual(className, "Shader"))
				return Shader::Bind(signature);
			if (hashEqual(className, "Wave"))
				return Wave::Bind(signature);
			if (hashEqual(className, "Mesh"))
				return Mesh::Bind(signature);
			if (hashEqual(className, "GameObject"))
				return GameObject::Bind(signature);
			if (hashEqual(className, "Transform"))
				return Transform::Bind(signature);
			if (hashEqual(className, "Camera"))
				return Camera::Bind(signature);
			if (hashEqual(className, "MeshRender"))
				return MeshRender::Bind(signature);
			if (hashEqual(className, "Lod"))
				return LOD::Bind(signature);
			if (hashEqual(className, "RigidBody"))
				return RigidBody::Bind(signature);
			if (hashEqual(className, "WaveSource"))
				return WaveSource::Bind(signature);
			if (hashEqual(className, "Collider"))
				return Collider::Bind(signature);
			if (hashEqual(className, "Light"))
				return Light::Bind(signature);
			if (hashEqual(className, "MonoBehaviour"))
				return MonoBehaviour::Bind(signature);
			if (hashEqual(className, "Graphics"))
				return Graphics::Bind(signature);
			if (hashEqual(className, "GUI"))
				return GUI::Bind(signature);
			if (hashEqual(className, "PostProcess"))
				return PostProcess::Bind(signature);
			if (hashEqual(className, "Input"))
				return Input::Bind(signature);
			if (hashEqual(className, "Math"))
				return Math::Bind(signature);
			if (hashEqual(className, "Time"))
				return Time::Bind(signature);
			if (hashEqual(className, "Debug"))
				return Debug::Bind(signature);
			if (hashEqual(className, "Physics"))
				return Physics::Bind(signature);
			if (hashEqual(className, "Manifold"))
				return Manifold::Bind(signature);
			if (hashEqual(className, "File"))
				return File::Bind(signature);
			if (hashEqual(className, "Noise"))
				return Noise::Bind(signature);
			if (hashEqual(className, "NavMeshNode"))
				return NavMeshNode::Bind(signature);
			if (hashEqual(className, "NavMesh"))
				return NavMesh::Bind(signature);
			if (hashEqual(className, "NavMeshPromise"))
				return NavMeshPromise::Bind(signature);
			if (hashEqual(className, "TriNavMesh"))
				return TriNavMesh::Bind(signature);
			if (hashEqual(className, "Assert"))
				return Assert::Bind(signature);
		}

		return nullptr;
	}

		///////////////////////////////////////////////////////////////////////////
		char* wrenLoadModule(WrenVM* vm, const char* name_cstr)
		{
			if (strstr(name_cstr, "Core") != 0)
			{
#include "wren_binding.inc"
				size_t data_size = strlen(wrenModuleSource);
				char* data = (char*)WREN_ALLOC(data_size + 1u);
				memcpy(data, wrenModuleSource, data_size + 1u);
				return data;
			}
			else
			{
				String str = FileSystem::FileToString(String(name_cstr) + ".wren");
				char* data = (char*)WREN_ALLOC(str.size() + 1u);
				memcpy(data, str.data(), str.size() + 1u);
				return data;
			}

			return nullptr;
		}

		///////////////////////////////////////////////////////////////////////////
		extern void WrenBind(void* config)
		{
			WrenConfiguration* configuration = (WrenConfiguration*)config;
			configuration->bindForeignClassFn = wrenBindForeignClass;
			configuration->bindForeignMethodFn = wrenBindForeignMethod;
			configuration->loadModuleFn = wrenLoadModule;
		}

		///////////////////////////////////////////////////////////////////////////
		extern void WrenSetWorld(world::IWorld* world)
		{
			g_scriptingData = foundation::Memory::construct<ScriptingData>();
			g_world = world;
			g_scene = &world->getScene();
		}

		///////////////////////////////////////////////////////////////////////////
		extern void WrenHandleValue(WrenVM* vm, const ScriptValue& value, int slot)
		{
			switch (value.getType())
			{
			case ScriptValue::kBoolean:
				wrenSetSlotBool(vm, slot, value.getBool()); break;
			case ScriptValue::kInt8:
				wrenSetSlotDouble(vm, slot, (double)value.getInt8()); break;
			case ScriptValue::kUint8:
				wrenSetSlotDouble(vm, slot, (double)value.getUint8()); break;
			case ScriptValue::kInt16:
				wrenSetSlotDouble(vm, slot, (double)value.getInt16()); break;
			case ScriptValue::kUint16:
				wrenSetSlotDouble(vm, slot, (double)value.getUint16()); break;
			case ScriptValue::kInt32:
				wrenSetSlotDouble(vm, slot, (double)value.getInt32()); break;
			case ScriptValue::kUint32:
				wrenSetSlotDouble(vm, slot, (double)value.getUint32()); break;
			case ScriptValue::kInt64:
				wrenSetSlotDouble(vm, slot, (double)value.getInt64()); break;
			case ScriptValue::kUint64:
				wrenSetSlotDouble(vm, slot, (double)value.getUint64()); break;
			case ScriptValue::kFloat:
				wrenSetSlotDouble(vm, slot, (double)value.getFloat()); break;
			case ScriptValue::kDouble:
				wrenSetSlotDouble(vm, slot, value.getDouble()); break;
			case ScriptValue::kVec2:
				Vec2::makeAt(vm, slot, slot + 1, (glm::vec2)value.getVec2()); break;
			case ScriptValue::kVec3:
				Vec3::makeAt(vm, slot, slot + 1, (glm::vec3)value.getVec3()); break;
			case ScriptValue::kVec4:
				Vec4::makeAt(vm, slot, slot + 1, (glm::vec4)value.getVec4()); break;
			case ScriptValue::kEntity:
				GameObject::makeAt(vm, slot, slot + 1, value.getEntity()); break;
			case ScriptValue::kString:
			{
				size_t len = value.getString().size() + 1;
				const char* c_str = (const char*)WREN_ALLOC(len);
				memcpy((char*)c_str, value.getString().c_str(), len);
				wrenSetSlotString(vm, slot, c_str);
				break;
			}
			default: case ScriptValue::kNull: wrenSetSlotNull(vm, slot); break;
			}
		}

		///////////////////////////////////////////////////////////////////////////
		extern void WrenRelease(WrenVM* vm)
		{
			g_scriptingData->freeAll(vm);
			foundation::Memory::destruct(g_scriptingData);
			g_scriptingData = nullptr;

#define SAFE_RELEASE(vm, handle) if (handle) { wrenReleaseHandle(vm, handle); handle = nullptr; }

			for (GUI::UserDataWrapper* user_data_wrapper : *GUI::g_userDataWrappers)
			{
				SAFE_RELEASE(vm, user_data_wrapper->object);
				SAFE_RELEASE(vm, user_data_wrapper->function);
				foundation::Memory::destruct(user_data_wrapper);
			}
			foundation::Memory::destruct(GUI::g_userDataWrappers);

			SAFE_RELEASE(vm, Vec2::handle);
			SAFE_RELEASE(vm, Vec3::handle);
			SAFE_RELEASE(vm, Vec4::handle);
			SAFE_RELEASE(vm, Quat::handle);
			SAFE_RELEASE(vm, Texture::handle);
			SAFE_RELEASE(vm, Shader::handle);
			SAFE_RELEASE(vm, Wave::handle);
			SAFE_RELEASE(vm, Mesh::handle);
			SAFE_RELEASE(vm, GameObject::handle);
			SAFE_RELEASE(vm, Transform::handle);
			SAFE_RELEASE(vm, Camera::handle);
			SAFE_RELEASE(vm, MeshRender::handle);
			SAFE_RELEASE(vm, LOD::handle);
			SAFE_RELEASE(vm, RigidBody::handle);
			SAFE_RELEASE(vm, WaveSource::handle);
			SAFE_RELEASE(vm, Collider::handle);
			SAFE_RELEASE(vm, Light::handle);
			SAFE_RELEASE(vm, Manifold::handle);
			SAFE_RELEASE(vm, File::handle);
			SAFE_RELEASE(vm, Noise::handle);
			SAFE_RELEASE(vm, NavMeshNode::handle);
			SAFE_RELEASE(vm, NavMesh::handle);
			SAFE_RELEASE(vm, NavMeshPromise::handle);
			SAFE_RELEASE(vm, TriNavMesh::handle);

			components::MonoBehaviourSystem::deinitialize(*g_scene);

			g_scene = nullptr;
			g_world = nullptr;
		}
  }
}