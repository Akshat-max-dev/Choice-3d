#pragma once
#include "cpch.h"

namespace choice
{
	enum MESSAGE_TYPE
	{
		ERROR_MSG = 0, WARNING = 1, INFO = 2
	};

	inline std::vector<std::pair<MESSAGE_TYPE, std::string>> Messages;

	class Console
	{
	public:
		Console();
		~Console();

		void Execute();
	};

	#define CONSOLE(msg, ...) { Messages.push_back({__VA_ARGS__, msg}); }
}