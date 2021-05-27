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
		
		void Save(const std::string& dstFile);
		void Load(const std::string& srcFile);
	private:
		tinyxml2::XMLDocument* mFile;
	};

	tinyxml2::XMLElement* WriteBinaryData(tinyxml2::XMLDocument* file, const char* name, std::vector<char>& data);
	void ReadBinaryData(const tinyxml2::XMLElement* element, std::vector<char>& data);
}