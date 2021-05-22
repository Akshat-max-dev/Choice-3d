#pragma once
#include "cpch.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "UniformBuffer.h"

namespace choice
{
	struct ReflectionData
	{
		std::map<std::string, uint32_t> Samplers;
		std::map<std::string, UniformBuffer*> UniformBuffers;
		~ReflectionData();
	};

	class Shader
	{
	public:
		Shader(const std::string& shader, ReflectionData& data);
		Shader(const std::string& shader);
		~Shader();

		void Use()const;
	private:
		void Read(const std::string& shader);
		void CreateProgram(const std::string& shader);
		void Reflect(ReflectionData& data);
	private:
		uint32_t mProgram;

		std::map<GLenum, std::vector<uint32_t>> mOpenGLSPV;
		std::map<GLenum, std::string> mShaderSource;
	};
}