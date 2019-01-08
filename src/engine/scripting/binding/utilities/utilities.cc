#include <scripting/binding/utilities/ini.h>
#include <containers/containers.h>
#include <utils/console.h>
#include <chrono>
#include <thread>
#include <scripting/script_vector.h>
#include <systems/transform_system.h>
#include <utils/utilities.h>

namespace lambda
{
  namespace scripting
  {
    namespace utilities
    {
      namespace utilities
      {
        void Sleep(const float& seconds)
        {
          uint16_t ms = (uint16_t)(seconds * 1000.0f);
          std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
        void Print(const String& msg)
        {
          foundation::Info(msg);
        }
        void PrintLine(const String& msg)
        {
          Print(msg + "\n");
        }
        void Error(const String& msg)
        {
          foundation::Error(msg + "\n");
        }
        void Warning(const String& msg)
        {
          foundation::Warning(msg + "\n");
        }
        void Info(const String& msg)
        {
          foundation::Info(msg + "\n");
        }
        void Debug(const String& msg)
        {
          foundation::Debug(msg + "\n");
        }
        ScriptQuat LookRotation(const ScriptVec3& forward, const ScriptVec3& up)
        {
          return components::TransformSystem::lookRotation(forward, up);
        }
        ScriptVec3 QuatToEuler(const ScriptQuat& q)
        {
          return glm::eulerAngles((glm::quat)q);
        }
        ScriptQuat EulerToQuat(const ScriptVec3& e)
        {
          return ScriptQuat(e);
        }
        float Clamp(const float& min, const float& max, const float& val)
        {
          return std::fminf(std::fmaxf(val, min), max);
        }
        float Lerp(const float& a, const float& b, const float& v)
        {
          return (a + v * (b - a));
        }
        float WrapMax(const float& val, const float& max)
        {
          return fmodf(max + fmodf(val, max), max);
        }
        float Wrap(const float& min, const float& max, const float& val)
        {
          return min + WrapMax(val - min, max - min);
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          return Map<lambda::String, void*> {
            { "void Sleep(const float& in)", (void*)Sleep },
            { "void Print(const String& in)", (void*)Print },
            { "void PrintLine(const String& in)", (void*)PrintLine },
            { "void Error(const String& in)", (void*)Error },
            { "void Warning(const String& in)", (void*)Warning },
            { "void Info(const String& in)", (void*)Info },
            { "void Debug(const String& in)", (void*)Debug },
            { "float Random()", (void*)lambda::utilities::random },
            { "float Random(const float& in, const float& in)", (void*)lambda::utilities::randomRange },
            { "Quat LookRotation(const Vec3&in, const Vec3&in)", (void*)LookRotation },
            { "Vec3 QuatToEuler(const Quat&in)", (void*)QuatToEuler },
            { "Quat EulerToQuat(const Vec3&in)", (void*)EulerToQuat },
            { "float Clamp(const float& in, const float& in, const float& in)", (void*)Clamp },
            { "float Lerp(const float& in, const float& in, const float& in)", (void*)Lerp },
            { "float Wrap(const float& in, const float& in, const float& in)", (void*)Wrap }
          };
        }

        extern void Unbind()
        {
        }
      }
    }
  }
}
