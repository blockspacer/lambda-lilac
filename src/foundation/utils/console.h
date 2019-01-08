#pragma once
#include <iostream>
#include <cassert>
#include "containers/containers.h"
#define LOG_TO_FILE 1

#ifndef LOG_TO_FILE
#define LOG_TO_FILE 0
#endif

namespace lambda
{
  namespace foundation
  {
    enum class LogPriority : unsigned char
    {
      kNone    = 0x0000,
      kInfo    = 0x0001,
      kWarning = 0x0002,
      kError   = 0x0004,
      kDebug   = 0x0008,
    };

    extern void Log(LogPriority priority, const char* format, ...);
    extern void LogToFile(const String& msg);

#define LMB_LOG(...)      (Log(lambda::foundation::LogPriority::kNone,    __VA_ARGS__))
#define LMB_LOG_INFO(...) (Log(lambda::foundation::LogPriority::kInfo,    __VA_ARGS__))
#define LMB_LOG_WARN(...) (Log(lambda::foundation::LogPriority::kWarning, __VA_ARGS__))
#define LMB_LOG_ERR(...)  (Log(lambda::foundation::LogPriority::kError,   __VA_ARGS__))
#define LMB_LOG_DEBG(...) (Log(lambda::foundation::LogPriority::kDebug,   __VA_ARGS__))

    enum class Colour : uint8_t
    {
      kR            = 1 << 1,
      kG            = 1 << 2,
      kB            = 1 << 3,
      kI            = 1 << 4,

      kBlack        = 0,
      kDarkGray     = kI,
      kGray         = kR | kG | kB,
      kWhite        = kGray | kI,

      kRed          = kR,
      kGreen        = kG,
      kBlue         = kB,
      kMagenta      = kR | kB,
      kCyan         = kB | kG,
      kYellow       = kR | kG,

      kLightRed     = kR | kI,
      kLightGreen   = kG | kI,
      kLightBlue    = kB | kI,
      kLightMagenta = kR | kB | kI,
      kLightCyan    = kB | kG | kI,
      kLightYellow  = kR | kG | kI,
    };

    std::ostream& operator<<(std::ostream& os, const Colour& colour);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void InfoNP(const String& msg)
    {
#if LOG_TO_FILE == 1
      LogToFile(msg);
#endif
      std::cout << Colour::kGray << msg.c_str() << Colour::kDarkGray;
    }
    inline void InfoNP(const char* msg)
    {
      InfoNP(String(msg));
    }
    inline void Info(const char* msg)
    {
      InfoNP("[INFO] " + String(msg));
    }
    inline void Info(const String& msg)
    {
      Info(msg.c_str());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void DebugNP(const String& msg)
    {
#if LOG_TO_FILE == 1
      LogToFile(msg);
#endif
      std::cout << Colour::kCyan << msg.c_str() << Colour::kDarkGray;
    }
    inline void DebugNP(const char* msg)
    {
      DebugNP(String(msg));
    }
    inline void Debug(const char* msg)
    {
      DebugNP("[DEBG] " + String(msg));
    }
    inline void Debug(const String& msg)
    {
      Debug(msg.c_str());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void WarningNP(const String& msg)
    {
#if LOG_TO_FILE == 1
      LogToFile(msg);
#endif
      std::cout << Colour::kYellow << msg.c_str() << Colour::kDarkGray;
    }
    inline void WarningNP(const char* msg)
    {
      WarningNP(String(msg));
    }
    inline void Warning(const char* msg)
    {
      WarningNP("[WARN] " + String(msg));
    }
    inline void Warning(const String& msg)
    {
      Warning(msg.c_str());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void ErrorNP(const String& msg)
    {
#if LOG_TO_FILE == 1
      LogToFile(msg);
#endif
      std::cout << Colour::kRed << msg.c_str() << Colour::kDarkGray;
    }
    inline void ErrorNP(const char* msg)
    {
      ErrorNP(String(msg));
    }
    inline void Error(const char* msg)
    {
      ErrorNP("[ERR ] " + String(msg));
    }
    inline void Error(const String& msg)
    {
      Error(msg.c_str());
    }
  }                                                                                  
}

// TODO (Hilze): Make this a global state.
//#if defined(_DEBUG) && !defined(VIOLET_ASSERT)
#define VIOLET_ASSERT
//#endif

// No asserts in release mode.
#ifdef VIOLET_ASSERT
#define LMB_ASSERT(expr, ...) { if ((expr) == false) { \
LMB_LOG_ERR("Assertion failure\n=============================\n", __VA_ARGS__);\
LMB_LOG(__VA_ARGS__); \
LMB_LOG("\n\n"); \
assert(false); } }
//#define LMB_ASSERT(expr, ...) { if ((expr) == false) { LMB_LOG_ERR("Assertion failure\n================================\nFile: %s\nLine: %s\nFunction: %s\n\n%s\n\n", __FILE__, __LINE__, __func__,  __VA_ARGS__); assert(false); } }
#else
#define LMB_ASSERT(expr, msg)
#endif
