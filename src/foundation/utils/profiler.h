#pragma once
#include "containers/containers.h"
#include "utils/timer.h"

namespace lambda
{
	namespace utilities
	{
		class Profiler
		{
		public:
			void startTimer(String name);
			void endTimer(String name);
			double getTime(String name) const;
			static Profiler& getInstance();
		
		private:
			static Profiler s_instance_;
			UnorderedMap<String, Timer> timers_;
			UnorderedMap<String, double> values_;
		};
	}
}