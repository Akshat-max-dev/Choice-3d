#pragma once
#include "cpch.h"

namespace choice
{
	class GUI
	{
	public:
		GUI();
		~GUI();

		void Begin();
		void End();
	private:
		void CustomDarkTheme();
	};
}