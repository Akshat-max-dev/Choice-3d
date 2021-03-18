#include "VertexArray.h"

#include <glad/glad.h>

namespace choice
{
	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &mVertexArray);
		glDeleteBuffers(1, &mVertexBuffer);
		if (mIndexBuffer) { glDeleteBuffers(1, &mIndexBuffer); }
	}

	void VertexArray::Bind() const
	{
		if (mVertexArray) { glBindVertexArray(mVertexArray); }
	}

	void VertexArray::VertexBuffer(void* data, size_t size, std::string layout)
	{
		choiceassert(data);

		glGenVertexArrays(1, &mVertexArray);
		glBindVertexArray(mVertexArray);

		glCreateBuffers(1, &mVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

		std::vector<int> counts;
		uint32_t strides = 0;
		for (auto& c : layout)
		{
			if (int(c) >= 49 && int(c) <= 57)
			{
				counts.push_back(int(c) - 48);
				strides += (int(c) - 48) * sizeof(float);
			}
		}
		uint32_t offset = 0;
		for (uint32_t i = 0; i < counts.size(); i++)
		{
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, counts[i], GL_FLOAT, GL_FALSE, strides,
				(const void*)(uintptr_t)offset);
			offset += counts[i] * sizeof(float);
		}
	}

	void VertexArray::IndexBuffer(void* data, uint32_t count)
	{
		glCreateBuffers(1, &mIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, GL_STATIC_DRAW);
	}

}