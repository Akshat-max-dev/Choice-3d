#pragma once
#include "cpch.h"

namespace choice
{
	struct UniformBufferMember
	{
		uint32_t offset;
		uint32_t size;
	};

	class UniformBuffer
	{
	public:
		UniformBuffer(uint32_t size, uint32_t binding, 
			const std::map<std::string, UniformBufferMember*> layout, const std::string& bufferName);
		~UniformBuffer();

		void SetDataArray(const char* membername, const void* data, uint32_t index);
		void SetData(const char* membername, const void* data);
	private:
		uint32_t mRendererId;
		uint32_t mBinding;
		std::map<std::string, UniformBufferMember*> mBufferLayout;
	};
}