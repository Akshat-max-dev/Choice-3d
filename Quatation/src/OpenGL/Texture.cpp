#include "Texture.h"

#include <cmp_compressonatorlib/common.h>
#include <cgltf.h>
#include <stb_image.h>
#include <gli/gli.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <tinyexr.h>

#include "Shader.h"
#include "Project/Scene/SceneObject/Property/Drawable.h"

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

		std::string dstFile = dstDirectory + srcFile.substr(srcFile.find_last_of('\\'), srcFile.size());

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

	const uint32_t LoadTextureCubemap(const std::string& hdrmap)
	{
		uint32_t framebuffer;
		glCreateFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		std::string ext = hdrmap.substr(hdrmap.find_last_of('.'), hdrmap.size());

		int width, height, components;
		float* data = nullptr;

		GLenum internalformat = GL_NONE;
		GLenum dataformat = GL_NONE;

		if (ext == ".hdr")
		{
			internalformat = GL_RGB16F;
			dataformat = GL_RGB;
			stbi_set_flip_vertically_on_load(1);
			data = stbi_loadf(hdrmap.c_str(), &width, &height, &components, 0);
		}
		else if (ext == ".exr")
		{
			internalformat = GL_RGBA16F;
			dataformat = GL_RGBA;
			const char* err = nullptr;
			int ret = LoadEXR(&data, &width, &height, hdrmap.c_str(), &err);
			if (ret != TINYEXR_SUCCESS)
			{
				if (err) { FreeEXRErrorMessage(err); }
			}
		}

		uint32_t hdr;
		if (!data)
		{
			std::cout << "Failed To Load HDR Image" << std::endl;
			return 0;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &hdr);
		glBindTexture(GL_TEXTURE_2D, hdr);
		glTextureStorage2D(hdr, 1, internalformat, width, height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureSubImage2D(hdr, 0, 0, 0, width, height, dataformat, GL_FLOAT, data);
		if (ext == ".hdr") { stbi_image_free(data); }
		else if (ext == ".exr") { free(data); }

		uint32_t hdrcubemap;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &hdrcubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, hdrcubemap);
		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalformat, 1024, 1024, 0, dataformat, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), (float)1024 / (float)1024, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		std::unique_ptr<Drawable> cube(LoadDrawable("", DrawableType::CUBE, false));
		std::unique_ptr<Shader> hdrToCubemap = std::make_unique<Shader>("Choice/assets/shaders/HDRToCubemap.glsl");

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdr);

		hdrToCubemap->Use();
		hdrToCubemap->Int("hdrMap", 0);
		hdrToCubemap->Mat4("uProjection", captureProjection);

		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		for (uint32_t i = 0; i < 6; i++)
		{
			hdrToCubemap->Mat4("uView", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, hdrcubemap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			cube->GetMeshes()[0].first->Bind();
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return hdrcubemap;
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
			stbi_set_flip_vertically_on_load(0);
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