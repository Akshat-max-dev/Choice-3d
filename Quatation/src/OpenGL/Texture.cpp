#include "Texture.h"

#include <cmp_compressonatorlib/common.h>
#include <cgltf.h>
#include <stb_image.h>
#include <gli/gli.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <tinyexr.h>

#include "Shader.h"
#include "Project/Scene/Nodes/Mesh.h"
#include "Error.h"

namespace choice
{
	const std::string CompressTexture(const std::string& srcFile,
		BlockCompressionFormat format, bool generateMips)
	{
		std::string dstFile = global::ActiveSceneDir + "Assets\\" + srcFile.substr(srcFile.find_last_of('\\'), srcFile.size()) + ".dds";
		if (ghc::filesystem::exists(dstFile)) { return dstFile; }

		CMP_MipSet srcMipSet = {};
		int cmp_status = CMP_LoadTexture(srcFile.c_str(), &srcMipSet);
		if (cmp_status != CMP_OK)
		{
			Message<WARNING>("[COMPRESSONATOR] Failed To Load Texture", MESSAGE_ORIGIN::PIPELINE);
			return {};
		}

		if (generateMips && srcMipSet.m_nMipLevels == 1)
		{
			int minSize = CMP_CalcMinMipSize(srcMipSet.m_nHeight, srcMipSet.m_nWidth, srcMipSet.m_nMaxMipLevels + 1);
			cmp_status = CMP_GenerateMIPLevels(&srcMipSet, minSize);
			if (cmp_status != CMP_OK)
			{
				Message<WARNING>("[COMPRESSONATOR] Failed To Generate Mip-Maps", MESSAGE_ORIGIN::PIPELINE);
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
			Message<WARNING>("[COMPRESSONATOR] Failed To Compress Texture", MESSAGE_ORIGIN::PIPELINE);
			return {};
		}

		CMP_SaveTexture(dstFile.c_str(), &dstMipSet);

		CMP_FreeMipSet(&srcMipSet);
		CMP_FreeMipSet(&dstMipSet);

		return dstFile;
	}

	const uint32_t LoadTexture2D(const std::string& ddsFile, bool usetrilinearfiltering)
	{
		gli::texture _texture = gli::load(ddsFile);
		if (_texture.empty())
		{
			Message<WARNING>("[GLI] Failed To Load Compressed Texture", MESSAGE_ORIGIN::PIPELINE);
			return 0;
		}

		gli::gl _gl(gli::gl::PROFILE_GL33);
		gli::gl::format const _format = _gl.translate(_texture.format(), _texture.swizzles());
		GLenum _target = _gl.translate(_texture.target());

		if (!gli::is_compressed(_texture.format()))
		{
			Message<WARNING>((ddsFile + " " + "Is Not Compressed Texture").c_str(), MESSAGE_ORIGIN::PIPELINE);
			return 0;
		}

		if (_texture.target() != gli::TARGET_2D)
		{
			Message<WARNING>((ddsFile + " " + "Is Not A 2D Texture").c_str(), MESSAGE_ORIGIN::PIPELINE);
			return 0;
		}

		uint32_t Id;
		glCreateTextures(_target, 1, &Id);
		glBindTexture(_target, Id);
		GLenum minfilter = usetrilinearfiltering ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
		glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, minfilter);
		glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

	const std::vector<uint32_t> LoadTextureCubemap(const std::string& hdrmap)
	{
		std::vector<uint32_t> Ids;

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
			Message<WARNING>("Failed To Load HDRI Image", MESSAGE_ORIGIN::PIPELINE);
			Ids.push_back(0); Ids.push_back(0); Ids.push_back(0); Ids.push_back(0);
			return Ids;
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

		// enable seamless cubemap sampling for lower mip levels in the pre-filter map.
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //Enable TriLinear Filtering
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

		std::unique_ptr<Mesh> cube(Cube());
		//Delete Unwanted Data In Cube
		delete cube->NodeTransform; cube->NodeTransform = {};
		delete cube->materials[0]; cube->materials[0] = {};

		ReflectionData& reflectiondata = global::GlobalReflectionData;

		std::unique_ptr<Shader> shader = std::make_unique<Shader>("Choice/assets/shaders/HDRToCubemap.glsl");

		glBindTextureUnit(reflectiondata.Samplers["hdrMap"], hdr);

		shader->Use();
		reflectiondata.UniformBuffers["Camera"]->SetData("Camera.Projection", &captureProjection);

		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		for (uint32_t i = 0; i < 6; i++)
		{
			reflectiondata.UniformBuffers["Camera"]->SetData("Camera.View", &captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, hdrcubemap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			cube->vertexarray->Bind();
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDeleteTextures(1, &hdr);

		//Start Of Code For IBL
		glBindTexture(GL_TEXTURE_CUBE_MAP, hdrcubemap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP); //Generate Mip-Maps For Pre-Filter Cubemap

		//Irradiance Convolution
		shader.reset();
		shader = std::make_unique<Shader>("Choice/assets/shaders/IrradianceConvolution.glsl");

		//Create Convolution Cubemap Texture
		uint32_t irradianceConvolution;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &irradianceConvolution);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceConvolution);
		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalformat, 128, 128, 0, dataformat, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTextureUnit(reflectiondata.Samplers["hdrSkybox"], hdrcubemap);

		shader->Use();

		glViewport(0, 0, 128, 128);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		for (uint32_t i = 0; i < 6; i++)
		{
			reflectiondata.UniformBuffers["Camera"]->SetData("Camera.View", &captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceConvolution, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			cube->vertexarray->Bind();
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//Pre-Filter Cubemap
		shader.reset();
		shader = std::make_unique<Shader>("Choice/assets/shaders/Pre-FilterCubemap.glsl");

		uint32_t prefilterCubemap;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &prefilterCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterCubemap);

		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalformat, 128, 128, 0, dataformat, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //Enable TriLinear Filtering 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glBindTextureUnit(reflectiondata.Samplers["hdrSkybox"], hdrcubemap);

		shader->Use();

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		const uint32_t maxMipLevels = 5;
		for (uint32_t mip = 0; mip < maxMipLevels; mip++)
		{
			uint32_t mipWidth = static_cast<uint32_t>(128 * pow(0.5, mip));
			uint32_t mipHeight = static_cast<uint32_t>(128 * pow(0.5, mip));
			glViewport(0, 0, mipWidth, mipHeight);
			float roughness = (float)mip / (float)(maxMipLevels - 1);
			reflectiondata.UniformBuffers["Roughness"]->SetData("Roughness.pfRoughness", &roughness);
			for (uint32_t i = 0; i < 6; i++)
			{
				reflectiondata.UniformBuffers["Camera"]->SetData("Camera.View", &captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterCubemap, mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				cube->vertexarray->Bind();
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//BRDF Lookup Texture
		shader.reset();
		shader = std::make_unique<Shader>("Choice/assets/shaders/BRDFLookup.glsl");

		uint32_t brdfLookup;
		glCreateTextures(GL_TEXTURE_2D, 1, &brdfLookup);
		glBindTexture(GL_TEXTURE_2D, brdfLookup);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLookup, 0);

		glViewport(0, 0, 512, 512);
		shader->Use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDeleteFramebuffers(1, &framebuffer);

		Ids.push_back(hdrcubemap); Ids.push_back(irradianceConvolution);
		Ids.push_back(prefilterCubemap); Ids.push_back(brdfLookup);

		return Ids;
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

	const std::string CompressTexture(void* data, BlockCompressionFormat format, bool generateMips)
	{
		const auto* texture = reinterpret_cast<const cgltf_texture*>(data);
		std::string name = texture->image->name;

		const auto* buffer = texture->image->buffer_view->buffer;
		if (buffer)
		{
			std::string dstFile = global::ActiveSceneDir + "Assets\\" + "\\" + name + ".dds";
			if (ghc::filesystem::exists(dstFile)) { return dstFile; }

			stbi_set_flip_vertically_on_load(0);
			int x, y, channels;
			auto* data = stbi_load_from_memory((stbi_uc*)buffer->data + texture->image->buffer_view->offset,
				static_cast<int>(buffer->size), &x, &y, &channels, 4);
			if (!data)
			{
				Message<WARNING>("[STB] Failed To Load Texture", MESSAGE_ORIGIN::PIPELINE);
				return {};
			}

			CMP_MipSet srcMipSet = {};
			int cmp_status = LoadTextureFromMemory(data, x, y, &srcMipSet);
			if (cmp_status != CMP_OK)
			{
				Message<WARNING>("[COMPRESSONATOR] Failed To Load Texture", MESSAGE_ORIGIN::PIPELINE);
				return {};
			}
			stbi_image_free(data);

			if (generateMips && srcMipSet.m_nMipLevels == 1)
			{
				int minSize = CMP_CalcMinMipSize(srcMipSet.m_nHeight, srcMipSet.m_nWidth, srcMipSet.m_nMaxMipLevels + 1);
				cmp_status = CMP_GenerateMIPLevels(&srcMipSet, minSize);
				if (cmp_status != CMP_OK)
				{
					Message<WARNING>("[COMPRESSONATOR] Failed To Generate Mips", MESSAGE_ORIGIN::PIPELINE);
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
				Message<WARNING>("[COMPRESSONATOR] Failed To Process Texture", MESSAGE_ORIGIN::PIPELINE);
				return {};
			}

			CMP_SaveTexture(dstFile.c_str(), &dstMipSet);

			CMP_FreeMipSet(&srcMipSet);
			CMP_FreeMipSet(&dstMipSet);

			return dstFile;
		}
		return {};
	}


}