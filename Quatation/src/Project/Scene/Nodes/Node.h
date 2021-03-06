#pragma once
#include "cpch.h"

#include "Transform.h"

namespace choice
{
	enum class NODE_DATA_TYPE
	{
		NONE = 0, MESH = 1, LIGHT = 2, JOINT = 3
	};

	namespace global
	{
		inline uint32_t NodeCounter = 0;
	}

	struct Node
	{
		std::string Name;
		NODE_DATA_TYPE node_data_type;
		Node* Parent;
		std::vector<Node*> Children;
		Transform* NodeTransform;
		glm::mat4 WorldTransform;
		uint32_t Id;

		Node();
		virtual ~Node();
	};

	void IterateNodes(Node* node, const std::function<void(Node*)>& func);
	void UpdateWorldTransform(Node* node);
}