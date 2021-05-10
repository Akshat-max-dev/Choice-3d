#pragma once
#include "cpch.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace choice
{
	class Binary
	{
	public:
		template<typename T>
		static void Write(std::ofstream& to, T& data) { static_assert(false); }

		template<>
		static void Write<int>(std::ofstream& to, int& data)
		{
			to.write((char*)&data, sizeof(int));
		}

		template<>
		static void Write<uint32_t>(std::ofstream& to, uint32_t& data)
		{
			to.write((char*)&data, sizeof(uint32_t));
		}

		template<>
		static void Write<float>(std::ofstream& to, float& data)
		{
			to.write((char*)&data, sizeof(float));
		}

		template<>
		static void Write<std::string>(std::ofstream& to, std::string& data)
		{
			uint32_t size = (uint32_t)data.size();
			Write<uint32_t>(to, size);
			if (size) { to.write((char*)data.data(), size); }
		}

		template<>
		static void Write<std::vector<float>>(std::ofstream& to, std::vector<float>& data)
		{
			uint32_t size = (uint32_t)data.size();
			Write<uint32_t>(to, size);
			to.write((char*)data.data(), size * sizeof(float));
		}

		template<>
		static void Write<std::vector<uint32_t>>(std::ofstream& to, std::vector<uint32_t>& data)
		{
			uint32_t size = (uint32_t)data.size();
			Write<uint32_t>(to, size);
			to.write((char*)data.data(), size * sizeof(uint32_t));
		}

		template<>
		static void Write<glm::mat4>(std::ofstream& to, glm::mat4& data)
		{
			to.write((char*)glm::value_ptr(data), sizeof(glm::mat4));
		}

		template<>
		static void Write<glm::vec3>(std::ofstream& to, glm::vec3& data)
		{
			to.write((char*)&data.x, sizeof(float));
			to.write((char*)&data.y, sizeof(float));
			to.write((char*)&data.z, sizeof(float));
		}

		template<>
		static void Write<glm::vec4>(std::ofstream& to, glm::vec4& data)
		{
			to.write((char*)&data.x, sizeof(float));
			to.write((char*)&data.y, sizeof(float));
			to.write((char*)&data.z, sizeof(float));
			to.write((char*)&data.w, sizeof(float));
		}

		template<typename T>
		static void Read(std::ifstream& from, T& data) { static_assert(false); }

		template<>
		static void Read<int>(std::ifstream& from, int& data)
		{
			from.read((char*)&data, sizeof(int));
		}

		template<>
		static void Read<uint32_t>(std::ifstream& from, uint32_t& data)
		{
			from.read((char*)&data, sizeof(uint32_t));
		}

		template<>
		static void Read<float>(std::ifstream& from, float& data)
		{
			from.read((char*)&data, sizeof(float));
		}

		template<>
		static void Read<std::string>(std::ifstream& from, std::string& data)
		{
			uint32_t size;
			Read<uint32_t>(from, size);
			if (size)
			{
				data.resize(size);
				from.read((char*)data.data(), size);
			}
		}

		template<>
		static void Read<std::vector<float>>(std::ifstream& from, std::vector<float>& data)
		{
			uint32_t size;
			Read<uint32_t>(from, size);
			data.resize(size);
			from.read((char*)data.data(), size * sizeof(float));
		}

		template<>
		static void Read<std::vector<uint32_t>>(std::ifstream& from, std::vector<uint32_t>& data)
		{
			uint32_t size;
			Read<uint32_t>(from, size);
			data.resize(size);
			from.read((char*)data.data(), size * sizeof(uint32_t));
		}

		template<>
		static void Read<glm::mat4>(std::ifstream& from, glm::mat4& data)
		{
			from.read((char*)glm::value_ptr(data), sizeof(glm::mat4));
		}

		template<>
		static void Read<glm::vec3>(std::ifstream& from, glm::vec3& data)
		{
			from.read((char*)&data.x, sizeof(float));
			from.read((char*)&data.y, sizeof(float));
			from.read((char*)&data.z, sizeof(float));
		}

		template<>
		static void Read<glm::vec4>(std::ifstream& from, glm::vec4& data)
		{
			from.read((char*)&data.x, sizeof(float));
			from.read((char*)&data.y, sizeof(float));
			from.read((char*)&data.z, sizeof(float));
			from.read((char*)&data.w, sizeof(float));
		}
	};
}