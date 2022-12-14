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
			~Profiler();
			void startTimer(String name);
			void endTimer(String name);
			double getTime(String name) const;
		
		private:
			UnorderedMap<String, Timer> timers_;
			UnorderedMap<String, double> values_;
		};
	}
}