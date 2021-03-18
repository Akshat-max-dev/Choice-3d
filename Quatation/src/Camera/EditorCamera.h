#pragma once
#include "cpch.h"

#include "Camera.h"

namespace choice
{
	class EditorCamera :public Camera
	{
	public:
		EditorCamera(float aspectratio);

		void Update()override;

		void OnMove(double xpos, double ypos)override;
		void OnScroll(double yoffset)override;
		void OnButtonDown(int button)override;
		void OnButtonUp(int button)override;
	};
}