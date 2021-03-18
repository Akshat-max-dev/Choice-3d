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

		void Mat4(const char* name, const glm::mat4& data);
	private:
		void Load(const std::string& shader);
	private:
		uint32_t mProgram;
	};
}