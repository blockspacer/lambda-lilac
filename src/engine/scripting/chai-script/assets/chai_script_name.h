#pragma once
#include "utils/name.h"

namespace lambda
{
  namespace scripting
  {
    class ChaiName
    {
    public:
      ChaiName();
      ChaiName(const std::string& name);
      ChaiName(const ChaiName& other);
      ChaiName(const Name& other);
      void operator=(const ChaiName& other);
      void operator=(const Name& other);
      operator Name() const;
      std::string getName() const;
      size_t getHash() const;
      bool operator==(const ChaiName& other) const;
      bool operator!=(const ChaiName& other) const;
      void operator=(const size_t& reset);
      void operator=(const std::string& str);

    private:
      std::string name_;
      size_t hash_;
    };
  }
}