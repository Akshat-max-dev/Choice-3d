#include "UniformBuffer.h"

#include <glad/glad.h>

namespace choice
{
	static std::unordered_map<uint32_t, std::string> BindingCache;

	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding,
		const std::map<std::string, UniformBufferMember*> layout, const std::string& bufferName)
		:mBufferLayout(layout), mBinding(binding)
	{
		glCreateBuffers(1, &mRendererId);
		glNamedBufferData(mRendererId, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, mBinding, mRendererId);

		if (BindingCache.find(mBinding) != BindingCache.end())
		{
			BindingCache[mBinding] = bufferName;
		}
		else
		{
			BindingCache.insert({ mBinding, bufferName });
		}
	}

	UniformBuffer::~UniformBuffer()
	{
		glDeleteBuffers(1, &mRendererId);
		for (auto& bufferlayout : mBufferLayout)
		{
			if (bufferlayout.second) { delete bufferlayout.second; }
		}
	}

	void UniformBuffer::SetDataArray(const char* membername, const void* data, uint32_t index)
	{
		if (mBufferLayout.find(membername) != mBufferLayout.end())
		{
			const auto* member = mBufferLayout[membername];
			if (!strstr(membername, BindingCache[mBinding].c_str()))
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, mBinding, mRendererId);
				std::string temp = membername;
				BindingCache[mBinding] = temp.substr(0, temp.find_last_of('.'));
			}
			uint32_t offset = (member->size * index) + member->offset;
			glNamedBufferSubData(mRendererId, offset, member->size, data);
		}
	}

	void UniformBuffer::SetData(const char* membername, const void* data)
	{
		if (mBufferLayout.find(membername) != mBufferLayout.end())
		{
			const auto* member = mBufferLayout[membername];
			if (!strstr(membername, BindingCache[mBinding].c_str()))
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, mBinding, mRendererId);
				std::string temp = membername;
				BindingCache[mBinding] = temp.substr(0, temp.find_last_of('.'));
			}
			glNamedBufferSubData(mRendererId, member->offset, member->size, data);
		}
	}
}