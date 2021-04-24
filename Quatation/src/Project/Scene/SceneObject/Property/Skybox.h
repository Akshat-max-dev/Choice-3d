#pragma once
#include "cpch.h"

#include "Drawable.h"
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
		const std::string& GetFilepath()const { return mFilepath; }
	private:
		Drawable* mCube;
		TextureCubemap* mCubemap;
		Shader* mShader;
		std::string mFilepath;
	};
}