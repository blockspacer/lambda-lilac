#include "package.h"
#include "lz4.h"
#include <utils/file_system.h>
#include <utils/console.h>

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  String VioletPackage::GetMagicNumber() const
  {
    return header_.magic_number;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletPackage::SetMagicNumber(const String& magic_number)
  {
    memcpy(header_.magic_number, magic_number.c_str(), 3u);
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Vector<char> VioletPackage::GetData() const
  {
    return data_;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletPackage::SetData(const Vector<char>& data)
  {
    data_ = data;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  bool VioletPackage::getContainsIndex() const
  {
    return header_.contains_index;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  bool VioletPackage::getContainsData() const
  {
    return header_.contains_data;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletPackage::setContainsIndex(const bool & contains_index)
  {
    header_.contains_index = contains_index;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletPackage::setContainsData(const bool & contains_data)
  {
    header_.contains_data = contains_data;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletPackage::Load(const String& file)
  {
    if (!FileSystem::DoesFileExist(file))
    {
      foundation::Error("Could not open file: " + file + ".\n");
      return;
    }

    Vector<char> data = FileSystem::FileToVector(file);
    uint64_t offset = 0u;
    
    // TODO (Hilze): Actually do something with this.
    if (data.empty())
      return;

    // check if the file is actually filled.
    if (data.size() < sizeof(PackageHeader))
    {
      LMB_ASSERT(false, "Package: The file was too small.");
      return;
    }

    // Get the header.
    PackageHeader header;
    memcpy(&header, data.data() + offset, sizeof(PackageHeader));
    offset += sizeof(PackageHeader);

    // Get the magic header.
    if (header.magic_number[0u] != header_.magic_number[0u] ||
        header.magic_number[1u] != header_.magic_number[1u] ||
        header.magic_number[2u] != header_.magic_number[2u])
    {
      LMB_ASSERT(false, "Package: Magic number was different. File was probably corrupted.");
      return;
    }
    header_ = header;

    // Remove previously used data from the buffer.
    data.erase(data.begin(), data.begin() + offset);

    for (unsigned char i = 0u; i < header_.compression_level; ++i)
    {
      Vector<char> old_data = data;

      header_.size = (uint64_t)LZ4_compressBound((int)header.size);
      // Decompress the data.
      data.resize(header.size, '\0');
      int res = LZ4_decompress_safe(old_data.data(), data.data(), (int)old_data.size(), (int)data.size());

      if (res < 0)
      {
        LMB_ASSERT(false, "Package: Could not decompress the package.");
        return;
      }
      else
      {
        data.resize((uint64_t)res);
      }
    }
    data_ = data;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void VioletPackage::Save(const String& file) const
  {
    Vector<char> out_data((uint64_t)LZ4_compressBound((int)data_.size()), '\0');
    out_data.resize(LZ4_compress_default(data_.data(), out_data.data(), (int)data_.size(), (int)out_data.size()));
    int compression = (int)((1.0f - (float)out_data.size() / (float)data_.size()) * 100.0f);

    if (compression <= 0)
    {
      out_data = data_;
      compression = 0;
    }

    //LMB_LOG_DEBG("Written to: \"%s\"\n", file.c_str());
    //LMB_LOG_DEBG("  Compressed by: %s%%\n", compression);

    PackageHeader header = header_;
    header.compression_level = compression > 0 ? 1u : 0u;
    header.size = (uint32_t)data_.size();
    FileSystem::WriteFile(file, out_data.data(), out_data.size(), (char*)&header, sizeof(header));
  }
}