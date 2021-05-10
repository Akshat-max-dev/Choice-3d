#include "Skeleton.h"

namespace choice
{
	Skeleton::Skeleton()
	{
		mBones = {};
		mJointsSphere = {};
	}

	Skeleton::~Skeleton()
	{
		for (auto& jointdata : mJointsData) { if (jointdata) { delete jointdata; } }
		Free();
	}

	void Skeleton::Draw(Transform* transform, Shader* shader, bool drawAxis)
	{
		if (!mJointsSphere && !mBones) { Setup(drawAxis); }

		shader->Use();
		shader->Float4("oColor", { 0.1f, 0.2f, 0.6f, 1.0f }); //Joint Sphere Color

		//Draw Joint Sphere
		mJointsSphere->GetMeshes()[0].first->Bind();
		uint32_t c = mJointsSphere->GetMeshes()[0].first->GetCount();
		for (auto& jointdata : mJointsData)
		{
			glm::vec3 p = glm::vec3(mArmatureTransform * jointdata->InvBindMatrix[3]);
			shader->Mat4("uTransform", glm::scale(glm::translate(transform->GetTransform(), { p.x, p.y, -p.z }), { 0.01f, 0.01f, 0.01f }));
			glDrawElements(GL_TRIANGLE_STRIP, c, GL_UNSIGNED_INT, nullptr);
		}

		//Draw Bones 
		shader->Mat4("uTransform", transform->GetTransform());
		shader->Float4("oColor", { 1.0f, 0.0f, 1.0f, 1.0f });
		mBones->Bind();
		uint32_t count = mBones->GetCount();
		glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, nullptr);

		//TODO : Draw Joint Axis 
	}

	void Skeleton::Free()
	{
		if (mJointsSphere) { delete mJointsSphere; mJointsSphere = {}; }
		if (mBones) { delete mBones; mBones = {}; }
		for (auto& jointaxis : mJointsAxis) { if (jointaxis) { delete jointaxis; jointaxis = {}; } }
	}

	void Skeleton::Setup(bool setupAxis)
	{
		Free();

		//Joint Sphere
		mJointsSphere = LoadDrawable("Joint Sphere", DrawableType::SPHERE, false);

		//Joint Bones
		mBones = new VertexArray();

		std::vector<float> vertices;
		std::vector<uint32_t> indices;

		for (auto j = 0; j < mJointsData.size(); j++)
		{
			glm::vec3 t = glm::vec3(mArmatureTransform * mJointsData[j]->InvBindMatrix[3]);
			vertices.push_back(t.x);
			vertices.push_back(t.y);
			vertices.push_back(-t.z);

			if (mJointsData[j]->Parent)
			{
				for (auto i = 0; i < mJointsData.size(); i++)
				{
					if (mJointsData[i]->Name == mJointsData[j]->Parent->Name) { indices.push_back(i); break; }
				}
				indices.push_back(j);
			}
		}

		mBones->VertexBuffer(vertices.data(), vertices.size() * sizeof(float), "3");
		mBones->IndexBuffer(indices.data(), (uint32_t)indices.size());

		if (setupAxis) //Joint Axis
		{
			//TODO
		}
	}
}