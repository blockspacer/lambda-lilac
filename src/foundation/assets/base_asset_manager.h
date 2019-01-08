#pragma once
#include <containers/containers.h>
#include "package/package.h"

namespace lambda
{
  class VioletBaseAssetManager
  {
  public: // Not virtuals
    void SetMagicNumber(String magic_number);
    void SetGeneratedFilePath(String file_path);
    void SetFilePathIndex(String file_path);
    Vector<char> GetData(uint64_t hash) const;
    
    void Save() const;
    void Load();

  protected:
    void SaveData(Vector<char> data, uint64_t hash);
    Vector<char> LoadData(uint64_t index) const;
    void RemoveData(uint64_t index);

  private:
    String file_path_generated_;
    String file_path_index_;
    String magic_number_;
  };
}