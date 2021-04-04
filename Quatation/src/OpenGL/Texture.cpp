#include "Texture.h"

#include <cmp_compressonatorlib/common.h>
#include <cgltf.h>
#include <stb_image.h>
#include <gli/gli.hpp>
#include <glad/glad.h>

namespace choice
{
	const std::string CompressTexture(const std::string& srcFile, const std::string& dstDirectory, 
		BlockCompressionFormat format, bool generateMips)
	{
		CMP_MipSet srcMipSet = {};
		int cmp_status = CMP_LoadTexture(srcFile.c_str(), &srcMipSet);
		if (cmp_status != CMP_OK)
		{
			std::cout << "Failed To Load Texture" << std::endl;
			return {};
		}

		if (generateMips && srcMipSet.m_nMipLevels == 1)
		{
			int minSize = CMP_CalcMinMipSize(srcMipSet.m_nHeight, srcMipSet.m_nWidth, srcMipSet.m_nMaxMipLevels + 1);
			cmp_status = CMP_GenerateMIPLevels(&srcMipSet, minSize);
			if (cmp_status != CMP_OK)
			{
				std::cout << "Failed To Generate Mips" << std::endl;
				return {};
			}
		}

		KernelOptions kerneloptions = {};
		kerneloptions.format = (CMP_FORMAT)format;
		kerneloptions.fquality = 0.1f;

		CMP_MipSet dstMipSet = {};
		cmp_status = CMP_ProcessTexture(&srcMipSet, &dstMipSet, kerneloptions, nullptr);
		if (cmp_status != CMP_OK)
		{
			std::cout << "Failed To Process Texture" << std::endl;
			return {};
		}

		std::string dstFile = dstDirectory + srcFile.substr(srcFile.find_last_of('\\') , srcFile.size());

		CMP_SaveTexture(dstFile.c_str(), &dstMipSet);

		CMP_FreeMipSet(&srcMipSet);
		CMP_FreeMipSet(&dstMipSet);

		return dstFile;
	}

	const uint32_t LoadTexture2D(Texture2DData data)
	{
		gli::texture _texture = gli::load(data.Source);
		if (_texture.empty())
		{
			std::cout << "Failed To Load Compressed Texture" << std::endl;
			return 0;
		}

		gli::gl _gl(gli::gl::PROFILE_GL33);
		gli::gl::format const _format = _gl.translate(_texture.format(), _texture.swizzles());
		GLenum _target = _gl.translate(_texture.target());

		if (!gli::is_compressed(_texture.format()))
		{
			std::cout << "Texture Is Not Compressed" << std::endl;
			return 0;
		}

		if (_texture.target() != gli::TARGET_2D)
		{
			std::cout << "Texture Is Not 2D" << std::endl;
			return 0;
		}

		uint32_t Id;
		glCreateTextures(_target, 1, &Id);
		glBindTexture(_target, Id);
		glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, data.minFilter);
		glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, data.magFilter);
		glTexParameteri(_target, GL_TEXTURE_WRAP_S, data.wrapS);
		glTexParameteri(_target, GL_TEXTURE_WRAP_T, data.wrapT);
		glTexParameteri(_target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(_target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(_texture.levels() - 1));
		glTexParameteriv(_target, GL_TEXTURE_SWIZZLE_RGBA, &_format.Swizzles[0]);
		glTexStorage2D(_target, static_cast<GLint>(_texture.levels()), _format.Internal,
			_texture.extent().x, _texture.extent().y);

		for (std::size_t Level = 0; Level < _texture.levels(); ++Level)
		{
			glm::tvec3<GLsizei> Extent(_texture.extent(Level));
			glCompressedTexSubImage2D(
				_target, static_cast<GLint>(Level), 0, 0, Extent.x, Extent.y,
				_format.Internal, static_cast<GLsizei>(_texture.size(Level)), _texture.data(0, 0, Level));
		}

		return Id;
	}

	CMP_ERROR LoadTextureFromMemory(void* data, int width, int height, CMP_MipSet* MipSetIn)
	{
		CMP_RegisterHostPlugins();
		CMP_CMIPS CMips;

		memset(MipSetIn, 0, sizeof(CMP_MipSet));
		if (!CMips.AllocateMipSet(MipSetIn, CF_8bit, TDT_ARGB, TT_2D, width, height, 1))
		{
			return CMP_ERR_MEM_ALLOC_FOR_MIPSET;
		}

		if (!CMips.AllocateMipLevelData(CMips.GetMipLevel(MipSetIn, 0), width, height, CF_8bit, TDT_ARGB))
		{
			return CMP_ERR_MEM_ALLOC_FOR_MIPSET;
		}

		MipSetIn->m_nMipLevels = 1;
		MipSetIn->m_format = CMP_FORMAT_RGBA_8888;

		CMP_BYTE* pData = CMips.GetMipLevel(MipSetIn, 0)->m_pbData;
		CMP_DWORD dwPitch = 4 * MipSetIn->m_nWidth;
		CMP_DWORD dwSize = dwPitch * MipSetIn->m_nHeight;

		memcpy(pData, data, dwSize);
		MipSetIn->pData = pData;
		MipSetIn->dwDataSize = dwSize;

		return CMP_OK;
	}

	const std::string CompressTexture(void* data, const std::string& dstDirectory, BlockCompressionFormat format, bool generateMips)
	{
		const auto* texture = reinterpret_cast<const cgltf_texture*>(data);
		std::string name = texture->image->name;

		const auto* buffer = texture->image->buffer_view->buffer;
		if (buffer)
		{
			int x, y, channels;
			auto* data = stbi_load_from_memory((stbi_uc*)buffer->data + texture->image->buffer_view->offset,
				buffer->size, &x, &y, &channels, STBI_rgb_alpha);
			if (!data)
			{
				std::cout << "Failed To Load Texture" << std::endl;
				return {};
			}

			CMP_MipSet srcMipSet = {};
			int cmp_status = LoadTextureFromMemory(data, x, y, &srcMipSet);
			if (cmp_status != CMP_OK)
			{
				std::cout << "Failed To Load Texture" << std::endl;
				return {};
			}
			stbi_image_free(data);

			if (generateMips && srcMipSet.m_nMipLevels == 1)
			{
				int minSize = CMP_CalcMinMipSize(srcMipSet.m_nHeight, srcMipSet.m_nWidth, srcMipSet.m_nMaxMipLevels + 1);
				cmp_status = CMP_GenerateMIPLevels(&srcMipSet, minSize);
				if (cmp_status != CMP_OK)
				{
					std::cout << "Failed To Generate Mips" << std::endl;
					return {};
				}
			}

			KernelOptions kerneloptions = {};
			kerneloptions.format = (CMP_FORMAT)format;
			kerneloptions.fquality = 0.1f;

			CMP_MipSet dstMipSet = {};
			cmp_status = CMP_ProcessTexture(&srcMipSet, &dstMipSet, kerneloptions, nullptr);
			if (cmp_status != CMP_OK)
			{
				std::cout << "Failed To Process Texture" << std::endl;
				return {};
			}

			std::string dstFile = dstDirectory + "\\" + name + ".dds";

			CMP_SaveTexture(dstFile.c_str(), &dstMipSet);

			CMP_FreeMipSet(&srcMipSet);
			CMP_FreeMipSet(&dstMipSet);

			return dstFile;
		}
		return {};
	}
} 