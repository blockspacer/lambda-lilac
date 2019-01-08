#pragma once
#include <containers/containers.h>

namespace lambda
{
  class Name
  {
  public:
    Name() :
      name_(""), hash_(0u)
    {
#if defined _DEBUG || defined VIOLET_OSX
      dbg_cstr_ = nullptr;
#endif
    }
    Name(const String& name) :
      name_(name), hash_(hash(name_))
    {
#if defined _DEBUG || defined VIOLET_OSX
      dbg_cstr_ = name_.c_str();
#endif
    }
    Name(const Name& other) :
      name_(other.name_), hash_(other.hash_)
    {
#if defined _DEBUG || defined VIOLET_OSX
      dbg_cstr_ = name_.c_str();
#endif
    }
    void operator=(const Name& other)
    {
      hash_ = other.hash_;
      name_ = other.name_;
#if defined _DEBUG || defined VIOLET_OSX
      dbg_cstr_ = name_.c_str();
#endif
    }

    const String& getName() const
    {
      return name_;
    }
    size_t getHash() const
    {
      return hash_;
    }

    bool operator==(const Name& other) const
    {
      return hash_ == other.hash_;
    }
    bool operator!=(const Name& other) const
    {
      return hash_ != other.hash_;
    }
    void operator=(const size_t& reset)
    {
      hash_ = 0u;
      name_.clear();
#if defined _DEBUG || defined VIOLET_OSX
      dbg_cstr_ = nullptr;
#endif
    }
    void operator=(const String& str)
    {
      hash_ = hash(str);
      name_ = str;
#if defined _DEBUG || defined VIOLET_OSX
      dbg_cstr_ = name_.c_str();
#endif
    }

  private:
    String name_;
    size_t hash_;
#if defined _DEBUG || defined VIOLET_OSX
    const char* dbg_cstr_;
#endif
  };
}

namespace eastl
{
  template <>
  struct hash<lambda::Name>
  {
    size_t operator()(const lambda::Name& x) const
    {
      return x.getHash();
    }
  };
  template <>
  struct equal_to<lambda::Name>
  {
    bool operator()(const lambda::Name& a, const lambda::Name& b) const
    {
      return a.getHash() == b.getHash();
    }
  };
  template <>
  struct less<lambda::Name>
  {
    bool operator()(const lambda::Name& a, const lambda::Name& b) const
    {
      return a.getHash() < b.getHash();
    }
  };
  template <>
  struct less_equal<lambda::Name>
  {
    bool operator()(const lambda::Name& a, const lambda::Name& b) const
    {
      return a.getHash() <= b.getHash();
    }
  };
  template <>
  struct greater<lambda::Name>
  {
    bool operator()(const lambda::Name& a, const lambda::Name& b) const
    {
      return a.getHash() > b.getHash();
    }
  };
  template <>
  struct greater_equal<lambda::Name>
  {
    bool operator()(const lambda::Name& a, const lambda::Name& b) const
    {
      return a.getHash() >= b.getHash();
    }
  };
}
