#pragma once
#define MULTI_THREADED_MANAGER
#ifdef MULTI_THREADED_MANAGER
#include <containers/containers.h>

namespace lambda
{
  namespace platform
  {
	namespace TaskScheduler
	{
	  enum Priority
	  {
	    kLow,
		kMedium,
		kHigh,
		kCritical,
		kCount,
	  };
	  extern void queue(Function<void(void*)> function, void* arguments, Priority priority);
	  void terminate();
	}
  }
}
#endif