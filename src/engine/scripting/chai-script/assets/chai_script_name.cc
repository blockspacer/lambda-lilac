#include "chai_script_name.h"

namespace lambda
{
  namespace scripting
  {
    ChaiName::ChaiName() : 
      name_(""), 
      hash_(0u) 
    {
    }
    ChaiName::ChaiName(const std::string& name) : 
      name_(name),
      hash_(hash(name_)) 
    {
    }
    ChaiName::ChaiName(const ChaiName& other) : 
      name_(other.name_), 
      hash_(other.hash_) 
    {
    }
    ChaiName::ChaiName(const Name& other) : 
      name_(stlString(other.getName())), 
      hash_(other.getHash()) 
    {
    }
    void ChaiName::operator=(const ChaiName& other) 
    { 
      hash_ = other.hash_; 
      name_ = other.name_; 
    }
    void ChaiName::operator=(const Name& other) 
    { 
      hash_ = other.getHash(); 
      name_ = stlString(other.getName()); 
    }
    ChaiName::operator Name() const 
    { 
      return Name(lmbString(name_)); 
    }
    std::string ChaiName::getName() const 
    { 
      return name_; 
    }
    size_t ChaiName::getHash() const 
    { 
      return hash_; 
    }
    bool ChaiName::operator==(const ChaiName& other) const 
    {
      return hash_ == other.hash_; 
    }
    bool ChaiName::operator!=(const ChaiName& other) const 
    { 
      return hash_ != other.hash_; 
    }
    void ChaiName::operator=(const size_t& reset) 
    { 
      hash_ = 0u; name_.clear(); 
    }
    void ChaiName::operator=(const std::string& str) 
    { 
      hash_ = hash(str); name_ = str; 
    }
  }
}