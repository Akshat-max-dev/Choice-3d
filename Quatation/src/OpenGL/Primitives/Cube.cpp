#include "Cube.h"

#include <glad/glad.h>

namespace choice
{

	Cube::Cube()
	{
		mName = "Cube";
		Setup();
	}

	void Cube::Draw()
	{
		mMesh->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	void Cube::Setup()
	{
		float vertices[] = {
			-1.0f, -1.0f, -1.0f, 
			 1.0f,  1.0f, -1.0f,  
			 1.0f, -1.0f, -1.0f,   
			 1.0f,  1.0f, -1.0f,  
			-1.0f, -1.0f, -1.0f,   
			-1.0f,  1.0f, -1.0f,  

			-1.0f, -1.0f,  1.0f,   
			 1.0f, -1.0f,  1.0f,  
			 1.0f,  1.0f,  1.0f,   
			 1.0f,  1.0f,  1.0f,  
			-1.0f,  1.0f,  1.0f,  
			-1.0f, -1.0f,  1.0f,  

			-1.0f,  1.0f,  1.0f, 
			-1.0f,  1.0f, -1.0f, 
			-1.0f, -1.0f, -1.0f, 
			-1.0f, -1.0f, -1.0f, 
			-1.0f, -1.0f,  1.0f,  
			-1.0f,  1.0f,  1.0f, 

			 1.0f,  1.0f,  1.0f,  
			 1.0f, -1.0f, -1.0f,  
			 1.0f,  1.0f, -1.0f,  
			 1.0f, -1.0f, -1.0f, 
			 1.0f,  1.0f,  1.0f,  
			 1.0f, -1.0f,  1.0f, 

			-1.0f, -1.0f, -1.0f, 
			 1.0f, -1.0f, -1.0f,  
			 1.0f, -1.0f,  1.0f, 
			 1.0f, -1.0f,  1.0f,   
			-1.0f, -1.0f,  1.0f,  
			-1.0f, -1.0f, -1.0f, 

			-1.0f,  1.0f, -1.0f, 
			 1.0f,  1.0f , 1.0f,  
			 1.0f,  1.0f, -1.0f,   
			 1.0f,  1.0f,  1.0f, 
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f
		};

		mMesh = new VertexArray();
		mMesh->VertexBuffer(vertices, sizeof(vertices), "3");
		mMesh->IndexBuffer(nullptr, 0);
	}
}