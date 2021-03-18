#include "EditorCamera.h"

#include "Input.h"

namespace choice
{
	EditorCamera::EditorCamera(float aspectratio)
		:Camera(aspectratio)
	{
		mFocus = { 0.0f, 0.0f, 0.0f };
		mOffset = { 0.0f, 0.0f, 5.0f };
		mView = glm::lookAt(mFocus + mOffset, mFocus, mUp);
		mRight = glm::cross(mUp, glm::normalize(mOffset));
		mUp = glm::cross(glm::normalize(mOffset), mRight);
	}

	void EditorCamera::Update()
	{
		switch (mMovementType)
		{
		case Camera::MovementType::ROTATION:
			glm::mat3 yaw = glm::rotate(glm::mat4(1.0f), glm::radians(mDeltaX * 0.1f), glm::vec3(0, 1, 0));
			mOffset = yaw * mOffset;
			mUp = yaw * mUp;

			glm::vec3 forward = glm::normalize(-mOffset);
			mRight = glm::normalize(glm::cross(forward, mUp));

			glm::mat3 pitch = glm::rotate(glm::mat4(1.0f), glm::radians(mDeltaY * 0.1f), mRight);
			mOffset = pitch * mOffset;
			mUp = pitch * mUp;

			break;
		case Camera::MovementType::TRANSLATION:
			float sensitivity = glm::length(mOffset) * 0.001f;
			mFocus += mRight * sensitivity * mDeltaX;
			mFocus += mUp * sensitivity * mDeltaY;
			break;
		}
		mDeltaX = 0.0f;
		mDeltaY = 0.0f;
		mView = glm::lookAt(mFocus + mOffset, mFocus, mUp);
	}

	void EditorCamera::OnMove(double xpos, double ypos)
	{
		mDeltaX = (float)xpos - mLastX;
		mDeltaY = mLastY - (float)ypos;
		mLastX = (float)xpos;
		mLastY = (float)ypos;
	}

	void EditorCamera::OnScroll(double yoffset)
	{
		mOffset -= (float)yoffset * mOffset * 0.05f;
	}

	void EditorCamera::OnButtonDown(int button)
	{
		if (button == GLFW_MOUSE_BUTTON_1)
		{
			mLastX = Input::GetMouseX();
			mLastY = Input::GetMouseY();
			mMovementType = Input::IsKeyPressed(GLFW_KEY_LEFT_ALT) ? Camera::MovementType::ROTATION :
				Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT) ? Camera::MovementType::TRANSLATION : 
				Camera::MovementType::NONE;
		}
	}

	void EditorCamera::OnButtonUp(int button)
	{
		if (button == GLFW_MOUSE_BUTTON_1) { mMovementType = Camera::MovementType::NONE; }
	}
}