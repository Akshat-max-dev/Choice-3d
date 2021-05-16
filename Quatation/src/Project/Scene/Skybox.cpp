#include "Skybox.h"

#include <glad/glad.h>

namespace choice
{
	Skybox::Skybox(const std::string& skybox) :mFilepath(skybox)
	{
		mCube = Cube();
		//Delete Unwanted Memory In Cube
		delete mCube->NodeTransform; mCube->NodeTransform = nullptr;
		delete mCube->primitives[0]->material; mCube->primitives[0]->material = nullptr;

		mShader = new Shader("Choice/assets/shaders/HDRSkybox.glsl");

		std::vector<uint32_t> Ids = LoadTextureCubemap(skybox);

		mCubemap			   = new TextureCubemap(Ids[0]);
		mIrradianceConvolution = new TextureCubemap(Ids[1]);
		mPreFilterCubemap	   = new TextureCubemap(Ids[2]);
		mBRDFLookup			   = new Texture2D(Ids[3]);
	}

	Skybox::~Skybox()
	{
		delete mCubemap;
		delete mCube;
		delete mShader;

		delete mIrradianceConvolution;
		delete mPreFilterCubemap;
		delete mBRDFLookup;
	}

	void Skybox::BindIBL(glm::uvec3 slots) const
	{
		mIrradianceConvolution->Bind(slots.x);
		mPreFilterCubemap->Bind(slots.y);
		mBRDFLookup->Bind(slots.z);
	}

	void Skybox::Draw(Camera* camera)
	{
		mShader->Use();
		mShader->Int("hdrSkybox", 0);
		mShader->Mat4("uProjection", camera->Projection());
		mShader->Mat4("uView", glm::mat4(glm::mat3(camera->View())));
		mCubemap->Bind(0);
		//Draw Cube
		mCube->primitives[0]->vertexarray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

}