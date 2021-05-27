#pragma once
#include "cpch.h"

#include "Node.h"
#include "Mesh.h"

namespace choice
{
	bool ImportGLTF(const std::string& srcFile, Node* root, const std::string& dstDirectory);
}