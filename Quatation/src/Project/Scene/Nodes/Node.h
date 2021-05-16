#pragma once
#include "cpch.h"

#include "Transform.h"

namespace choice
{
	enum class NODE_DATA_TYPE
	{
		NONE = 0, MESH = 1, LIGHT = 2
	};

	struct Node
	{
		std::string Name;
		NODE_DATA_TYPE node_data_type;
		Node* Parent;
		std::vector<Node*> Children;
		Transform* NodeTransform;
		glm::mat4 WorldTransform;
		
		Node();
		~Node();
	};
}