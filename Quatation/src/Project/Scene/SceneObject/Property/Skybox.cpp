#include "Skybox.h"

namespace choice
{
	Skybox::Skybox(const std::string& skybox)
	{
		mCube = new Cube();
		mShader = new Shader("Choice/assets/shaders/HDRSkybox.glsl");
		mCubemap = new TextureCubemap(LoadTextureCubemap(skybox));
	}

	Skybox::~Skybox()
	{
		delete mCubemap;
		delete mCube;
		delete mShader;
	}

	void Skybox::Draw(Camera* camera)
	{
		mShader->Use();
		mShader->Int("hdrSkybox", 0);
		mShader->Mat4("uProjection", camera->Projection());
		mShader->Mat4("uView", glm::mat4(glm::mat3(camera->View())));
		mCubemap->Bind(0);
		mCube->Mesh()->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

}