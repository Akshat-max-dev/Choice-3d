#pragma once
#include "cpch.h"

#include "OpenGL/Primitives/Cube.h"
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

		void Draw(Camera* camera);
	private:
		Cube* mCube;
		TextureCubemap* mCubemap;
		Shader* mShader;
	};
}