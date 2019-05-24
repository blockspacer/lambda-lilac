#include "mt_manager.h"
#include <memory/memory.h>

#ifdef MULTI_THREADED_MANAGER
namespace lambda
{
	namespace platform
	{
		namespace TaskScheduler
		{
			struct FunctionArgument
			{
				Function<void(void*)> function;
				void* argument;
			};
			bool k_alive = true;
			Queue<FunctionArgument> k_functions[Priority::kCount];
			std::atomic<int> k_num_functions[Priority::kCount];
			std::mutex k_lock[Priority::kCount];
			std::thread k_worker_threads[Priority::kCount];

			void executeFunctions(Priority priority)
			{
				while (true)
				{
					Priority selected_priority = Priority::kCount;
					for (int8_t i = Priority::kCount - 1; i >= priority; --i)
					{
						if (k_num_functions[i] > 0)
						{
							selected_priority = (Priority)i;
							break;
						}
					}

					if (selected_priority == Priority::kCount && k_alive)
					{
						std::this_thread::sleep_for(std::chrono::microseconds(1));
						continue;
					}

					if (!k_alive)
						return;

					k_lock[selected_priority].lock();

					// Damn.. we just missed it..
					if (k_num_functions[selected_priority] <= 0)
					{
						k_lock[selected_priority].unlock();
						continue;
					}

					k_num_functions[selected_priority]--;
					auto function = k_functions[selected_priority].front();
					k_functions[selected_priority].pop();
					k_lock[selected_priority].unlock();

					function.function(function.argument);
				}
			}

			void queue(Function<void(void*)> function, void* arguments, Priority priority)
			{
				k_lock[priority].lock();
				k_functions[priority].push({ function, arguments });
				k_num_functions[priority]++;
				k_lock[priority].unlock();

				if (!k_worker_threads[0].joinable())
				{
					for (uint8_t i = 0u; i < Priority::kCount; ++i)
					{
						k_worker_threads[i] = std::thread(executeFunctions, (Priority)i);
					}
				}
			}

			void terminate()
			{
				k_alive = false;

				for (uint8_t i = 0u; i < Priority::kCount; ++i)
				{
					k_worker_threads[i].join();
					k_functions[i] = {};
					k_num_functions[i] = 0;
				}
			}
		}
	}
}
#endif