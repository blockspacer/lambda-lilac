#pragma once

namespace lambda
{
  namespace io
  {
    ///////////////////////////////////////////////////////////////////////////
    template<typename State>
    class Input
    {
    public:
      void update(const State& state);

      unsigned char getStateCount() const;
      const State& getState(unsigned char idx) const;
      const State& getCurrentState() const;
      const State& getPreviousState() const;

    private:
      static const unsigned char max_state_count_ = 2u;
      State states_[max_state_count_];
      unsigned char current_state_index_ = 0u;
    };

    ///////////////////////////////////////////////////////////////////////////
    template<typename State>
    inline void Input<State>::update(const State& state)
    {
      if (++current_state_index_ >= max_state_count_)
      {
        current_state_index_ = 0;
      }
      states_[current_state_index_] = state;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template<typename State>
    inline unsigned char Input<State>::getStateCount() const
    {
      return this->max_state_count_; // TODO (Hilze): Find out if this is wrong.
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template<typename State>
    inline const State& Input<State>::getState(unsigned char idx) const
    {
      return states_[idx];
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template<typename State>
    inline const State& Input<State>::getCurrentState() const
    {
      return states_[current_state_index_];
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template<typename State>
    inline const State& Input<State>::getPreviousState() const
    {
      return states_[(current_state_index_ == 0u) ?
        (max_state_count_ - 1u) : (current_state_index_ - 1u)];
    }
  }
}