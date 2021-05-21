#pragma once
#include "cpch.h"

#include "Nodes/Mesh.h"
#include "OpenGL/Texture.h"
#include "OpenGL/Shader.h"
#include "Camera/EditorCamera.h"

namespace choice
{
	class Skybox
	{
	public:
		Skybox(const std::string& skybox);
		~Skybox();

		void BindIBL(glm::uvec3 slots);

		void Draw(Camera* camera, UniformBuffer* camerabuffer);
		const std::string& GetFilepath()const { return mFilepath; }
	private:
		Mesh* mCube;
		TextureCubemap* mCubemap;
		Shader* mShader;
		std::string mFilepath;

		//IBL
		TextureCubemap* mIrradianceConvolution;
		TextureCubemap* mPreFilterCubemap;
		Texture2D* mBRDFLookup;
		bool mBindIBL = true;
	};
}