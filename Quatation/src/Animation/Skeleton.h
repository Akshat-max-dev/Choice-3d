#pragma once
#include "cpch.h"

#include "OpenGL/VertexArray.h"
#include "OpenGL/Shader.h"
#include "Project/Scene/SceneObject/Property/Drawable.h"
#include "Project/Scene/SceneObject/Property/Transform.h"

namespace choice
{
	struct Joint
	{
		Joint* Parent;
		std::string Name;
		glm::mat4 InvBindMatrix;
	};

	class Drawable;

	class Skeleton
	{
	public:
		Skeleton();
		~Skeleton();

		void Draw(Transform* transform, Shader* shader, bool drawAxis = false);
		void Free();

		std::vector<Joint*>& GetJointsData() { return mJointsData; }
		void SetArmatureTransform(const glm::mat4 t) { mArmatureTransform = t; }
	private:
		void Setup(bool setupAxis);
	private:
		std::vector<Joint*> mJointsData;
		Drawable* mJointsSphere;
		VertexArray* mBones;
		std::vector<VertexArray*> mJointsAxis;

		glm::mat4 mArmatureTransform;
	};
}