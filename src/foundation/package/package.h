#pragma once
#include <containers/containers.h>

namespace lambda
{
  class VioletPackage
  {
  public:
    String GetMagicNumber() const;
    void SetMagicNumber(const String& magic_number);
    Vector<char> GetData() const;
    void SetData(const Vector<char>& data);
    bool getContainsIndex() const;
    bool getContainsData() const;
    void setContainsIndex(const bool& contains_index);
    void setContainsData(const bool& contains_data);
    void Load(const String& file);
    void Save(const String& file) const;
    
  private:
    Vector<char> data_;

    struct PackageHeader
    {
      char magic_number[3u];
      bool contains_index = false;
      bool contains_data  = false;
      unsigned char compression_level = 0u;
      uint32_t size = 0u;
    };
    PackageHeader header_;
  };
}