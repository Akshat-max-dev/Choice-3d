#pragma once
#include "cpch.h"

#include "Primitive.h"

namespace choice
{
	class Sphere :public Primitive
	{
	public:
		Sphere();
		Sphere(const char* name);
		void Draw()override;
	private:
		void Setup()override;
	};
}