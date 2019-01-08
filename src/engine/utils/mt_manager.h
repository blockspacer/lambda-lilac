#pragma once
#ifdef MULTI_THREADED_MANAGER
#include "primitives.h"
#include <functional>
#include <thread>
#include <mutex>

namespace lambda
{
  namespace platform
  {
    struct MTJob
    {
      friend class MTManager;
    public:
      MTJob(Function<void(void*)> function, void* data) :
        function_(function), data_(data), is_done_(false) {}

      void GetIsDone(bool& done)
      {
        mutex_.lock();
        done = is_done_;
        mutex_.unlock();
      }
      void SetIsDone(bool done)
      {
        mutex_.lock();
        is_done_ = done;
        mutex_.unlock();
      }

    protected:
      Function<void(void*)> function_;
      void* data_;
      std::mutex mutex_;
      bool is_done_;
    };

    class MTManager
    {
    private:
      class MTExecuter
      {
      public:
        MTExecuter()
        {
          state_ = 0;
          job_ = nullptr;
        }
        void execute(MTJob* job)
        {
          mutex_.lock();
          job_ = job;
          state_ = 1;
          mutex_.unlock();
        }

        void update()
        {
          mutex_.lock();
          if (state_ != 1)
          {
            mutex_.unlock();
            return;
          }
          mutex_.unlock();

          job_->function_(job_->data_);
          job_->SetIsDone(true);

          mutex_.lock();
          state_ = 0;
          mutex_.unlock();
        }

        void getState(char& state)
        {
          mutex_.lock();
          state = state_;
          mutex_.unlock();
        }

      private:
        MTJob* job_;
        char state_;
        std::mutex mutex_;
      };

    public:
      MTManager();
      ~MTManager();

      void queueFunction(MTJob* job);

      static MTManager& getInstance();
      static void terminate();

      void update();

    private:
      static void execExecuter(bool* keep_executing, MTExecuter* executer);
      static void execMTManager(void* data);

    private:
      bool continue_updating_ = true;
      std::mutex mutex_;
      static const uint8_t num_threads_ = 4u;
      Array<std::thread, num_threads_> threads_;
      Array<MTExecuter, num_threads_> executers_;
      static MTManager* g_instance_;
      Vector<MTJob*> jobs_;
    };
  }
}
#endif