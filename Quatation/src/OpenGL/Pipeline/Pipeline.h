#pragma once
#include "cpch.h"

namespace choice
{
	enum class PipelineTypes
	{
		DEFERRED = 0
	};

	struct Pipeline
	{
		virtual void Init(uint32_t w, uint32_t h) {}
		virtual void Visible(uint32_t w, uint32_t h) {}
		virtual void Shutdown() {}
	};
}