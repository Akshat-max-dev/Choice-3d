#pragma once
#include "cpch.h"

namespace choice
{
	enum class BlockCompressionFormat
	{
		BC1 = 44, BC2 = 45, BC3 = 46, BC4 = 47, BC5 = 49, BC6 = 51, BC7 = 53
	};

	const std::string CompressTexture(const std::string& srcFile, const std::string& dstDirectory,
		BlockCompressionFormat format, bool generateMips);
	const std::string CompressTexture(void* data, const std::string& dstDirectory,
		BlockCompressionFormat format, bool generateMips);
}