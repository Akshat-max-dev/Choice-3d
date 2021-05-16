#pragma once
#include "cpch.h"

#include "Nodes/Mesh.h"
#include "Nodes/Light.h"
#include "Nodes/BoundingBox.h"
#include "Skybox.h"

namespace choice
{
	class Scene
	{
	public:
		Scene(const std::string& name, const std::string& directory);
		Scene(const std::string& cscene);
		~Scene();

		void AddNode(Node* node) { mNodes.push_back(node); }
		void SetSkybox(Skybox* skybox) { mSkybox = skybox; }

		std::vector<Node*>& GetNodes() { return mNodes; }
		BoundingBox& GetBoundingBox() { return mBoundingBox; }
		Skybox* GetSkybox() { return mSkybox; }

		void Save();
		void Clean();

		std::string& Name() { return mName; }
		const std::string& Directory()const { return mDirectory; }
	private:
		std::string mName;
		std::string mDirectory;
		std::vector<Node*> mNodes;
		Skybox* mSkybox;
		BoundingBox mBoundingBox;
	};
}