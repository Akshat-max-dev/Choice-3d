#include "UniformBuffer.h"

#include <glad/glad.h>

namespace choice
{
	static std::unordered_map<uint32_t, std::string> BindingCache;

	UniformBuffer::UniformBuffer(const std::string& name, uint32_t binding,
		uint32_t size, UniformBufferLayout layout)
		:mName(name), mBinding(binding), mSize(size), mBufferLayout(layout)
	{
		glCreateBuffers(1, &mRendererId);
		glNamedBufferData(mRendererId, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, mBinding, mRendererId);

		if (BindingCache.find(mBinding) != BindingCache.end())
			BindingCache[mBinding] = mName;
		else
			BindingCache.insert({ mBinding, mName });
	}

	UniformBuffer::~UniformBuffer()
	{
		for (auto&& [name, member] : mBufferLayout)
		{
			if (member) { delete member; }
		}
	}

	void UniformBuffer::SetData(const char* membername, const void* data)
	{
		if (mBufferLayout.find(membername) != mBufferLayout.end())
		{
			BindBufferBase();
			const auto* member = mBufferLayout[membername];
			glNamedBufferSubData(mRendererId, member->offset, member->size, data);
		}
	}

	void UniformBuffer::SetData(const std::vector<char>& data)
	{
		if (data.size() == mSize)
		{
			BindBufferBase();
			glNamedBufferSubData(mRendererId, 0, data.size(), (const void*)data.data());
		}
	}

	void UniformBuffer::BindBufferBase()
	{
		if (mName != BindingCache[mBinding])
			glBindBufferBase(GL_UNIFORM_BUFFER, mBinding, mRendererId);
	}
}