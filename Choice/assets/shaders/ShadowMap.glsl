#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;

uniform mat4 uLightViewProjection;
uniform mat4 uTransform;

void main()
{
	gl_Position = uLightViewProjection * uTransform * vec4(aPosition, 1.0);
}

#source fragment
#version 450 core

void main()
{
}