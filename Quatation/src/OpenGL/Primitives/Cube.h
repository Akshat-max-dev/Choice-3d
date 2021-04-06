#pragma once
#include "cpch.h"

#include "OpenGL/VertexArray.h"

namespace choice
{
	class Cube
	{
	public:
		Cube();
		~Cube();

		VertexArray* Mesh() { return mMesh; }
	private:
		void Setup();
		VertexArray* mMesh;
	};
}