#pragma once
#include "cpch.h"

namespace choice
{
	class VertexArray
	{
	public:
		~VertexArray();

		void Bind()const;
		const uint32_t& GetCount()const { return mCount; }
		void VertexBuffer(void* data, size_t size, std::string layout);
		void IndexBuffer(void* data, uint32_t count);
	private:
		uint32_t mVertexArray;
		uint32_t mVertexBuffer, mIndexBuffer;
		uint32_t mCount;
	};
}