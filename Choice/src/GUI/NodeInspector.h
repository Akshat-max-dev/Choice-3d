#pragma once
#include "cpch.h"

#include "project/Scene/Nodes/Node.h"

namespace choice
{
	class NodeInspector
	{
	public:
		NodeInspector();
		~NodeInspector();

		void Execute(Node* node);
	};
}