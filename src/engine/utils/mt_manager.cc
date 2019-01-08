#include "mt_manager.h"

#ifdef MULTI_THREADED_MANAGER
namespace lambda
{
  namespace platform
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MTManager* MTManager::g_instance_ = nullptr;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MTManager::MTManager()
    {
      for (uint8_t i = 0u; i < num_threads_; ++i)
      {
        threads_.at(i) = std::thread(execExecuter,&continue_updating_,&executers_.at(i));
      }

      MTJob* job = foundation::Memory::construct<MTJob>(execMTManager, this);
      queueFunction(job);
      update();
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MTManager::~MTManager()
    {
    }
   
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MTManager::queueFunction(MTJob* job)
    {
      mutex_.lock();
      jobs_.push_back(job);
      mutex_.unlock();
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MTManager& MTManager::getInstance()
    {
      if (g_instance_ == nullptr)
      {
        g_instance_ = foundation::Memory::construct<MTManager>();
      }
      return *g_instance_;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MTManager::terminate()
    {
      g_instance_->mutex_.lock();
      g_instance_->continue_updating_ = false;
      g_instance_->mutex_.unlock();
      
      for (uint8_t i = 0u; i < g_instance_->num_threads_; ++i)
      {
        if (g_instance_->threads_.at(i).joinable())
        {
          g_instance_->threads_.at(i).join();
        }
      }

      foundation::Memory::destruct(g_instance_);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MTManager::update()
    {
      mutex_.lock();

      if (jobs_.empty())
      {
        mutex_.unlock();
        return;
      }
      mutex_.unlock();

      for (uint8_t i = 0u; i < num_threads_; ++i)
      {
        mutex_.lock();
        char state;
        if (executers_.at(i).getState(state), state == 0)
        {
          executers_.at(i).execute(jobs_.front());
          jobs_.erase(jobs_.begin());
        }
        mutex_.unlock();

        mutex_.lock();
        if (jobs_.empty())
        {
          mutex_.unlock();
          return;
        }
        mutex_.unlock();
      }
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MTManager::execExecuter(bool* keep_executing, MTExecuter* executer)
    {
      while (*keep_executing)
      {
        executer->update();
      }
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void MTManager::execMTManager(void* data)
    {
      MTManager* mt_manager = (MTManager*)data;
      while (mt_manager->continue_updating_)
      {
        mt_manager->update();
      }
    }
  }
}
#endif