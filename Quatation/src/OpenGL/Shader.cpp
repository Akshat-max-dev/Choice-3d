#include "Shader.h"

#include <glad/glad.h>
#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

#include "Error.h"

namespace choice
{
	GLenum getTypeFromString(const std::string& t)	
	{
		if (t == "vertex") { return GL_VERTEX_SHADER; }
		else if (t == "fragment") { return GL_FRAGMENT_SHADER; }
		else choiceassert(0); return GL_NONE;
	}

	shaderc_shader_kind getshadercFromShaderStage(GLenum stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER: return shaderc_glsl_vertex_shader;
		case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
		}
		choiceassert(0);
		return (shaderc_shader_kind)0;
	}

	const char* CachedOpenGLShaderStageExtension(GLenum stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER:    return ".cached_opengl.vert";
		case GL_FRAGMENT_SHADER:  return ".cached_opengl.frag";
		}
		choiceassert(0);
		return "";
	}

	const char* GetCachedDirectory()
	{
		return "Choice/assets/cache/opengl";
	}

	void CreateCacheDirectory()
	{
		if (!ghc::filesystem::exists("Choice/assets/cache"))
			ghc::filesystem::create_directory("Choice/assets/cache");

		auto dir = GetCachedDirectory();

		if (!ghc::filesystem::exists(dir))
			ghc::filesystem::create_directory(dir);
	} 

	Shader::Shader(const std::string& shader)
	{
		CreateProgram(shader);
		Reflect();
	}

	Shader::~Shader()
	{
		glDeleteProgram(mProgram);
	}

	void Shader::Use() const
	{
		glUseProgram(mProgram);
	}

	std::vector<std::string> GLSLInclude(const std::string& line, const std::string& dir)
	{
		std::string tolocate = "include";
		size_t tolocatesize = tolocate.size();

		std::vector<std::string> result;

		if (line.find(tolocate) != std::string::npos)
		{
			//extract include file name from the line
			std::string includefilename = line.substr(0, line.find(tolocate) - 1);

			//Get Structs Name To Inlclude
			std::string includes = line.substr(line.find(tolocate) + tolocatesize + 1, line.find(';'));
			includes.erase(includes.find(';'));

			std::stringstream ss(includes);

			std::vector<std::string> includesname; //names of structs to include
			while (ss.good())
			{
				std::string include_name;
				getline(ss, include_name, ',');
				includesname.push_back(include_name);
			}

			result.resize(includesname.size());

			//read includes from the given file
			std::ifstream includefile(dir + includefilename, std::ios::in);

			if (includefile.is_open())
			{
				std::string line;
				bool nowReading = false;

				for (uint32_t i = 0; i < includesname.size(); i++)
				{
					while (getline(includefile, line))
					{
						if (line.find(includesname[i]) != std::string::npos) //read the (struct/buffer/function Name) line 
						{
							line += "\n";
							result[i] += line;
							nowReading = true;
						}
						else if (line.find("};") != std::string::npos || line.find("}") != std::string::npos) //break if the struct/function is completely saved
						{
							if (nowReading)
							{
								line += "\n";
								result[i] += line;
								nowReading = false;
								
								includefile.seekg(0);
								break;
							}
						}
						else //get struct data
						{
							if (nowReading)
							{
								line += "\n";
								result[i] += line;
							}
						}
					}
				}

				includefile.close();
			}
		}

		return result;
	}


	void Shader::Read(const std::string& shader)
	{
		std::ifstream file(shader, std::ios::in);

		if (file.fail())
		{
			std::cout << "Cannot Open " + shader.substr(0, shader.find_last_of('/') + 1) << std::endl;
			choiceassert(0);
		}

		std::string line;

		std::string tolocate = "#source";
		auto sizeoftolocate = tolocate.size();

		std::string includeline = "from";
		auto sizeofincludeline = includeline.size();

		GLenum readingcurrent = GL_NONE;

		std::vector<std::string> includes;

		while (getline(file, line))
		{
			if (line.find(tolocate) != std::string::npos)
			{
				readingcurrent = getTypeFromString(line.substr(sizeoftolocate + 1, line.size() - 1));
				mShaderSource.insert({ readingcurrent, "" });
			}
			else if (line.find(includeline) != std::string::npos)
			{
				includes = GLSLInclude(line.substr(sizeofincludeline + 1, line.size()),
					shader.substr(0, shader.find_last_of('/') + 1));

				auto it = mShaderSource.find(readingcurrent);
				for (auto& include : includes)
				{
					it->second += include;
				}
			}
			else
			{
				auto it = mShaderSource.find(readingcurrent);
				line += "\n";
				it->second += line;
			}
		}

		file.close();
	}

	void Shader::CreateProgram(const std::string& shader)
	{
		CreateCacheDirectory(); //Creates a SPIR-V Cache Directory If Not Exists

		std::string cacheDirectory = GetCachedDirectory();
		
		Read(shader);

		for (auto&& [stage, source] : mShaderSource)
		{
			auto& cachedShaderStagePath = cacheDirectory + '/' + ghc::filesystem::path(shader).filename().string() +
				CachedOpenGLShaderStageExtension(stage);

			if (ghc::filesystem::exists(cachedShaderStagePath))
			{
				std::ifstream spv(cachedShaderStagePath, std::ios::in | std::ios::binary);

				spv.seekg(0, std::ios::end);
				auto size = spv.tellg();
				spv.seekg(0, std::ios::beg);

				auto& data = mOpenGLSPV[stage];
				data.resize(size / sizeof(uint32_t));
				spv.read((char*)data.data(), size);

				spv.close();
			}
			else
			{
				shaderc::Compiler compiler;
				shaderc::CompileOptions options;
				options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

				shaderc::SpvCompilationResult spv = compiler.CompileGlslToSpv(source,
					getshadercFromShaderStage(stage), shader.c_str(), options);

				if (spv.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					Message<ERROR_MSG>(spv.GetErrorMessage().c_str(), MESSAGE_ORIGIN::SHADER_COMPILER);
					return;
				}

				mOpenGLSPV[stage] = std::vector<uint32_t>(spv.cbegin(), spv.cend());

				std::ofstream outspv(cachedShaderStagePath, std::ios::out | std::ios::binary);
				if (outspv.is_open())
				{
					auto& data = mOpenGLSPV[stage];
					outspv.write((char*)data.data(), data.size() * sizeof(uint32_t));
					outspv.close();
					Message<INFO>(cachedShaderStagePath.c_str(), MESSAGE_ORIGIN::SHADER_COMPILER);
				}
			}
		}

		uint32_t program = glCreateProgram();
		std::vector<uint32_t> shadersIds;

		for (auto&& [stage, spv] : mOpenGLSPV)
		{
			uint32_t shaderId = glCreateShader(stage);
			glShaderBinary(1, &shaderId, GL_SHADER_BINARY_FORMAT_SPIR_V, spv.data(), static_cast<GLsizei>(spv.size() * sizeof(uint32_t)));
			glSpecializeShader(shaderId, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderId);
			shadersIds.push_back(shaderId);
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

			for (auto& shaderId : shadersIds)
			{
				glDetachShader(program, shaderId);
				glDeleteShader(shaderId);
			}

			glDeleteProgram(program);

			Message<ERROR_MSG>(infoLog.data(), MESSAGE_ORIGIN::SHADER_COMPILER);
			return;
		}

		for (auto& shaderId : shadersIds)
		{
			glDetachShader(program, shaderId);
			glDeleteShader(shaderId);
		}

		mProgram = program;
	}

	void Shader::Reflect()
	{
		ReflectionData& data = global::GlobalReflectionData;

		for (auto&& [stage, spv] : mOpenGLSPV)
		{
			spirv_cross::Compiler compiler(spv);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();

			for (const auto& resource : resources.sampled_images)
			{
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				std::string name = resource.name;
				if (data.Samplers.find(name) == data.Samplers.end())
				{
					data.Samplers.insert({ name, binding });
				}
			}

			for (const auto& resource : resources.separate_samplers)
			{
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				std::string name = resource.name;
				if (data.Samplers.find(name) == data.Samplers.end())
				{
					data.Samplers.insert({ name, binding });
				}
			}

			for (const auto& resource : resources.uniform_buffers)
			{
				const auto& bufferType = compiler.get_type(resource.base_type_id);
				uint32_t bufferSize = static_cast<uint32_t>(compiler.get_declared_struct_size(bufferType));
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				uint32_t membercount = static_cast<uint32_t>(bufferType.member_types.size());
				std::string buffername = compiler.get_name(bufferType.self);

				if (data.UniformBuffers.find(buffername) == data.UniformBuffers.end())
				{
					UniformBufferLayout layout;

					for (uint32_t i = 0; i < membercount; i++)
					{
						const auto& memberType = compiler.get_type(bufferType.member_types[i]);
						std::string membername = buffername + '.' + compiler.get_member_name(bufferType.self, i);

						//If Member Is A Struct Store All Struct Member Data In Buffer Layout
						if (memberType.basetype == spirv_cross::SPIRType::Struct)
						{
							uint32_t structmembercount = static_cast<uint32_t>(memberType.member_types.size());
							for (uint32_t m = 0; m < structmembercount; m++)
							{
								std::string name = membername + "." + compiler.get_member_name(memberType.self, m);
								if (layout.find(name) == layout.end())
								{
									UniformBufferMember* member = new UniformBufferMember();
									member->size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(memberType, m));
									member->offset = compiler.type_struct_member_offset(memberType, m);
									layout.insert({ name, member });
								}
							}
						}

						uint32_t arraysize = 1;
						if (memberType.array.size())
						{
							arraysize = memberType.array[0];
						}
						uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(bufferType, i) / arraysize);
						uint32_t offset = compiler.type_struct_member_offset(bufferType, i);

						std::string temp = membername;

						for (uint32_t i = 0; i < arraysize; i++)
						{
							if (memberType.array.size())
							{
								temp += "[" + std::to_string(i) + "]";
							}

							if (layout.find(temp) == layout.end())
							{
								UniformBufferMember* member = new UniformBufferMember();
								member->size = size;
								member->offset = offset + (size * i);
								layout.insert({ temp, member });
							}

							temp = membername;
						}
					}

					UniformBuffer* buffer = new UniformBuffer(buffername, binding, bufferSize, layout);;
					data.UniformBuffers.insert({ buffername, buffer });
				}
			}
		}

		mOpenGLSPV.clear();
	}
}