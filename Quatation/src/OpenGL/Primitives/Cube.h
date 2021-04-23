#pragma once
#include "cpch.h"

#include "Primitive.h"

namespace choice
{
	class Cube :public Primitive
	{
	public:
		Cube();
		Cube(const char* name);
		void Draw()override;
	private:
		void Setup()override;
	};
}