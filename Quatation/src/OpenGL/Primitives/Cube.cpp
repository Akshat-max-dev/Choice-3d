#include "Cube.h"

namespace choice
{
	Cube::Cube()
	{
		mMesh = {};
		Setup();
	}

	Cube::~Cube()
	{
		delete mMesh;
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