#include "Shader.h"

#include <glad/glad.h>

namespace choice
{
	GLenum getTypeFromString(const std::string& t)
	{
		if (t == "vertex") { return GL_VERTEX_SHADER; }
		else if (t == "fragment") { return GL_FRAGMENT_SHADER; }
		else choiceassert(0); return GL_NONE;
	}

	Shader::Shader(const std::string& shader)
	{
		Load(shader);
	}

	Shader::~Shader()
	{
		glDeleteProgram(mProgram);
	}

	void Shader::Use() const
	{
		glUseProgram(mProgram);
	}

	void Shader::Int(const char* name, const int data)
	{
		GLint location = glGetUniformLocation(mProgram, name);
		glUniform1i(location, data);
	}

	void Shader::UInt(const char* name, const uint32_t data)
	{
		GLint location = glGetUniformLocation(mProgram, name);
		glUniform1ui(location, data);
	}

	void Shader::Mat4(const char* name, const glm::mat4& data)
	{
		GLint location = glGetUniformLocation(mProgram, name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data));
	}

	void Shader::Load(const std::string& shaderfile)
	{
		std::ifstream file(shaderfile, std::ios::in);

		if (file.fail() || file.bad())
		{
			std::cout << "Cannot Open " + shaderfile.substr(0, shaderfile.find_last_of('/') + 1) << std::endl;
			return;
		}

		std::map<GLenum, std::string> Data;
		std::string line;

		std::string tolocate = "#source";
		auto sizeoftolocate = tolocate.size();

		GLenum readingcurrent = GL_NONE;

		while (getline(file, line))
		{
			if (line.find(tolocate) != std::string::npos)
			{
				readingcurrent = getTypeFromString(line.substr(sizeoftolocate + 1, line.size() - 1));
				Data.insert({ readingcurrent, "" });
			}
			else
			{
				auto it = Data.find(readingcurrent);
				line += "\n";
				it->second += line;
			}
		}

		file.close();

		uint32_t program = glCreateProgram();
		std::vector<uint32_t> shaders(Data.size());
		int shadercount = 0;
		for (auto& shader : Data)
		{
			uint32_t Id = glCreateShader(shader.first);
			const char* src = shader.second.c_str();
			glShaderSource(Id, 1, &src, 0);
			glCompileShader(Id);

			int isCompiled;
			glGetShaderiv(Id, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				int maxLength = 0;
				glGetShaderiv(Id, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<char> infoLog(maxLength);
				glGetShaderInfoLog(Id, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(Id);

				std::cout << "Failed To Compile Shader " + ghc::filesystem::path(shaderfile).stem().string() << std::endl;
				std::cout << infoLog.data() << std::endl;
				choiceassert(0);
			}

			glAttachShader(program, Id);

			shaders[shadercount++] = Id;
		}

		glLinkProgram(program);

		int isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			int maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<char> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);

			for (auto& shader : shaders)
			{
				glDetachShader(program, shader);
				glDeleteShader(shader);
			}

			std::cout << "Program Linking Failed" << std::endl;
			return;
		}

		for (auto& shader : shaders)
		{
			glDetachShader(program, shader);
			glDeleteShader(shader);
		}

		mProgram = program;
	}

}