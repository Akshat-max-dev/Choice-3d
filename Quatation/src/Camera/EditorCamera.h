#pragma once
#include "cpch.h"

#include "Camera.h"

namespace choice
{
	class EditorCamera :public Camera
	{
	public:
		EditorCamera(float aspectratio);
		EditorCamera(float aspectratio, const glm::vec3& focus, 
			const glm::vec3& offset, const glm::vec3& up, const glm::vec3& right);

		void Update()override;

		void OnMove(double xpos, double ypos)override;
		void OnScroll(double yoffset)override;
		void OnButtonDown(int button)override;
		void OnButtonUp(int button)override;
	};
}