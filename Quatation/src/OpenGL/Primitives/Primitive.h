#pragma once
#include"cpch.h"

#include "OpenGL/VertexArray.h"

namespace choice
{
	class Primitive
	{
	public:
		~Primitive()
		{
			delete mMesh;
		}

		std::string& GetName() { return mName; }

		virtual void Draw() {}
	protected:
		virtual void Setup() = 0;
		VertexArray* mMesh;
		std::string mName;
	};
}