#pragma once
#include "cpch.h"

#include "Primitive.h"

namespace choice
{
	class Cube :public Primitive
	{
	public:
		Cube();
		void Draw()override;
	private:
		void Setup()override;
	};
}