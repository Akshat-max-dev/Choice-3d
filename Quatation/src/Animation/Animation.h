#pragma once
#include "cpch.h"

#include "Skeleton.h"

namespace choice
{
	class Skeleton;

	class Animation
	{
	public:
		Animation();
		~Animation();

		Skeleton* GetSkeleton() { return mSkeleton; }
	private:
		Skeleton* mSkeleton;
	};
}