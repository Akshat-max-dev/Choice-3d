#pragma once
#include "cpch.h"

namespace choice
{
	enum class BlockCompressionFormat
	{
		BC1 = 44, BC2 = 45, BC3 = 46, BC4 = 47, BC5 = 49, BC6 = 51, BC7 = 53
	};

	const std::string CompressTexture(const std::string& srcFile,
		BlockCompressionFormat format, bool generateMips);
	const std::string CompressTexture(void* data,
		BlockCompressionFormat format, bool generateMips);

	class Texture
	{
	public:
		const uint32_t& GetId()const { return mRendererId; }
		virtual void Bind(uint32_t slot)const 
		{
			glBindTextureUnit(slot, mRendererId);
		}
	protected:
		void Destroy() { glDeleteTextures(1, &mRendererId); }
		uint32_t mRendererId;
	};

	const uint32_t LoadTexture2D(const std::string& ddsFile);

	class Texture2D :public Texture
	{
	public:
		Texture2D(uint32_t id) { mRendererId = id; }
		~Texture2D() { Texture::Destroy(); }
	};

	const std::vector<uint32_t> LoadTextureCubemap(const std::string& hdrmap);

	class TextureCubemap :public Texture
	{
	public:
		TextureCubemap(uint32_t id) { mRendererId = id; }
		~TextureCubemap() { Texture::Destroy(); }
	};
}