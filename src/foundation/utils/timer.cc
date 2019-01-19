#include "timer.h"

namespace lambda
{
  namespace utilities
  {
		///////////////////////////////////////////////////////////////////////////
    Timer::Timer(bool run)
    {
      if (run == true)
        reset();
    }
    
		///////////////////////////////////////////////////////////////////////////
    void Timer::reset()
    {
      start_ = high_resolution_clock::now();
    }
    
		///////////////////////////////////////////////////////////////////////////
    Time Timer::elapsed() const
    {
      return Time(
				std::chrono::duration_cast<nanoseconds>(
					high_resolution_clock::now() - start_
				)
			);
    }
    
		///////////////////////////////////////////////////////////////////////////
    Time::Time(std::chrono::nanoseconds ns) :
			ns_(ns)
    {
    }
    
		///////////////////////////////////////////////////////////////////////////
    double Time::seconds() const
    {
      return milliseconds() * 0.001;
    }
    
		///////////////////////////////////////////////////////////////////////////
    double Time::milliseconds() const
    {
      return microseconds() * 0.001;
    }

		///////////////////////////////////////////////////////////////////////////
		double Time::microseconds() const
		{
			return nanoseconds() * 0.001;
		}

		///////////////////////////////////////////////////////////////////////////
		double Time::nanoseconds() const
		{
			return (double)ns_.count();
		}
	}
}