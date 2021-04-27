#pragma once
#include "cpch.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace choice
{
	class Camera
	{
	public:
		Camera(float aspectratio) :mAspectRatio(aspectratio)
		{
			mProjection = glm::perspective(glm::radians(mFov), mAspectRatio, mNearClip, mFarClip);
		}

		const glm::mat4 ViewProjection()const { return mProjection * mView; }
		const glm::mat4& View()const { return mView; }
		const glm::mat4& Projection()const { return mProjection; }

		const glm::vec3 Position()const { return mFocus + mOffset; }

		const glm::vec3& Offset()const { return mOffset; }
		const glm::vec3& Focus()const { return mFocus; }
		const glm::vec3& Up()const { return mUp; }
		const glm::vec3& Right()const { return mRight; }

		void Visible(uint32_t w, uint32_t h)
		{
			mAspectRatio = (float)w / (float)h;
			mProjection = glm::perspective(glm::radians(mFov), mAspectRatio, mNearClip, mFarClip);
		}

		void AcceptInput(bool acceptinput) { mAcceptInput = acceptinput; }

		virtual void Update() {}

		virtual void OnButtonDown(int button) {}
		virtual void OnButtonUp(int button) {}
		virtual void OnMove(double xpos, double ypos) {}
		virtual void OnScroll(double yoffset) {}
	protected:
		glm::mat4 mView = glm::mat4(1.0f);
		glm::mat4 mProjection;
		glm::vec3 mFocus, mOffset, mRight;
		glm::vec3 mUp;
		float mFov = 45.0f;
		float mAspectRatio;
		float mNearClip = 0.1f;
		float mFarClip = 1000.0f;
		float mLastX, mLastY, mDeltaX, mDeltaY;

		enum class MovementType
		{
			NONE = -1, ROTATION = 0, TRANSLATION = 1
		};

		MovementType mMovementType = MovementType::NONE;

		bool mAcceptInput = true;
	};
}