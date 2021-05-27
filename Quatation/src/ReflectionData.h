#pragma once
#include "cpch.h"

#include "OpenGL/UniformBuffer.h"

namespace choice
{
	struct ReflectionData
	{
		std::map<std::string, uint32_t> Samplers;
		std::map<std::string, UniformBuffer*> UniformBuffers;

		~ReflectionData()
		{
			for (auto& buffer : UniformBuffers)
			{
				if (buffer.second) { delete buffer.second; }
			}
			Samplers.clear();
		}
	};

	namespace global
	{
		inline ReflectionData GlobalReflectionData;
	}
}