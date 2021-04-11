#pragma once
#include "cpch.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace choice
{
	class Shader
	{
	public:
		Shader(const std::string& shader);
		~Shader();

		void Use()const;

		void Int(const char* name, const int data);
		void UInt(const char* name, const uint32_t data);
		void Mat4(const char* name, const glm::mat4& data);
	private:
		void Load(const std::string& shader);
	private:
		uint32_t mProgram;
	};
}