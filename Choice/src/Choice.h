#pragma once
#include "cpch.h"

#include "Window.h"
#include "GUI/GUI.h"
#include "GUI/Editor.h"

namespace choice
{
	class Choice
	{
	public:
		Choice();
		~Choice();

		void run();

		std::unique_ptr<Window>& GetWindow() { return mWindow; }
		std::unique_ptr<Editor>& GetEditor() { return mEditor; }

		static Choice* Instance() { return sInstance; }
	private:
		static Choice* sInstance;
	private:
		std::unique_ptr<Window> mWindow;
		std::unique_ptr<GUI> mGUI;
		std::unique_ptr<Editor> mEditor;
	};
}