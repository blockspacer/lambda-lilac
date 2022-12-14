#pragma once
#include <Ultralight/platform/FileSystem.h>

namespace lambda
{
  namespace gui
  {
		///////////////////////////////////////////////////////////////////////////
		///// MY FILE SYSTEM //////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////
		class MyFileSystem
			: public ultralight::FileSystem
		{
		public:
			MyFileSystem();
			virtual ~MyFileSystem() override;

			virtual bool FileExists(
				const ultralight::String16& path) override;

			virtual bool DeleteFile_(
				const ultralight::String16& path) override;

			virtual bool DeleteEmptyDirectory(
				const ultralight::String16& path) override;

			virtual bool MoveFile_(
				const ultralight::String16& old_path, 
				const ultralight::String16& new_path) override;

			virtual bool GetFileSize(
				const ultralight::String16& path,
				int64_t& result) override;

			virtual bool GetFileSize(
				ultralight::FileHandle handle, 
				int64_t& result) override;

			virtual bool GetFileMimeType(
				const ultralight::String16& path,
				ultralight::String16& result) override;

			virtual bool GetFileModificationTime(
				const ultralight::String16& path, 
				time_t& result) override;

			virtual bool GetFileCreationTime(
				const ultralight::String16& path, 
				time_t& result) override;

			virtual ultralight::MetadataType GetMetadataType(
				const ultralight::String16& path) override;

			virtual ultralight::String16 GetPathByAppendingComponent(
				const ultralight::String16& path,
				const ultralight::String16& component) override;

			virtual bool CreateDirectory_(
				const ultralight::String16& path) override;

			virtual ultralight::String16 GetHomeDirectory() override;

			virtual ultralight::String16 GetFilenameFromPath(
				const ultralight::String16& path) override;

			virtual ultralight::String16 GetDirectoryNameFromPath(
				const ultralight::String16& path) override;

			virtual bool GetVolumeFreeSpace(
				const ultralight::String16& path, 
				uint64_t& result) override;

			virtual int32_t GetVolumeId(
				const ultralight::String16& path) override;

			virtual ultralight::Ref<ultralight::String16Vector> ListDirectory(
				const ultralight::String16& path, 
				const ultralight::String16& filter) override;

			virtual ultralight::String16 OpenTemporaryFile(
				const ultralight::String16& prefix, 
				ultralight::FileHandle& handle) override;

			virtual ultralight::FileHandle OpenFile(
				const ultralight::String16& path, 
				bool open_for_writing) override;

			virtual void CloseFile(ultralight::FileHandle& handle) override;

			virtual int64_t SeekFile(
				ultralight::FileHandle handle, 
				int64_t offset, 
				ultralight::FileSeekOrigin origin) override;

			virtual bool TruncateFile(
				ultralight::FileHandle handle, 
				int64_t offset) override;

			virtual int64_t WriteToFile(
				ultralight::FileHandle handle, 
				const char* data, 
				int64_t length) override;

			virtual int64_t ReadFromFile(
				ultralight::FileHandle handle, 
				char* data, 
				int64_t length) override;

			virtual bool CopyFile_(
				const ultralight::String16& source_path, 
				const ultralight::String16& destination_path) override;
		};
  }
}
