#include <gui/file_system.h>
#include <utils/file_system.h>
#include <utils/console.h>

#if VIOLET_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace lambda
{
	namespace gui
	{
		///////////////////////////////////////////////////////////////////////////
		MyFileSystem::MyFileSystem()
		{
		}
		
		///////////////////////////////////////////////////////////////////////////
		MyFileSystem::~MyFileSystem()
		{
		}
		
		///////////////////////////////////////////////////////////////////////////
		static String toString(const ultralight::String16& path)
		{
			WideString wstr(path.data(), path.data() + path.size());
			return lmbString(std::string(wstr.begin(), wstr.end()));
		}

		///////////////////////////////////////////////////////////////////////////
		static WideString toWideString(const String& path)
		{
			std::wstring wstr(path.begin(), path.end());
			return WideString(wstr.c_str(), wstr.size());
		}

		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::FileExists(const ultralight::String16& path)
		{
			return lambda::FileSystem::DoesFileExist(toString(path));
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::DeleteFile_(const ultralight::String16& path)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return false;
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::DeleteEmptyDirectory(const ultralight::String16& path)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return false;
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::MoveFile_(
			const ultralight::String16& old_path, 
			const ultralight::String16& new_path)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return false;
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::GetFileSize(
			const ultralight::String16& path, 
			int64_t& result)
		{
			result = (int64_t)lambda::FileSystem::GetFileSize(toString(path));
			return true;
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::GetFileSize(
			ultralight::FileHandle handle, 
			int64_t& result)
		{
			fseek((FILE*)handle, 0, SEEK_END);
			result = ftell((FILE*)handle);
			fseek((FILE*)handle, 0, SEEK_SET);
			
			return result >= 0;
		}

		///////////////////////////////////////////////////////////////////////////
		WideString GetMimeType(const WideString& szExtension)
		{
#if VIOLET_WIN32
			// return mime type for extension
			HKEY hKey = NULL;
			WideString szResult = L"application/unknown";

			// open registry key
			if (RegOpenKeyExW(HKEY_CLASSES_ROOT, szExtension.c_str(),
				0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{
				// define buffer
				wchar_t szBuffer[256] = { 0 };
				DWORD dwBuffSize = sizeof(szBuffer);

				// get content type
				if (RegQueryValueExW(hKey, L"Content Type", NULL, NULL,
					(LPBYTE)szBuffer, &dwBuffSize) == ERROR_SUCCESS)
				{
					// success
					szResult = szBuffer;
				}

				// close key
				RegCloseKey(hKey);
			}

			// return result
			return szResult;
#else
			return "";
#endif
		}

		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::GetFileMimeType(
			const ultralight::String16& path, 
			ultralight::String16& result)
		{
			String str = lambda::FileSystem::GetExtension(toString(path));
			WideString mimetype = GetMimeType(toWideString(str));
			result = ultralight::String16(mimetype.c_str(), mimetype.length());
			return true;
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::GetFileModificationTime(
			const ultralight::String16& path, 
			time_t& result)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return false;
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::GetFileCreationTime(
			const ultralight::String16& path, 
			time_t& result)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return false;
		}
		
		///////////////////////////////////////////////////////////////////////////
		ultralight::MetadataType MyFileSystem::GetMetadataType(
			const ultralight::String16& path)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return ultralight::MetadataType();
		}
		
		///////////////////////////////////////////////////////////////////////////
		ultralight::String16 MyFileSystem::GetPathByAppendingComponent(
			const ultralight::String16& path,
			const ultralight::String16& component)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return ultralight::String16();
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::CreateDirectory_(const ultralight::String16& path)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return false;
		}
		
		///////////////////////////////////////////////////////////////////////////
		ultralight::String16 MyFileSystem::GetHomeDirectory()
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return ultralight::String16();
		}
		
		///////////////////////////////////////////////////////////////////////////
		ultralight::String16 MyFileSystem::GetFilenameFromPath(
			const ultralight::String16& path)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return ultralight::String16();
		}
		
		///////////////////////////////////////////////////////////////////////////
		ultralight::String16 MyFileSystem::GetDirectoryNameFromPath(
			const ultralight::String16& path)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return ultralight::String16();
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::GetVolumeFreeSpace(
			const ultralight::String16& path, 
			uint64_t& result)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return false;
		}
		
		///////////////////////////////////////////////////////////////////////////
		int32_t MyFileSystem::GetVolumeId(const ultralight::String16& path)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return int32_t();
		}
		
		///////////////////////////////////////////////////////////////////////////
		ultralight::Ref<ultralight::String16Vector> MyFileSystem::ListDirectory(
			const ultralight::String16& path, 
			const ultralight::String16& filter)
		{
			ultralight::Ref<ultralight::String16Vector> entries = ultralight::String16Vector::Create();

			for (const auto& file : lambda::FileSystem::GetAllFilesInFolder(toString(path)))
				entries->push_back(ultralight::String16(file.c_str(), file.size()));

			return entries;
		}
		
		///////////////////////////////////////////////////////////////////////////
		ultralight::String16 MyFileSystem::OpenTemporaryFile(
			const ultralight::String16& prefix, 
			ultralight::FileHandle& handle)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return ultralight::String16();
		}
		
		///////////////////////////////////////////////////////////////////////////
		ultralight::FileHandle MyFileSystem::OpenFile(
			const ultralight::String16& path, 
			bool open_for_writing)
		{
			if (open_for_writing)
				return (ultralight::FileHandle)lambda::FileSystem::fopen(toString(path), "wb");
			else
				return (ultralight::FileHandle)lambda::FileSystem::fopen(toString(path), "rb");
		}
		
		///////////////////////////////////////////////////////////////////////////
		void MyFileSystem::CloseFile(ultralight::FileHandle& handle)
		{
			lambda::FileSystem::fclose((FILE*)handle);
		}
		
		///////////////////////////////////////////////////////////////////////////
		int64_t MyFileSystem::SeekFile(
			ultralight::FileHandle handle, 
			int64_t offset, 
			ultralight::FileSeekOrigin origin)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return int64_t();
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::TruncateFile(
			ultralight::FileHandle handle, 
			int64_t offset)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return false;
		}
		
		///////////////////////////////////////////////////////////////////////////
		int64_t MyFileSystem::WriteToFile(
			ultralight::FileHandle handle, 
			const char* data, 
			int64_t length)
		{
			return fwrite(data, sizeof(char), length, (FILE*)handle);
		}
		
		///////////////////////////////////////////////////////////////////////////
		int64_t MyFileSystem::ReadFromFile(
			ultralight::FileHandle handle, 
			char* data, 
			int64_t length)
		{
			return fread(data, sizeof(char), length, (FILE*)handle);
		}
		
		///////////////////////////////////////////////////////////////////////////
		bool MyFileSystem::CopyFile_(
			const ultralight::String16& source_path, 
			const ultralight::String16& destination_path)
		{
			LMB_ASSERT(false, "Not supported ATM.");
			return false;
		}
	}
}