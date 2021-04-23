#pragma once
#include"cpch.h"

#include "OpenGL/VertexArray.h"

namespace choice
{
	enum class PrimitiveType
	{
		NONE = -1, CUBE = 0, SPHERE = 1
	};

	class Primitive
	{
	public:
		~Primitive()
		{
			delete mMesh;
		}

		std::string& GetName() { return mName; }
		const PrimitiveType& GetPrimitiveType()const { return mPrimitiveType; }

		virtual void Draw() {}
	protected:
		virtual void Setup() = 0;
		VertexArray* mMesh;
		std::string mName;
		PrimitiveType mPrimitiveType = PrimitiveType::NONE;
	};
}