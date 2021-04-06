#pragma once
#include "cpch.h"

#include "Window.h"
#include "GUI/GUI.h"
#include "GUI/Editor.h"
#include "OpenGL/Pipeline/DeferredPipeline.h"

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
		std::unique_ptr<GUI>& GetGUI() { return mGUI; }
		std::unique_ptr<Pipeline>& GetPipeline() { return mPipeline; }

		static Choice* Instance() { return sInstance; }
	private:
		static Choice* sInstance;
	private:
		std::unique_ptr<Window> mWindow;
		std::unique_ptr<GUI> mGUI;
		std::unique_ptr<Editor> mEditor;
		std::unique_ptr<Pipeline> mPipeline;
	};
}