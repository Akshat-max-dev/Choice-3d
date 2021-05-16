#pragma once
#include "cpch.h"

#include "project/Scene/Scene.h"

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