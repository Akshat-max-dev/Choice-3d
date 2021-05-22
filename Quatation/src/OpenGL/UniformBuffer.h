#pragma once
#include "cpch.h"

namespace choice
{
	struct UniformBufferMember
	{
		uint32_t offset;
		uint32_t size;
	};

	typedef std::map<std::string, UniformBufferMember*> UniformBufferLayout;

	class UniformBuffer
	{
	public:
		UniformBuffer(const std::string& name, uint32_t binding, uint32_t size, UniformBufferLayout layout);
		~UniformBuffer();

		template<typename T>
		T* MemberData(const char* membername, std::vector<char>& data);

		const uint32_t& GetBufferSize()const { return mSize; }
		const UniformBufferLayout& GetBufferLayout()const { return mBufferLayout; }

		void SetData(const char* membername, const void* data);
		void SetData(const std::vector<char>& data);
	private:
		//Bind the buffer to mBinding if the buffer is not bound
		void BindBufferBase();
	private:
		std::string mName;
		uint32_t mBinding;
		uint32_t mSize;
		uint32_t mRendererId;
		UniformBufferLayout mBufferLayout;
	};

	template<typename T>
	T* UniformBuffer::MemberData(const char* membername, std::vector<char>& data)
	{
		if (mBufferLayout.find(membername) != mBufferLayout.end())
		{
			const auto* member = mBufferLayout[membername];
			return reinterpret_cast<T*>(data.data() + member->offset);
		}
		return nullptr;
	}
}