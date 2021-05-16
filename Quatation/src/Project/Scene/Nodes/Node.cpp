#include "Node.h"

namespace choice
{
	Node::Node()
	{
		NodeTransform = new Transform();
	}

	Node::~Node()
	{
		for (auto& child : Children) { if (child) { delete child; } }
		if (NodeTransform) { delete NodeTransform; }
	}
}