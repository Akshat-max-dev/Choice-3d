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
		void Float(const char* name, const float data);
		void Float2(const char* name, const glm::vec2& data);
		void Float3(const char* name, const glm::vec3& data);
		void Float4(const char* name, const glm::vec4& data);
		void Mat4(const char* name, const glm::mat4& data);
	private:
		void Load(const std::string& shader);
	private:
		uint32_t mProgram;
	};
}