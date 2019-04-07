#include "console.h"

#ifdef VIOLET_WIN32
#include <Windows.h>
#undef min
#undef max
#endif

#include <fstream>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <time.h>
#include <chrono>
#include <ctime>
#include "memory.h"
#include "file_system.h"

namespace lambda
{
  namespace foundation
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Log(LogPriority priority, const char* format, ...)
    {
      va_list args;
      va_start(args, format);
      uint16_t size = vsnprintf(nullptr, 0, format, args) + 1;
      va_end(args);

      va_start(args, format);
			char* msg = (char*)malloc(size);
			memset(msg, '\0', size);
      vsnprintf(msg, size, format, args);
      va_end(args);

      switch (priority)
      {
      case LogPriority::kNone:
        ErrorNP(msg);
        break;
      case LogPriority::kDebug:
        Debug(msg);
        break;
      case LogPriority::kError:
        Error(msg);
        break;
      case LogPriority::kInfo:
        Info(msg);
        break;
      case LogPriority::kWarning:
        Warning(msg);
        break;
      }

#if LOG_TO_FILE == 1
      LogToFile(msg);
#endif

			free(msg);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::ofstream& LogFile()
    {
      static std::ofstream fout;
      if (false == fout.is_open())
      {
        std::chrono::high_resolution_clock::time_point p = std::chrono::high_resolution_clock::now();
        std::time_t t = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::duration_cast<std::chrono::milliseconds>(p.time_since_epoch())
          ).count();

#ifdef VIOLET_OSX
        String time = std::ctime(&t);
#else
				char* time = (char*)malloc(26u);
				memset(time, '\0', 26u);
        ctime_s(time, 26u, &t);
#endif
        String output_file = FileSystem::GetBaseDir() + "logs/" + time + ".log";
        fout.open(stlString(output_file));
#ifndef VIOLET_OSX
				free(time);
#endif
      }
      return fout;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void LogToFile(const String& msg)
    {
      LogToFile(msg.c_str());
    }

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void LogToFile(const char* msg)
		{
			LogFile() << msg;
		}
    
#ifdef VIOLET_WIN32
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& os, const Colour& colour)
    {
      WORD res = 0;
      if ((uint8_t)colour & (uint8_t)Colour::kR) res |= FOREGROUND_RED;
      if ((uint8_t)colour & (uint8_t)Colour::kG) res |= FOREGROUND_GREEN;
      if ((uint8_t)colour & (uint8_t)Colour::kB) res |= FOREGROUND_BLUE;
      if ((uint8_t)colour & (uint8_t)Colour::kI) res |= FOREGROUND_INTENSITY;

      HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      SetConsoleTextAttribute(hConsole, res);
      return os;
    }
#elif VIOLET_OSX
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& os, const Colour& colour)
    {
      bool r = ((uint8_t)colour & (uint8_t)Colour::kR) ? true : false;
      bool g = ((uint8_t)colour & (uint8_t)Colour::kG) ? true : false;
      bool b = ((uint8_t)colour & (uint8_t)Colour::kB) ? true : false;
      bool i = ((uint8_t)colour & (uint8_t)Colour::kI) ? true : false;

#define CHECK(x, y) if (x) { os << (i ? "\033[1m" : "") << y; return os; }
      
      CHECK(r && g && b, "\033[37m"); // White
      CHECK(!r && !g && !b, "\033[30m"); // Black
      
      CHECK(r && g, "\033[33m"); // Yellow
      CHECK(g && b, "\033[36m"); // Cyan
      CHECK(b && r, "\033[35m"); // Magenta
      
      CHECK(r, "\033[31m"); // Red
      CHECK(g, "\033[32m"); // Green
      CHECK(b, "\033[34m"); // Blue
      
      return os;
    }
#else
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& os, const Colour& colour) { return os; }
#endif
  }
}
