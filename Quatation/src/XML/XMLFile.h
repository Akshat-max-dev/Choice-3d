#pragma once
#include "cpch.h"

#include <tinyxml2.h>
#include <base64.h> //Use For Base64 Encoding/Decoding

#include "Project/Scene/Nodes/Mesh.h"
#include "Project/Scene/Nodes/Light.h"

namespace choice
{
	class XMLFile
	{
	public:
		XMLFile();
		~XMLFile();

		tinyxml2::XMLDocument* GetFile() { return mFile; }

		void WriteNode(Node* node);
		Node* ReadNode(uint32_t id, std::vector<uint32_t>& childrenid);
		
		template<typename T>
		void AttributeElement(const char* name, const T& data);

		template<typename T>
		T AttributeElement(const char* name);



		void Save(const std::string& dstFile);
		void Load(const std::string& srcFile);
	private:
		tinyxml2::XMLDocument* mFile;
	};

	template<typename T>
	void XMLFile::AttributeElement(const char* name, const T& data)
	{
		tinyxml2::XMLElement* attributeelement = mFile->NewElement(name);
		attributeelement->SetAttribute("Value", data);
		mFile->InsertEndChild(attributeelement);
	}

	template<typename T>
	T XMLFile::AttributeElement(const char* name)
	{
		T data;
		tinyxml2::XMLElement* attributeelement = mFile->FirstChildElement(name);
		attributeelement->QueryAttribute("Value", &data);

		return data;
	}

}