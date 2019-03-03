#include "texture_compiler.h"
#include <utils/file_system.h>
#include <utils/utilities.h>
#include <utils/console.h>
#include <memory/memory.h>

#define STBI_MALLOC(sz)           lambda::foundation::Memory::allocate(sz)
#define STBI_REALLOC(p,newsz)     lambda::foundation::Memory::reallocate(p,newsz)
#define STBI_FREE(p)              lambda::foundation::Memory::deallocate(p)
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>

namespace lambda
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  VioletTextureCompiler::VioletTextureCompiler() :
    VioletTextureManager()
  {
  }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int calcImageSize(int w, int h, int mips)
	{
		int data_size = 0;
		for (int i = 0; i < mips; ++i)
		{
			data_size += w * h * 4;
			w = std::max(1, w / 2);
			h = std::max(1, h / 2);
		}
		return data_size;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void genMips(int w, int h, int mips, unsigned char* data)
	{
		int offset = 0;

		for (int i = 0; i < mips - 1; ++i)
		{
			int new_width = std::max(1, w / 2);
			int new_height = std::max(1, h / 2);
			int new_offset = offset + w * h * 4;

			stbir_resize_uint8(
				data + offset,
				w,
				h,
				0,
				data + new_offset,
				new_width,
				new_height,
				0,
				STBI_rgb_alpha
			);

			w = new_width;
			h = new_height;
			offset = new_offset;
		}
	}

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool VioletTextureCompiler::Compile(TextureCompileInfo texture_info)
  {
		String extension = FileSystem::GetExtension(texture_info.file);
		Vector<char> raw_data;
		int w, h, num_mips;
		bool contains_alpha = false;
		TextureFormat format;
		Vector<char> raw_texture = FileSystem::FileToVector(texture_info.file);

		if (stbi_is_hdr_from_memory((unsigned char*)raw_texture.data(), (int)raw_texture.size()))
		{
			format = TextureFormat::kR32G32B32A32;

			int bpp;
			float* data = stbi_loadf_from_memory((unsigned char*)raw_texture.data(), (int)raw_texture.size(), &w, &h, &bpp, STBI_rgb_alpha);

			if (data == nullptr)
			{
				foundation::Error("Failed to load texture: " + texture_info.file + "\n");
				foundation::Error("STBI Error: " + String(stbi_failure_reason()) + "\n");
				return false;
			}

			for (int i = 0; i < w * h * 4; i += 4)
			{
				if (data[i + 3] < 1.0f)
				{
					contains_alpha = true;
					break;
				}
			}

			num_mips = 1;

			raw_data.resize(w * h * 4 * sizeof(float)* num_mips);
			memcpy(raw_data.data(), data, w * h * 4 * sizeof(float));
			stbi_image_free(data);
		}
		else
		{
			format = TextureFormat::kR8G8B8A8;

			int bpp;
			uint8_t* data = stbi_load_from_memory((unsigned char*)raw_texture.data(), (int)raw_texture.size(), &w, &h, &bpp, STBI_rgb_alpha);

			if (data == nullptr)
			{
				foundation::Error("Failed to load texture: " + texture_info.file + "\n");
				foundation::Error("STBI Error: " + String(stbi_failure_reason()) + "\n");
				return false;
			}

			for (int i = 0; i < w * h * 4; i += 4)
			{
				if (data[i + 3] < UINT8_MAX)
				{
					contains_alpha = true;
					break;
				}
			}

			num_mips = (int)floorf(std::log2f(std::fminf((float)w, (float)h)) + 1.0f);

			int data_size = calcImageSize(w, h, num_mips);
			raw_data.resize(data_size);
			memcpy(raw_data.data(), data, w * h * 4);
			stbi_image_free(data);
			genMips(w, h, num_mips, (unsigned char*)raw_data.data());
		}

		VioletTexture texture;
		texture.hash      = GetHash(texture_info.file);
		texture.file      = texture_info.file;
		texture.format    = format;
		texture.width     = (uint32_t)w;
		texture.height    = (uint32_t)h;
		texture.mip_count = num_mips;
		texture.flags     = kTextureFlagFromDDS | (contains_alpha ? kTextureFlagContainsAlpha : 0);
		texture.data      = raw_data;
		AddTexture(texture);

		return true;
  }
}