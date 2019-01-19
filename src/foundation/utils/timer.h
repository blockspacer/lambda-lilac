#pragma once
#include <chrono>
#include <ostream>

namespace lambda
{
  namespace utilities
  {
		///////////////////////////////////////////////////////////////////////////
    class Time
    {
    public:
      Time(std::chrono::nanoseconds ns);

      double seconds() const;
			double milliseconds() const;
			double microseconds() const;
			double nanoseconds() const;

    private:
			std::chrono::nanoseconds ns_;
    };

		///////////////////////////////////////////////////////////////////////////
		class Timer
    {
      typedef std::chrono::high_resolution_clock high_resolution_clock;
      typedef std::chrono::nanoseconds nanoseconds;
    public:
      explicit Timer(bool run = true);
      void reset();
      Time elapsed() const;

      template <typename T, typename Traits>
      friend std::basic_ostream<T, Traits>& 
				operator<<(std::basic_ostream<T, Traits>& out, const Timer& timer);
    private:
      high_resolution_clock::time_point start_;
    };

    template<typename T, typename Traits>
    std::basic_ostream<T, Traits>& 
			operator<<(std::basic_ostream<T, Traits>& out, const Timer& timer)
    {
      return out << timer.elapsed().seconds();
    }
  }
}