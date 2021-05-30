#pragma once
#include "cpch.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ReflectionData.h"

namespace choice
{
	class Shader
	{
	public:
		Shader(const std::string& shader);
		~Shader();

		void Use()const;
	private:
		void Read(const std::string& shader);
		void CreateProgram(const std::string& shader);
		void Reflect();
	private:
		uint32_t mProgram = 0;

		std::map<GLenum, std::vector<uint32_t>> mOpenGLSPV;
		std::map<GLenum, std::string> mShaderSource;
	};
}