#include "Error.h"

namespace choice
{
	static const char* MESSAGE_ORIGIN_NAMES[MESSAGE_ORIGIN::MESSAGE_ORIGIN_COUNT] = {
		"-", "[FILESYSTEM] ", "[PIPELINE] ", "[EDITOR] ", "[SHADER COMPILER] ", "[OPENGL] "
	};

	template<MESSAGE_TYPE T>
	bool Message(const char* msg, MESSAGE_ORIGIN message_origin)
	{
		static_assert(false);
	}

	template<>
	bool Message<ERROR_MSG>(const char* msg, MESSAGE_ORIGIN message_origin)
	{
		std::string message = MESSAGE_ORIGIN_NAMES[message_origin];
		message += msg;
		CONSOLE(message, ERROR_MSG);
		return false;
	}

	template<>
	bool Message<WARNING>(const char* msg, MESSAGE_ORIGIN message_origin)
	{
		std::string message = MESSAGE_ORIGIN_NAMES[message_origin];
		message += msg;
		CONSOLE(message, WARNING);
		return false;
	}

	template<>
	bool Message<INFO>(const char* msg, MESSAGE_ORIGIN message_origin)
	{
		std::string message = MESSAGE_ORIGIN_NAMES[message_origin];
		message += msg;
		CONSOLE(message, INFO);
		return true;
	}
}