#pragma once
#include "cpch.h"

#include "Choice.h"

namespace choice
{
	enum MESSAGE_ORIGIN
	{
		NONE = 0, FILESYSTEM = 1, PIPELINE = 2, EDITOR = 3, SHADER_COMPILER = 4, OPENGL = 5, MESSAGE_ORIGIN_COUNT = 6
	};

	template<MESSAGE_TYPE T>
	bool Message(const char* msg, MESSAGE_ORIGIN message_origin);
}