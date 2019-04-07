#include "utils/profiler.h"

namespace lambda
{
	namespace utilities
	{
		///////////////////////////////////////////////////////////////////////////
		Profiler::~Profiler()
		{
		}

		///////////////////////////////////////////////////////////////////////////
		void Profiler::startTimer(String name)
		{
			timers_[name].reset();
		}

		///////////////////////////////////////////////////////////////////////////
		void Profiler::endTimer(String name)
		{
			values_[name] = timers_[name].elapsed().milliseconds();
		}

		///////////////////////////////////////////////////////////////////////////
		double Profiler::getTime(String name) const
		{
			auto it = values_.find(name);
			if (it == values_.end())
				return 0.0;
			else
				return it->second;
		}
	}
}