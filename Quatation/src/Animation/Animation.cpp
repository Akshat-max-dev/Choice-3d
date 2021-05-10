#include "Animation.h"

namespace choice
{
	Animation::Animation()
	{
		mSkeleton = new Skeleton();
	}

	Animation::~Animation()
	{
		if (mSkeleton) { delete mSkeleton; }
	}
}