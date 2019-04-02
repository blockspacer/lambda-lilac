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
	Vector<char> GetData(uint64_t hash) const;
	Vector<char> GetHeader(uint64_t hash) const;
	bool HasHeader(uint64_t hash) const;

    void Save() const;
    void Load();

  protected:
    void SaveData(const Vector<char>& data, uint64_t hash);
	void SaveHeader(const Vector<char>& data, uint64_t hash);
	Vector<char> LoadData(uint64_t index) const;
	Vector<char> LoadHeader(uint64_t index) const;
	void RemoveData(uint64_t hash);
	void RemoveHeader(uint64_t hash);

  private:
    String file_path_generated_;
    String magic_number_;
  };
}