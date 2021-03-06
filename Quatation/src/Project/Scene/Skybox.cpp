#include "Skybox.h"

#include <glad/glad.h>

#include "ReflectionData.h"

namespace choice
{
	Skybox::Skybox(const std::string& skybox) :mFilepath(skybox)
	{
		mCube = Cube();
		//Delete Unwanted Memory In Cube
		delete mCube->NodeTransform; mCube->NodeTransform = nullptr;
		delete mCube->materials[0]; mCube->materials[0] = nullptr;

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

	void Skybox::BindIBL(glm::uvec3 slots)
	{
		mIrradianceConvolution->Bind(slots.x);
		mPreFilterCubemap->Bind(slots.y);
		mBRDFLookup->Bind(slots.z);
	}

	void Skybox::Draw(Camera* camera)
	{
		mCubemap->Bind(global::GlobalReflectionData.Samplers["hdrSkybox"]);
		mShader->Use();

		glm::mat4* view = global::GlobalReflectionData.UniformBuffers["Camera"]->MemberData<glm::mat4>("Camera.View", global::CameraBuffer);
		*view = glm::mat4(glm::mat3(camera->View()));

		global::GlobalReflectionData.UniformBuffers["Camera"]->SetData(global::CameraBuffer);

		//Draw Cube
		mCube->vertexarray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

}