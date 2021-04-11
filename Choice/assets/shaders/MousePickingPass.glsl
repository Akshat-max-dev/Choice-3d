#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;

uniform mat4 uViewProjection;
uniform mat4 uTransform;

void main()
{
	gl_Position = uViewProjection * uTransform * vec4(aPosition, 1.0);
}

#source fragment
#version 450 core

layout(location = 0)out vec3 pPixelInfo;

uniform uint pObjectIndex;
uniform uint pDrawIndex;

void main()
{
	pPixelInfo = vec3(float(pObjectIndex), float(pDrawIndex), float(gl_PrimitiveID + 1));
}