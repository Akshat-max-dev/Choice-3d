#include "Node.h"

namespace choice
{
	Node::Node()
	{
		NodeTransform = new Transform();
		WorldTransform = NodeTransform->GetTransform();
		Id = 0;
		Parent = {};
	}

	Node::~Node()
	{
		for (auto& child : Children) { if (child) { delete child; } }
		if (NodeTransform) { delete NodeTransform; }
	}

	void IterateNodes(Node* node, const std::function<void(Node*)>& func)
	{
		if (node)
		{
			func(node);

			//Children
			for (auto& child : node->Children)
			{
				IterateNodes(child, func);
			}
		}
	}

	void UpdateWorldTransform(Node* node)
	{
		if (node->Parent)
		{
			node->WorldTransform = node->Parent->WorldTransform * node->NodeTransform->GetTransform();
		}
		else
		{
			node->WorldTransform = node->NodeTransform->GetTransform();
		}
	}

}